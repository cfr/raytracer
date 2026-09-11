#pragma once

#include "box.hpp"
#include "ray.hpp"
#include "tolerance.hpp"
#include "values.hpp"

#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

namespace aktis {

struct Sphere {
    Vec3 center;
    Float radius;

    [[nodiscard]] Box aabb() const {
        return {.min = center - Vec3{radius}, .max = center + Vec3{radius}};
    }

    [[nodiscard]] static Float pdfArea() {
        return 0;  // not samplable
    }

    [[nodiscard]] Vec4 normal(Vec3 point) const {
        return Vec4{point - center, 0};
    }

    [[nodiscard]] Float tlocal(Ray ray) const {
        auto rc = ray.origin - center;
        auto a = glm::dot(ray.dir, ray.dir);
        auto b = 2 * glm::dot(ray.dir, rc);
        auto c = glm::dot(rc, rc) - (radius * radius);

        auto disc = (b * b) - (4 * a * c);
        if (disc < 0) {
            return 0;
        }

        auto sq = glm::sqrt(disc);
        auto near = (-b - sq) / (2 * a);
        auto far = (-b + sq) / (2 * a);

        if (near > tol::tmin) {
            return near;
        }
        if (far > tol::tmin) {
            return far;
        }
        return 0;
    }
};

}  // namespace aktis
