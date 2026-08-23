#pragma once

#include "hittable.hpp"
#include "ray.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vector_relational.hpp>

#include <memory>

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

    Quad(MaterialPtr m, Vec3 v0, Vec3 v1, Vec3 v2, Vec3 v3)
        : Hittable{std::move(m)}, v0(v0), v1(v1), v2(v2), v3(v3) {
        edge1 = v1 - v0;
        edge2 = v3 - v0;
        Vec3 const n = glm::cross(edge2, edge1);
        area = glm::length(n);
        planeNormal = n / area;
    }

    [[nodiscard]] Box aabb() const override {
        Vec3 const lo = glm::min(glm::min(v0, v1), glm::min(v2, v3));
        Vec3 const hi = glm::max(glm::max(v0, v1), glm::max(v2, v3));
        return {.min = lo, .max = hi};
    }

    [[nodiscard]] Float pdfArea() const override {
        return 1 / area;
    }

    [[nodiscard]] Vec4 normal(Vec3 /*point*/) const override {
        return Vec4{planeNormal, 0};
    }

    [[nodiscard]] Float tlocal(Ray ray) const override {
        Float const denom = glm::dot(planeNormal, ray.dir);
        if (denom == 0)
            return 0;

        Float const t = glm::dot(v0 - ray.origin, planeNormal) / denom;
        if (t <= 0)
            return 0;

        Vec3 const p = ray.at(t);

        Float const d0 = glm::dot(glm::cross(v1 - v0, p - v0), planeNormal);
        Float const d1 = glm::dot(glm::cross(v2 - v1, p - v1), planeNormal);
        Float const d2 = glm::dot(glm::cross(v3 - v2, p - v2), planeNormal);
        Float const d3 = glm::dot(glm::cross(v0 - v3, p - v3), planeNormal);

        auto const d = Vec4{d0, d1, d2, d3};
        auto const zero = Vec4{0};
        bool const inside =
            glm::all(glm::greaterThanEqual(d, zero)) || glm::all(glm::lessThanEqual(d, zero));

        return inside ? t : 0;
    }

    [[nodiscard]] Float irradiance(Vec3 const r, Vec3 const rnormal) const {
        Vec3 const u0 = glm::normalize(v0 - r);
        Vec3 const u1 = glm::normalize(v1 - r);
        Vec3 const u2 = glm::normalize(v2 - r);
        Vec3 const u3 = glm::normalize(v3 - r);
        auto edge = [](Vec3 a, Vec3 b) -> Vec3 {
            Float const theta = glm::acos(glm::clamp(glm::dot(a, b), Float(-1), Float(1)));
            Vec3 const c = glm::cross(a, b);
            Float const len = glm::length(c);
            return len > 0 ? theta * (c / len) : Vec3{0};
        };
        Vec3 const phi = edge(u0, u1) + edge(u1, u2) + edge(u2, u3) + edge(u3, u0);
        return Float(0.5) * glm::dot(phi, rnormal);
    }

    [[nodiscard]] Vec3 sample(Vec2 u) const {
        return v0 + u.x * edge1 + u.y * edge2;
    }
};

using QuadPtr = std::shared_ptr<Quad const>;

}  // namespace raytracer
