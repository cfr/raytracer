#pragma once

#include "box.hpp"
#include "ray.hpp"
#include "values.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace aktis {

using PrimId = std::uint32_t;

// Called with the slot of each candidate primitive, near-first; returns true to stop.
template <class F>
concept LeafCallback = std::is_invocable_r_v<bool, F const&, PrimId>;

// Binned-SAH BVH. Tree is an index structure, build reorders arrays
class BoundingVolumeHierarchy final {
    using NodeId = std::uint32_t;
    static constexpr NodeId nullNode = std::numeric_limits<NodeId>::max();

    // left node is always next (id + 1), leaf prims is [first, first + count)
    struct Node {
        Box box;
        std::uint32_t payload = nullNode;  // leaf: first; internal: right child
        PrimId count = 0;                  // 0 = internal

        explicit Node(Box const& b) : box{b} {}

        [[nodiscard]] bool leaf() const {
            return count > 0;
        }
        [[nodiscard]] NodeId right() const {
            return payload;
        }
        [[nodiscard]] PrimId first() const {
            return payload;
        }
        void makeLeaf(PrimId first, PrimId n) {
            payload = first;
            count = n;
        }
        void setRight(NodeId id) {
            payload = id;
        }
    };

    struct Prim {
        Box box;
        PrimId index;
    };

    struct Bounds {
        Box box;
        Box centroids;  // aabb of centroids
    };

    static constexpr PrimId sahMinPrims = 2;  // never split fewer
    static constexpr PrimId sahMaxPrims = 8;  // may stay a leaf
    static constexpr int binCount = 16;
    static constexpr Float traversalCost = 1.0;
    static constexpr int maxDepth = 64;
    // pop one, push two
    static constexpr size_t stackSize = maxDepth + 2;

    static auto onAxis(int axis) {
        return [axis](Prim const& p) { return p.box.centroid()[axis]; };
    }

    // centroid coordinate -> [0, binCount)
    struct BinMap {
        Float scale = 0;
        Float cmin = 0;

        int operator()(Float c) const {
            Float const t = (c - cmin) * scale;
            return t > 0 ? int(std::min(t, Float(binCount - 1))) : 0;
        }
    };

    struct Split {
        int axis = -1;
        int bin = 0;  // last bin that goes left
        Float cost = inf;
        BinMap map;

        [[nodiscard]] bool found() const {
            return axis >= 0;
        }
        [[nodiscard]] bool goesLeft(Prim const& p) const {
            return map(p.box.centroid()[axis]) <= bin;
        }
    };

    std::vector<Node> nodes_;
    NodeId root_ = nullNode;
    std::vector<PrimId> ids_;  // original order, for tie-break

  public:
    BoundingVolumeHierarchy() = default;

    // builds from world-space boxes
    explicit BoundingVolumeHierarchy(std::span<Box const> boxes) {
        if (boxes.empty()) {
            return;
        }
        if (boxes.size() > std::numeric_limits<PrimId>::max() / 2) {
            throw std::length_error("too many primitives for the BVH's 32-bit indices");
        }

        std::vector<Prim> prims;
        prims.reserve(boxes.size());
        for (PrimId index = 0; auto const& box : boxes) {
            prims.push_back(Prim{box, index++});
        }
        nodes_.reserve(prims.size() * 2);
        root_ = build(prims, 0, static_cast<PrimId>(prims.size()), 0);
        ids_.reserve(prims.size());
        for (Prim const& prim : prims) {
            ids_.push_back(prim.index);
        }
    }

    // slot -> original index
    [[nodiscard]] std::vector<PrimId> const& order() const {
        return ids_;
    }

    [[nodiscard]] size_t size() const {
        return ids_.size();
    }

    // any-hit traversal within tmax, no pruning; return true from the callback to stop
    template <LeafCallback F> void any(Ray const& ray, Float tmax, F const& leafCallback) const {
        traverse<Prune::no>(ray, tmax, leafCallback);
    }

    // closest-hit traversal, near-first: callback(slot) tests one primitive, may lower tmax
    template <LeafCallback F>
    void closest(Ray const& ray, Float& tmax, F const& leafCallback) const {
        traverse<Prune::yes>(ray, tmax, leafCallback);
    }

  private:
    enum class Prune : std::uint8_t { no, yes };

    template <Prune prune, LeafCallback F>
    void traverse(Ray const& ray, Float& tmax, F const& leafCallback) const {
        if (root_ == nullNode) {
            return;
        }

        // occlusion only needs the node id; intersect also tracks the box entry t
        struct Slot {
            NodeId id;
            Float enter;
        };
        std::array<Slot, stackSize> stack;
        size_t top = 0;

        auto enterOf = [&](NodeId id) { return nodes_[id].box.enter(ray.origin, ray.inv, tmax); };
        auto push = [&](NodeId id, Float enter) {
            if (enter == inf) {
                return;  // missed
            }
            assert(top < stackSize);
            stack[top].id = id;
            if constexpr (prune == Prune::yes) {
                stack[top].enter = enter;
            }
            ++top;
        };

        push(root_, enterOf(root_));
        while (top > 0) {
            --top;
            if constexpr (prune == Prune::yes) {
                if (stack[top].enter > tmax) {
                    continue;  // a nearer hit was found since this node was pushed
                }
            }
            Node const& node = nodes_[stack[top].id];

            if (node.leaf()) {
                PrimId const first = node.first();
                for (PrimId k = 0; k < node.count; ++k) {
                    if (leafCallback(first + k)) {
                        return;
                    }
                }
            } else {
                NodeId near = stack[top].id + 1, far = node.right();
                Float tN = enterOf(near), tF = enterOf(far);
                if (tF < tN) {
                    std::swap(tN, tF);
                    std::swap(near, far);
                }
                push(far, tF);
                push(near, tN);
            }
        }
    }

    static Bounds boundsOf(std::span<Prim const> prims) {
        Bounds b;
        for (Prim const& p : prims) {
            b.box.expand(p.box);
            if (!p.box.empty()) {
                b.centroids.expand(p.box.centroid());
            }
        }
        return b;
    }

    struct Bin {
        Box box;
        int count = 0;
    };
    struct Side {
        Float area = 0;
        int count = 0;
    };

    // cheapest bin boundary along one axis, inf-cost Split if none
    static Split splitOnAxis(std::span<Prim const> prims, Bounds const& bounds, int axis,
                             Float area) {
        Float const cmin = bounds.centroids.min[axis];
        Float const cmax = bounds.centroids.max[axis];

        // bin the primitives by centroid
        auto coord = onAxis(axis);
        BinMap const map{Float(binCount) / (cmax - cmin), cmin};
        std::array<Bin, binCount> bins{};
        for (Prim const& p : prims) {
            Bin& bin = bins[map(coord(p))];
            bin.box.expand(p.box);
            ++bin.count;
        }

        // suffix sweep: right[i] aggregates bins [i, binCount)
        std::array<Side, binCount> right{};
        Box rightBox;
        for (int i = binCount - 1, count = 0; i >= 0; --i) {
            count += bins[i].count;
            rightBox.expand(bins[i].box);
            right[i] = Side{rightBox.surfaceArea(), count};
        }

        // prefix sweep: pick the cheapest boundary
        Split best;
        Box leftBox;
        for (int i = 0, leftCount = 0; i < binCount - 1; ++i) {
            leftCount += bins[i].count;
            leftBox.expand(bins[i].box);
            Side const& r = right[i + 1];
            if (leftCount == 0 || r.count == 0)
                continue;
            Float const cost =
                traversalCost
                + (Float(leftCount) * leftBox.surfaceArea() + Float(r.count) * r.area) / area;
            if (cost < best.cost) {
                best = Split{axis, i, cost, map};
            }
        }
        return best;
    }

    static Split findSplit(std::span<Prim const> prims, Bounds const& bounds) {
        Float const area = bounds.box.surfaceArea();
        if (!(area > 0)) {
            return {};
        }

        Split best;
        for (int axis = 0; axis < 3; ++axis) {
            if (!(bounds.centroids.max[axis] > bounds.centroids.min[axis]))
                continue;
            Split const candidate = splitOnAxis(prims, bounds, axis, area);
            if (candidate.cost < best.cost) {
                best = candidate;
            }
        }
        return best;
    }

    NodeId build(std::vector<Prim>& prims, PrimId start, PrimId end, int depth) {
        assert(start < end);

        PrimId n = end - start;
        std::span<Prim> range{prims.begin() + start, n};
        Bounds const bounds = boundsOf(range);

        NodeId const id = static_cast<NodeId>(nodes_.size());
        nodes_.emplace_back(bounds.box);

        if (n <= sahMinPrims || depth >= maxDepth) {
            nodes_[id].makeLeaf(start, n);
            return id;
        }

        auto medianSplit = [&](int axis) {
            std::ranges::nth_element(range.begin(), range.begin() + n / 2, range.end(), {},
                                     onAxis(axis));
            return start + n / 2;
        };

        Split split = findSplit(range, bounds);

        PrimId mid;
        if (split.found()) {
            // splitting must beat testing all n primitives directly
            if (split.cost >= Float(n) && n <= sahMaxPrims) {
                nodes_[id].makeLeaf(start, n);
                return id;
            }
            auto right =
                std::ranges::partition(range, [&](Prim const& p) { return split.goesLeft(p); });
            mid = start + static_cast<PrimId>(right.begin() - range.begin());
            if (mid == start || mid == end) {
                mid = medianSplit(split.axis);  // all in one bin: fall back
            }
        } else {
            mid = medianSplit(bounds.box.majorAxis());  // degenerate bounds
        }

        [[maybe_unused]] NodeId const left = build(prims, start, mid, depth + 1);
        assert(left == id + 1);  // preorder: the left child follows its parent
        nodes_[id].setRight(build(prims, mid, end, depth + 1));
        return id;
    }
};

}  // namespace aktis
