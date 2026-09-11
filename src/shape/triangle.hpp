#pragma once

#include "box.hpp"
#include "ray.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/geometric.hpp>

namespace aktis {

struct Triangle {
    Vec3 a;
    Vec3 edge1;  // b - a
    Vec3 edge2;  // c - a

    [[nodiscard]] static Triangle of(Vec3 a, Vec3 b, Vec3 c) {
        Triangle t;
        t.a = a;
        t.edge1 = b - a;
        t.edge2 = c - a;
        return t;
    }

    [[nodiscard]] Box aabb() const {
        // reconstructed within an ulp
        Vec3 const b = a + edge1;
        Vec3 const c = a + edge2;
        return {.min = glm::min(a, glm::min(b, c)), .max = glm::max(a, glm::max(b, c))};
    }

    [[nodiscard]] static Float pdfArea() {
        return 0;  // not samplable
    }

    [[nodiscard]] Vec4 normal(Vec3 /*point*/) const {
        // unnormalised; makeHit normalises once per confirmed hit
        return Vec4{glm::cross(edge1, edge2), 0};
    }

    [[nodiscard]] Float tlocal(Ray ray) const {
        auto h = glm::cross(ray.dir, edge2);
        auto det = glm::dot(edge1, h);

        if (det == 0) {
            return 0;
        }

        auto f = 1 / det;
        auto s = ray.origin - a;

        auto u = f * glm::dot(s, h);
        if (u < 0 || u > 1) {
            return 0;
        }

        auto q = glm::cross(s, edge1);
        auto v = f * glm::dot(ray.dir, q);
        if (v < 0 || u + v > 1) {
            return 0;
        }

        auto t = f * glm::dot(edge2, q);
        return t;
    }
};

}  // namespace aktis
