#pragma once

#include "values.hpp"
#include "hit.hpp"

#include <glm/glm.hpp>

namespace raytracer {

struct QuadLight {
    Vec3 v0 = {0, 0, 0};
    Vec3 v1 = {0, 0, 0};
    Vec3 v2 = {0, 0, 0};
    Vec3 v3 = {0, 0, 0};
    Vec3 edge1 = {0, 0, 0};
    Vec3 edge2 = {0, 0, 0};
    Vec3 normal = {0, 0, 0};
    Color radiance = {0, 0, 0, 1};

    Float irradiance(const Vec3 r, const Vec3 rnormal) const {
        Vec3 u0 = glm::normalize(v0 - r);
        Vec3 u1 = glm::normalize(v1 - r);
        Vec3 u2 = glm::normalize(v2 - r);
        Vec3 u3 = glm::normalize(v3 - r);
        Vec3 phi = Vec3{0.0};
        phi += glm::acos(glm::dot(u0, u1)) * glm::normalize(glm::cross(u0, u1));
        phi += glm::acos(glm::dot(u1, u2)) * glm::normalize(glm::cross(u1, u2));
        phi += glm::acos(glm::dot(u2, u3)) * glm::normalize(glm::cross(u2, u3));
        phi += glm::acos(glm::dot(u3, u0)) * glm::normalize(glm::cross(u3, u0));
        return 0.5 * glm::dot(phi, rnormal);
    }

    Float intersect(const Ray ray) const {
        const Float denom = dot(normal, ray.dir);
        if (glm::abs(denom) < Hittable::step) return inf;

        const Float t = dot(v0 - ray.eye, normal) / denom;
        if (t <= Hittable::step) return inf;

        const Vec3 p = ray.at(t);

        const Float d0 = glm::dot(glm::cross(v1 - v0, p - v0), normal);
        const Float d1 = glm::dot(glm::cross(v2 - v1, p - v1), normal);
        const Float d2 = glm::dot(glm::cross(v3 - v2, p - v2), normal);
        const Float d3 = glm::dot(glm::cross(v0 - v3, p - v3), normal);

        const auto d = Vec4{d0, d1, d2, d3};
        const auto zero = Vec4{0.0};
        const bool inside = glm::all(glm::greaterThanEqual(d, zero))
                         || glm::all(glm::lessThanEqual(d, zero));

        return inside ? t : inf;
    }
};

}  // namespace raytracer
