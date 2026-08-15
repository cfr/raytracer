#pragma once

#include "values.hpp"
#include "transforms.hpp"

#include <glm/common.hpp>

#include <utility>

namespace raytracer {

struct Box {
    Vec3 min{inf};
    Vec3 max{-inf};

    std::pair<Float, Float> slab(const Vec3& origin, const Vec3& inv) const {
        // Thiago Ize, 2013, "Robust BVH Ray Traversal"
        Float tenter = -inf, texit = inf;
        for (int i = 0; i < 3; ++i) {
            Float t1 = (min[i] - origin[i]) * inv[i];
            Float t2 = (max[i] - origin[i]) * inv[i];
            tenter = glm::min(glm::max(tenter, t1), glm::max(tenter, t2));
            texit  = glm::max(glm::min(texit,  t1), glm::min(texit,  t2));
        }
        return {tenter, texit};
    }

    Float enter(const Vec3& origin, const Vec3& inv) const {
        return slab(origin, inv).first;
    }

    bool intersects(const Vec3& origin, const Vec3& inv) const {
        auto [te, tx] = slab(origin, inv);
        return te <= tx && tx >= 0;
    }

    bool intersects(const Vec3& origin, const Vec3& inv, Float tmin, Float tmax) const {
        auto [te, tx] = slab(origin, inv);
        return te <= tx && tx >= tmin && te <= tmax;
    }

    Vec3 center() const {
        return (min + max) * Float(0.5);
    }

    bool empty() const {
        return min.x > max.x || min.y > max.y || min.z > max.z;
    }

    void expand(const Vec3& p) {
        min = glm::min(min, p);
        max = glm::max(max, p);
    }

    void expand(const Box& b) {
        min = glm::min(min, b.min);
        max = glm::max(max, b.max);
    }

    Vec3 extent() const {
        return empty() ? Vec3(0) : max - min;
    }

    Vec3 centroid() const {
        return empty() ? Vec3(0) : Float(0.5) * (min + max);
    }

    Float surfaceArea() const {
        Vec3 d = extent();
        return Float(2) * (d.x*d.y + d.y*d.z + d.z*d.x);
    }

    Box transformed(const Transforms& xf) const {
        if (empty()) { return Box(); }
        Box w;
        for (int i = 0; i < 8; ++i) {
            w.expand(transformPoint(xf.m, Vec3{
                (i & 1) ? max.x : min.x,
                (i & 2) ? max.y : min.y,
                (i & 4) ? max.z : min.z }));
        }
        return w;
    }
};

inline Box merge(const Box& a, const Box& b) {
    return { glm::min(a.min, b.min), glm::max(a.max, b.max) };
}

}  // namespace raytracer
