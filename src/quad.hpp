#pragma once

#include "values.hpp"
#include "ray.hpp"
#include "hit.hpp"

#include <glm/glm.hpp>

namespace raytracer {

struct Quad : Hittable {
    Vec3 v0 = {0, 0, 0};
    Vec3 v1 = {0, 0, 0};
    Vec3 v2 = {0, 0, 0};
    Vec3 v3 = {0, 0, 0};
    Vec3 edge1 = {0, 0, 0};
    Vec3 edge2 = {0, 0, 0};
    Vec3 planeNormal = {0, 0, 0};
    Float area = 0;
    Color radiance = colors::black;

    Quad(const Object& obj, Vec3 v0, Vec3 v1, Vec3 v2, Vec3 v3, Color radiance)
        : Hittable{obj}, v0(v0), v1(v1), v2(v2), v3(v3), radiance(radiance) {
        edge1 = v1 - v0;
        edge2 = v3 - v0;
        Vec3 n = glm::cross(edge2, edge1);
        area = glm::length(n);
        planeNormal = n / area;
    }

    Box aabb() const override {
        Vec3 lo = glm::min(glm::min(v0, v1), glm::min(v2, v3));
        Vec3 hi = glm::max(glm::max(v0, v1), glm::max(v2, v3));
        return {lo, hi};
    }

    Vec4 normal(Vec3 /*point*/) const override {
        return Vec4{planeNormal, 0};
    }

    Float tlocal(Ray ray) const override {
        const Float denom = glm::dot(planeNormal, ray.dir);
        if (glm::abs(denom) < step) return 0;

        const Float t = glm::dot(v0 - ray.origin, planeNormal) / denom;
        if (t <= 0) return 0;

        const Vec3 p = ray.at(t);

        const Float d0 = glm::dot(glm::cross(v1 - v0, p - v0), planeNormal);
        const Float d1 = glm::dot(glm::cross(v2 - v1, p - v1), planeNormal);
        const Float d2 = glm::dot(glm::cross(v3 - v2, p - v2), planeNormal);
        const Float d3 = glm::dot(glm::cross(v0 - v3, p - v3), planeNormal);

        const auto d = Vec4{d0, d1, d2, d3};
        const auto zero = Vec4{0.0};
        const bool inside = glm::all(glm::greaterThanEqual(d, zero))
                         || glm::all(glm::lessThanEqual(d, zero));

        return inside ? t : 0;
    }

    Float irradiance(const Vec3 r, const Vec3 rnormal) const {
        Vec3 u0 = glm::normalize(v0 - r);
        Vec3 u1 = glm::normalize(v1 - r);
        Vec3 u2 = glm::normalize(v2 - r);
        Vec3 u3 = glm::normalize(v3 - r);
        auto edge = [](Vec3 a, Vec3 b) -> Vec3 {
            Float theta = glm::acos(glm::clamp(glm::dot(a, b), Float(-1), Float(1)));
            Vec3 c = glm::cross(a, b);
            Float len = glm::length(c);
            return len > 0 ? theta * (c / len) : Vec3{0.0};
        };
        Vec3 phi = edge(u0, u1) + edge(u1, u2) + edge(u2, u3) + edge(u3, u0);
        return 0.5 * glm::dot(phi, rnormal);
    }

    Vec3 sample(Vec2 u) const {
        return v0 + u.x * edge1 + u.y * edge2;
    }
};

}  // namespace raytracer
