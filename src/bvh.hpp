#pragma once

#include "values.hpp"
#include "box.hpp"
#include "ray.hpp"
#include "hit.hpp"

#include <glm/glm.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace raytracer {

template<typename T>
concept SceneObject = requires(T obj, const Ray& ray) {
    { obj->aabb() } -> std::convertible_to<Box>;
    { obj->intersect(ray) } -> std::convertible_to<std::optional<Hit>>;
    { obj->tworld(ray) } -> std::convertible_to<Float>;
};

template <SceneObject Obj> class BoundingVolumeHierarchy {

    struct Node;
    using ObjId = std::vector<Obj>::size_type;
    using NodeId = std::vector<Node>::size_type;
    static constexpr NodeId nullNode = std::numeric_limits<NodeId>::max();

    struct Node
    {
        Box box;
        NodeId left = nullNode;
        NodeId right = nullNode;
        ObjId start = 0;
        ObjId count = 0;
        bool leaf() const { return left == nullNode && right == nullNode; }
    };

    struct Prim {
        Obj obj;
        Box box;
    };

    static constexpr ObjId LeafSize = 4;
    static constexpr size_t StackSize = 64;

    std::vector<Obj> objects;
    std::vector<Node> nodes;
    NodeId root = nullNode;

 public:
    BoundingVolumeHierarchy() {}

    explicit BoundingVolumeHierarchy(std::vector<Obj> objs) {
        if (objs.empty()) {
            return;
        }
        std::vector<Prim> prims;
        prims.reserve(objs.size());
        for (auto& obj : objs) {
            Box box = obj->aabb();
            prims.push_back(Prim{std::move(obj), box});
        }
        nodes.reserve(prims.size() * 2);
        root = build(prims, 0, prims.size());

        objects.reserve(prims.size());
        for (auto& prim : prims) {
            objects.push_back(std::move(prim.obj));
        }
    }

    std::optional<Hit> intersect(const Ray& ray) const {
        if (root == nullNode || childEnter(root, ray, inf) == inf) {
            return {};
        }

        std::optional<Hit> closest;
        Float closestT = inf;

        struct Entry { NodeId id; Float enter; };
        std::array<Entry, StackSize> stk;
        size_t top = 0;
        stk[top++] = {root, 0};

        while (top > 0) {
            auto [id, enter] = stk[--top];
            if (enter >= closestT) continue;
            const Node& n = nodes[id];

            if (n.leaf()) {
                for (ObjId i = n.start; i < n.start + n.count; ++i) {
                    if (auto hit = objects[i]->intersect(ray)) {
                        if (hit->t < closestT) {
                            closestT = hit->t;
                            closest = std::move(hit);
                        }
                    }
                }
            } else {
                Float tL = childEnter(n.left, ray, closestT);
                Float tR = childEnter(n.right, ray, closestT);
                NodeId near = n.left, far = n.right;
                if (tR < tL) {
                    std::swap(tL, tR);
                    std::swap(near, far);
                }
                if (tR < inf) { assert(top < StackSize); stk[top++] = {far, tR}; }
                if (tL < inf) { assert(top < StackSize); stk[top++] = {near, tL}; }
            }
        }
        return closest;
    }

    [[nodiscard]] bool occluded(const Ray& ray, Float tmax = inf,
                                const Hittable* ignore = nullptr) const
    {
        if (root == nullNode || childEnter(root, ray, tmax) == inf) {
            return false;
        }

        std::array<NodeId, StackSize> stk;
        size_t top = 0;
        stk[top++] = root;

        while (top > 0) {
            const Node& n = nodes[stk[--top]];

            if (n.leaf()) {
                for (ObjId i = n.start; i < n.start + n.count; ++i) {
                    const auto& obj = objects[i];
                    if (ignore && obj.get() == ignore) continue;
                    Float t = obj->tworld(ray);
                    if (t > 0 && t <= tmax) {
                        return true;
                    }
                }
            } else {
                Float tL = childEnter(n.left, ray, tmax);
                Float tR = childEnter(n.right, ray, tmax);
                NodeId near = n.left, far = n.right;
                if (tR < tL) {
                    std::swap(tL, tR);
                    std::swap(near, far);
                }
                if (tR < inf) { assert(top < StackSize); stk[top++] = far; }
                if (tL < inf) { assert(top < StackSize); stk[top++] = near; }
            }
        }
        return false;
    }

 private:
    Float childEnter(NodeId id, const Ray& ray, Float tmax) const {
        if (id == nullNode) return inf;
        auto [te, tx] = nodes[id].box.slab(ray.origin, ray.inv);
        return (te <= tx && tx >= 0 && te <= tmax) ? te : inf;
    }

    NodeId build(std::vector<Prim>& prims, ObjId start, ObjId end) {
        if (start >= end) return nullNode;

        Box bounds = prims[start].box;
        for (ObjId i = start + 1; i < end; ++i) {
            bounds = merge(bounds, prims[i].box);
        }

        NodeId id = static_cast<NodeId>(nodes.size());
        nodes.emplace_back(Node{.box = bounds});

        if (end - start <= LeafSize) {
            nodes[id].start = start;
            nodes[id].count = end - start;
            return id;
        }

        Vec3 extent = bounds.max - bounds.min;
        int axis = (extent.x >= extent.y && extent.x >= extent.z) ? 0
                 : (extent.y >= extent.z) ? 1 : 2;

        ObjId mid = start + (end - start) / 2;
        std::nth_element(prims.begin() + start,
                        prims.begin() + mid,
                        prims.begin() + end,
            [axis](const Prim& a, const Prim& b) {
                Float ca = a.box.min[axis] + a.box.max[axis];  // 2x centroid
                Float cb = b.box.min[axis] + b.box.max[axis];
                return ca < cb;
            });

        nodes[id].left  = build(prims, start, mid);
        nodes[id].right = build(prims, mid, end);
        return id;
    }
};

}  // namespace raytracer
