#pragma once

#include "box.hpp"
#include "ray.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace aktis {

// parallelogram: corners v0, v0 + edge1, v0 + edge1 + edge2, v0 + edge2
struct Quad {
    Vec3 v0;
    Vec3 edge1;
    Vec3 edge2;
    Vec3 n;
    Float area;

    [[nodiscard]] static Quad of(Vec3 v0, Vec3 edge1, Vec3 edge2) {
        Quad q;
        q.v0 = v0;
        q.edge1 = edge1;
        q.edge2 = edge2;
        Vec3 const normal = glm::cross(edge2, edge1);
        q.area = glm::length(normal);
        q.n = normal / q.area;
        return q;
    }

    [[nodiscard]] Box aabb() const {
        Vec3 const v1 = v0 + edge1;
        Vec3 const v2 = v0 + edge1 + edge2;
        Vec3 const v3 = v0 + edge2;
        Vec3 const lo = glm::min(glm::min(v0, v1), glm::min(v2, v3));
        Vec3 const hi = glm::max(glm::max(v0, v1), glm::max(v2, v3));
        return {.min = lo, .max = hi};
    }

    [[nodiscard]] Float pdfArea() const {
        return 1 / area;
    }

    [[nodiscard]] Vec4 normal(Vec3 /*point*/) const {
        return Vec4{n, 0};
    }

    [[nodiscard]] Float tlocal(Ray ray) const {
        Float const denom = glm::dot(n, ray.dir);
        if (denom == 0)
            return 0;

        Float const t = glm::dot(v0 - ray.origin, n) / denom;
        if (t <= 0)
            return 0;

        Vec3 const q = ray.at(t) - v0;
        Float const u = glm::dot(glm::cross(edge2, q), n) / area;
        Float const v = glm::dot(glm::cross(q, edge1), n) / area;
        bool const inside = u >= 0 && u <= 1 && v >= 0 && v <= 1;

        return inside ? t : 0;
    }

    [[nodiscard]] Float irradiance(Vec3 const r, Vec3 const rnormal) const {
        Vec3 const u0 = glm::normalize(v0 - r);
        Vec3 const u1 = glm::normalize(v0 + edge1 - r);
        Vec3 const u2 = glm::normalize(v0 + edge1 + edge2 - r);
        Vec3 const u3 = glm::normalize(v0 + edge2 - r);
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

}  // namespace aktis
