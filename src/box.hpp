#pragma once

#include "values.hpp"

#include <glm/glm.hpp>

namespace raytracer {

struct Box
{
    Vec3 min{inf};
    Vec3 max{-inf};

    Float enter(const Vec3& origin, const Vec3& inv) const {
        Vec3 t1 = (min - origin) * inv;
        Vec3 t2 = (max - origin) * inv;
        Vec3 tmin = glm::min(t1, t2);
        return glm::max(tmin.x, glm::max(tmin.y, tmin.z));
    }

    bool intersects(const Vec3& origin, const Vec3& inv) const {
        Vec3 t1 = (min - origin) * inv;
        Vec3 t2 = (max - origin) * inv;
        Vec3 tmin = glm::min(t1, t2);
        Vec3 tmax = glm::max(t1, t2);
        Float tenter = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
        Float texit  = glm::min(tmax.x, glm::min(tmax.y, tmax.z));
        return tenter <= texit && texit >= 0;
    }

    bool intersects(const Vec3& origin, const Vec3& inv, Float tmin, Float tmax) const
    {
        Vec3 t1 = (min - origin) * inv;
        Vec3 t2 = (max - origin) * inv;
        Vec3 btmin = glm::min(t1, t2);
        Vec3 btmax = glm::max(t1, t2);

        Float enter = glm::max(btmin.x, glm::max(btmin.y, btmin.z));
        Float exit  = glm::min(btmax.x, glm::min(btmax.y, btmax.z));

        return enter <= exit && exit >= tmin && enter <= tmax;
    }

    inline Vec3 center() {
        return (min + max) * 0.5;
    }
};

inline Box merge(const Box& a, const Box& b) {
    return { glm::min(a.min, b.min), glm::max(a.max, b.max) };
}

}  // namespace raytracer
