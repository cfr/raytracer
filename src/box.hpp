#pragma once

#include "transforms.hpp"
#include "values.hpp"

#include <glm/common.hpp>

#include <utility>

namespace raytracer {

struct Box {
    Vec3 min{inf};
    Vec3 max{-inf};

    [[nodiscard]] std::pair<Float, Float> slab(Vec3 const& origin, Vec3 const& inv) const {
        // Thiago Ize, 2013, "Robust BVH Ray Traversal"
        Float tenter = -inf, texit = inf;
        for (int i = 0; i < 3; ++i) {
            Float const t1 = (min[i] - origin[i]) * inv[i];
            Float const t2 = (max[i] - origin[i]) * inv[i];
            tenter = glm::min(glm::max(tenter, t1), glm::max(tenter, t2));
            texit = glm::max(glm::min(texit, t1), glm::min(texit, t2));
        }
        return {tenter, texit};
    }

    [[nodiscard]] Float enter(Vec3 const& origin, Vec3 const& inv) const {
        return slab(origin, inv).first;
    }

    [[nodiscard]] bool intersects(Vec3 const& origin, Vec3 const& inv) const {
        auto [te, tx] = slab(origin, inv);
        return te <= tx && tx >= 0;
    }

    [[nodiscard]] bool intersects(Vec3 const& origin, Vec3 const& inv, Float tmin,
                                  Float tmax) const {
        auto [te, tx] = slab(origin, inv);
        return te <= tx && tx >= tmin && te <= tmax;
    }

    [[nodiscard]] Vec3 center() const {
        return (min + max) * Float(0.5);
    }

    [[nodiscard]] bool empty() const {
        return min.x > max.x || min.y > max.y || min.z > max.z;
    }

    void expand(Vec3 const& p) {
        min = glm::min(min, p);
        max = glm::max(max, p);
    }

    void expand(Box const& b) {
        min = glm::min(min, b.min);
        max = glm::max(max, b.max);
    }

    [[nodiscard]] Vec3 extent() const {
        return empty() ? Vec3(0) : max - min;
    }

    [[nodiscard]] Vec3 centroid() const {
        return empty() ? Vec3(0) : Float(0.5) * (min + max);
    }

    [[nodiscard]] Float surfaceArea() const {
        Vec3 const d = extent();
        return Float(2) * (d.x * d.y + d.y * d.z + d.z * d.x);
    }

    [[nodiscard]] Box transformed(Transforms const& xf) const {
        if (empty()) {
            return Box();
        }
        Box w;
        for (int i = 0; i < 8; ++i) {
            auto corner = Vec3{((i & 1) != 0) ? max.x : min.x, ((i & 2) != 0) ? max.y : min.y,
                               ((i & 4) != 0) ? max.z : min.z};
            w.expand(transformPoint(xf.m, corner));
        }
        return w;
    }
};

inline Box merge(Box const& a, Box const& b) {
    return {.min = glm::min(a.min, b.min), .max = glm::max(a.max, b.max)};
}

}  // namespace raytracer
