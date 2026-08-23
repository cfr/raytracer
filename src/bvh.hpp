#pragma once

#include "box.hpp"
#include "hittable.hpp"
#include "ray.hpp"
#include "values.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace aktis {

template <typename T>
concept SceneObject = requires(T obj, Ray const& ray, Float t) {
    { obj->aabb() } -> std::convertible_to<Box>;
    { obj->tworld(ray) } -> std::convertible_to<Float>;
    { obj->makeHit(ray, t) } -> std::convertible_to<Hit>;
    { obj.get() } -> std::convertible_to<Hittable const*>;
};

template <SceneObject Obj> class BoundingVolumeHierarchy {
    using ObjId = std::uint32_t;
    using NodeId = std::uint32_t;
    static constexpr NodeId nullNode = std::numeric_limits<NodeId>::max();

    using ObjRaw = decltype(std::declval<Obj const&>().get());

    struct Node {
        Box box;
        // internal: index of the right child (the left child is always id + 1)
        // leaf:     index of its first primitive in ptrs_/ids_
        std::uint32_t payload = nullNode;
        ObjId count = 0;  // 0 = internal

        explicit Node(Box const& b) : box{b} {}
        [[nodiscard]] bool leaf() const {
            return count > 0;
        }
        [[nodiscard]] NodeId right() const {
            return payload;
        }
        [[nodiscard]] ObjId start() const {
            return payload;
        }
    };

    struct Prim {
        Box box;
        ObjId index;
    };

    struct PruningSlot {
        NodeId id;
        Float enter;
        [[nodiscard]] bool stale(Float tmax) const {
            return enter > tmax;
        }
    };
    struct PlainSlot {
        NodeId id;
        PlainSlot() = default;
        PlainSlot(NodeId id, Float /*unused*/) : id{id} {}
        [[nodiscard]] bool stale(Float /*unused*/) const {
            return false;
        }
    };

    struct Bounds {
        Box box;
        Box centroids;
    };

    static constexpr ObjId sahMinPrims = 2;
    static constexpr ObjId sahMaxPrims = 8;
    static constexpr int binCount = 16;
    static constexpr Float traversalCost = 0.125;
    static constexpr int maxDepth = 64;
    static constexpr size_t stackSize = maxDepth + 2;

    static auto onAxis(int axis) {
        return [axis](Prim const& p) { return p.box.centroid()[axis]; };
    }

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
        int bin = 0;
        Float cost = inf;
        BinMap map;

        [[nodiscard]] bool found() const {
            return axis >= 0;
        }
        [[nodiscard]] bool goesLeft(Prim const& p) const {
            return map(p.box.centroid()[axis]) <= bin;
        }
    };

    std::vector<Obj> objects_;
    std::vector<ObjRaw> ptrs_;
    std::vector<ObjId> ids_;
    std::vector<Node> nodes_;
    NodeId root_ = nullNode;

  public:
    BoundingVolumeHierarchy() = default;

    explicit BoundingVolumeHierarchy(std::vector<Obj> objs) {
        if (objs.empty()) {
            return;
        }
        if (objs.size() > std::numeric_limits<ObjId>::max() / 2) {
            throw std::length_error("too many primitives for the BVH's 32-bit indices");
        }
        std::vector<Prim> prims;
        prims.reserve(objs.size());
        for (ObjId index = 0; auto const& obj : objs) {
            prims.push_back(Prim{obj->aabb(), index++});
        }
        nodes_.reserve(prims.size() * 2);
        root_ = build(prims, 0, static_cast<ObjId>(prims.size()), 0);

        objects_.reserve(prims.size());
        ptrs_.reserve(prims.size());
        ids_.reserve(prims.size());
        for (auto& prim : prims) {
            objects_.push_back(std::move(objs[prim.index]));
            ptrs_.push_back(objects_.back().get());
            ids_.push_back(prim.index);
        }
    }

    [[nodiscard]] std::optional<Hit> intersect(Ray const& ray) const {
        ObjRaw best = nullptr;
        ObjId bestId = 0;
        Float closestT = inf;

        traverse<Prune::yes>(ray, closestT, [&](ObjRaw obj, ObjId slot) {
            Float const t = obj->tworld(ray);
            if (t > 0 && (t < closestT || (t == closestT && ids_[slot] < bestId))) {
                closestT = t;
                best = obj;
                bestId = ids_[slot];
            }
            return false;
        });

        if (!best) {
            return {};
        }
        return best->makeHit(ray, closestT);
    }

    [[nodiscard]] bool occluded(Ray const& ray, Float tmax = inf,
                                Hittable const* ignore = nullptr) const {
        bool hit = false;
        traverse<Prune::no>(ray, tmax, [&](ObjRaw obj, ObjId /*slot*/) {
            if (obj == ignore) {
                return false;
            }
            Float const t = obj->tworld(ray);
            if (t > 0 && t <= tmax) {
                hit = true;
            }
            return hit;
        });
        return hit;
    }

  private:
    enum class Prune : std::uint8_t { no, yes };

    template <Prune prune, typename LeafFn>
    void traverse(Ray const& ray, Float& tmax, LeafFn const& leaf) const {
        if (root_ == nullNode) {
            return;
        }

        using Slot = std::conditional_t<prune == Prune::yes, PruningSlot, PlainSlot>;
        std::array<Slot, stackSize> stack;
        size_t top = 0;

        auto enterOf = [&](NodeId id) { return nodes_[id].box.enter(ray.origin, ray.inv, tmax); };
        auto push = [&](NodeId id, Float enter) {
            if (enter == inf) {
                return;
            }
            assert(top < stackSize);
            stack[top++] = Slot{id, enter};
        };

        push(root_, enterOf(root_));
        while (top > 0) {
            Slot const slot = stack[--top];
            if (slot.stale(tmax)) {
                continue;
            }
            Node const& node = nodes_[slot.id];

            if (node.leaf()) {
                ObjId const base = node.start();
                for (ObjId k = 0; k < node.count; ++k) {
                    if (leaf(ptrs_[base + k], base + k)) {
                        return;
                    }
                }
            } else {
                NodeId near = slot.id + 1, far = node.right();
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

    static Split findSplit(std::span<Prim const> prims, Bounds const& bounds) {
        Split best;
        Float const area = bounds.box.surfaceArea();
        if (!(area > 0)) {
            return best;
        }

        struct Bin {
            Box box;
            int count = 0;
        };
        struct Side {
            Float area = 0;
            int count = 0;
        };

        for (int axis = 0; axis < 3; ++axis) {
            Float const cmin = bounds.centroids.min[axis];
            Float const cmax = bounds.centroids.max[axis];
            if (!(cmax > cmin))
                continue;

            auto coord = onAxis(axis);
            BinMap const map{Float(binCount) / (cmax - cmin), cmin};
            std::array<Bin, binCount> bins{};
            for (Prim const& p : prims) {
                Bin& bin = bins[map(coord(p))];
                bin.box.expand(p.box);
                ++bin.count;
            }

            std::array<Side, binCount> right{};
            Box rightBox;
            for (int i = binCount - 1, count = 0; i >= 0; --i) {
                count += bins[i].count;
                rightBox.expand(bins[i].box);
                right[i] = Side{rightBox.surfaceArea(), count};
            }

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
        }
        return best;
    }

    NodeId build(std::vector<Prim>& prims, ObjId start, ObjId end, int depth) {
        if (start >= end)
            return nullNode;

        ObjId n = end - start;
        std::span<Prim> range{prims.begin() + start, n};
        Bounds const bounds = boundsOf(range);

        NodeId id = static_cast<NodeId>(nodes_.size());
        nodes_.emplace_back(bounds.box);

        auto makeLeaf = [&] {
            nodes_[id].payload = start;
            nodes_[id].count = n;
            return id;
        };

        if (n <= sahMinPrims || depth >= maxDepth) {
            return makeLeaf();
        }

        auto medianSplit = [&](int axis) {
            std::ranges::nth_element(range.begin(), range.begin() + n / 2, range.end(), {},
                                     onAxis(axis));
            return start + n / 2;
        };

        Split split = findSplit(range, bounds);

        ObjId mid;
        if (split.found()) {
            if (split.cost >= Float(n) && n <= sahMaxPrims) {
                return makeLeaf();
            }
            auto right =
                std::ranges::partition(range, [&](Prim const& p) { return split.goesLeft(p); });
            mid = start + static_cast<ObjId>(right.begin() - range.begin());
            if (mid == start || mid == end) {
                mid = medianSplit(split.axis);
            }
        } else {
            mid = medianSplit(bounds.box.majorAxis());
        }

        [[maybe_unused]] NodeId const left = build(prims, start, mid, depth + 1);
        assert(left == id + 1);
        nodes_[id].payload = build(prims, mid, end, depth + 1);
        return id;
    }
};

}  // namespace aktis
