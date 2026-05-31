#pragma once

#include "values.hpp"
#include "object.hpp"
#include "box.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace raytracer {

struct Hittable;

struct Hit {
    Float t = inf;
    Vec3 point;
    Vec3 normal;
    const Hittable* object = nullptr;
};

using ManagedObject = std::shared_ptr<Hittable>;

struct Hittable: Object {
    static constexpr Float step = 0.0001;  // min distance for intersection

    explicit Hittable(const Object& obj) : Object{obj} {};
    virtual ~Hittable() = default;

    virtual Box aabb() const = 0;

    // local normal at point
    virtual Vec4 normal(Vec3 point) const = 0;

    // local distance to object, 0 if no intersection
    virtual Float distance(Ray ray) const = 0;

    std::optional<Hit> intersect(Ray ray) const {
        // transform to local coordinates
        auto tRay = ray.transformed(inverse);
        auto t = distance(tRay);

        if (t < step) { return {}; }

        auto point = tRay.at(t);

        // ray/world coordinates
        auto wpoint = transformVec3(transform, point);
        auto wnormal = Vec3{inverseTranspose * normal(point)};
        Float wt = glm::length(wpoint - ray.eye);

        return Hit{wt, wpoint, glm::normalize(wnormal), this};
    }
};

}  // namespace raytracer
