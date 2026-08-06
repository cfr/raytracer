#pragma once

#include "values.hpp"
#include "object.hpp"
#include "box.hpp"
#include "ray.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <optional>
#include <algorithm>
#include <memory>

namespace raytracer {

using ManagedObject = std::shared_ptr<Hittable>;

struct Hittable: Object {
    static constexpr Float step = 0.0001;  // min distance for intersection

    // TODO: Wächter & Binder's integer-arithmetic offset
    static Float eps(Vec3 p) {
        return step * std::max({Float(1), glm::abs(p.x), glm::abs(p.y), glm::abs(p.z)});
    }

    explicit Hittable(const Object& obj) : Object{obj} {};
    virtual ~Hittable() = default;

    virtual Box aabb() const = 0;

    // area-measure pdf of uniform surface sampling, 0 if not samplable
    virtual Float pdfArea() const { return 0; }

    // local normal at point
    virtual Vec4 normal(Vec3 point) const = 0;

    // local-space ray parameter to object, 0 if no intersection
    virtual Float tlocal(Ray ray) const = 0;

    // world-space distance to object for unit ray, 0 if none.
    Float tworld(Ray ray) const {
        auto t = tlocal(ray.transformed(inverse));
        return t < step ? 0 : t;
    }

    std::optional<Hit> intersect(Ray ray) const {
        // transform to local coordinates
        auto tRay = ray.transformed(inverse);
        auto t = tlocal(tRay);

        if (t <= 0) { return {}; }

        auto point = tRay.at(t);

        // ray/world coordinates
        auto wpoint = transformVec3(transform, point);
        Float wt = glm::length(wpoint - ray.origin);
        if (wt < step) { return {}; }

        auto wnormal = Vec3{inverseTranspose * normal(point)};
        bool front = glm::dot(ray.dir, wnormal) < 0;
        wnormal = front ? wnormal : -wnormal;

        return Hit{-ray.dir, wpoint, glm::normalize(wnormal), wt, this, front};
    }
};

}  // namespace raytracer
