#pragma once

#include "values.hpp"
#include "box.hpp"
#include "ray.hpp"
#include "hit.hpp"

#include <glm/glm.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <limits>
#include <numeric>
#include <optional>
#include <stack>
#include <utility>
#include <vector>

namespace raytracer {

template<typename T>
concept SceneObject = requires(T obj, const Ray& ray) {
    { obj->aabb() } -> std::convertible_to<Box>;
    { obj->intersect(ray) } -> std::convertible_to<std::optional<Hit>>;
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

    static constexpr ObjId LeafSize = 4;

    std::vector<Obj> objects;
    std::vector<Node> nodes;
    NodeId root = nullNode;

 public:
    BoundingVolumeHierarchy() {}

    explicit BoundingVolumeHierarchy(std::vector<Obj> objs)
        : objects(std::move(objs))
    {
        if (objects.empty()) {
            return;
        }
        nodes.reserve(objects.size() * 2);
        root = build(0, objects.size());
    }

    std::optional<Hit> intersect(const Ray& ray) const {
        if (root == nullNode || !nodes[root].box.intersects(ray.eye, ray.inv)) {
            return {};
        }

        std::optional<Hit> closest;
        std::stack<NodeId> stk;
        stk.push(root);

        while (!stk.empty()) {
            NodeId id = stk.top(); stk.pop();
            const Node& n = nodes[id];

            if (!n.box.intersects(ray.eye, ray.inv)) continue;

            if (n.leaf()) {
                for (ObjId i = n.start; i < n.start + n.count; ++i) {
                    const auto& obj = objects[i];
                    if (auto hit = obj->intersect(ray)) {
                        if (!closest || hit->t < closest->t) {
                            closest = std::move(hit);
                        }
                    }
                }
            } else {
                Float tL = n.left  != nullNode ? nodes[n.left].box.enter(ray.eye, ray.inv) : inf;
                Float tR = n.right != nullNode ? nodes[n.right].box.enter(ray.eye, ray.inv) : inf;
                if (tL < tR) {
                    if (n.right != nullNode) stk.push(n.right);
                    if (n.left  != nullNode) stk.push(n.left);
                } else {
                    if (n.left  != nullNode) stk.push(n.left);
                    if (n.right != nullNode) stk.push(n.right);
                }
            }
        }
        return closest;
    }

    [[nodiscard]] bool occluded(const Ray& ray, Float tmax = inf,
                                const Hittable* ignore = nullptr) const
    {
        if (root == nullNode) { return false; }
        if (!nodes[root].box.intersects(ray.eye, ray.inv, 0, tmax)) {
            return false;
        }

        std::stack<NodeId> stk;
        stk.push(root);

        while (!stk.empty()) {
            NodeId id = stk.top(); stk.pop();
            const Node& n = nodes[id];

            if (!n.box.intersects(ray.eye, ray.inv, 0, tmax)) continue;

            if (n.leaf()) {
                for (ObjId i = n.start; i < n.start + n.count; ++i) {
                    const auto& obj = objects[i];
                    if (ignore && obj.get() == ignore) continue;
                    auto hit = obj->intersect(ray);
                    if (hit && hit->t > 0 && hit->t <= tmax) {
                        return true;
                    }
                }
            } else {
                Float tL = n.left  != nullNode ? nodes[n.left].box.enter(ray.eye, ray.inv) : inf;
                Float tR = n.right != nullNode ? nodes[n.right].box.enter(ray.eye, ray.inv) : inf;

                if (tL < tR) {
                    if (n.right != nullNode && tR < tmax) stk.push(n.right);
                    if (n.left  != nullNode && tL < tmax) stk.push(n.left);
                } else {
                    if (n.left  != nullNode && tL < tmax) stk.push(n.left);
                    if (n.right != nullNode && tR < tmax) stk.push(n.right);
                }
            }
        }
        return false;
    }

 private:
    NodeId build(ObjId start, ObjId end) {
        if (start >= end) return nullNode;

        Box bounds = objects[start]->aabb();
        for (ObjId i = start + 1; i < end; ++i) {
            bounds = merge(bounds, objects[i]->aabb());
        }

        NodeId id = static_cast<NodeId>(nodes.size());
        nodes.emplace_back(Node{.box = bounds});

        if (end - start <= LeafSize) {
            nodes[id].start = start;
            nodes[id].count = end - start;
            nodes[id].left = nodes[id].right = nullNode;
            return id;
        }

        Vec3 extent = bounds.max - bounds.min;
        int axis = (extent.x >= extent.y && extent.x >= extent.z) ? 0
                 : (extent.y >= extent.z) ? 1 : 2;

        ObjId mid = start + (end - start) / 2;
        std::nth_element(objects.begin() + start,
                        objects.begin() + mid,
                        objects.begin() + end,
            [axis](const auto& a, const auto& b) {
                Box ba = a->aabb(), bb = b->aabb();
                Float ca = ba.min[axis] + ba.max[axis];  // 2x
                Float cb = bb.min[axis] + bb.max[axis];
                return ca < cb;
            });

        nodes[id].left  = build(start, mid);
        nodes[id].right = build(mid, end);
        return id;
    }
};

}  // namespace raytracer
