#pragma once

#include "values.hpp"

#include <glm/glm.hpp>

#include <utility>

namespace raytracer {

struct Box {
    Vec3 min{inf};
    Vec3 max{-inf};

    std::pair<Float, Float> slab(const Vec3& origin, const Vec3& inv) const {
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
        return (min + max) * 0.5;
    }
};

inline Box merge(const Box& a, const Box& b) {
    return { glm::min(a.min, b.min), glm::max(a.max, b.max) };
}

}  // namespace raytracer
