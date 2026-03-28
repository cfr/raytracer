#pragma once

#include "values.hpp"
#include "node.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace raytracer {

struct Hit {
    Float t;
    Vec3 point;
    Vec3 normal;
};

struct Hittable: Node {
    static constexpr Float step = 0.0001; // min distance for intersection

    Hittable(const Node& n) : Node{n} {};
    virtual ~Hittable() = default;

    // local normal at point
    virtual Vec4 normal(Vec3 point) = 0;

    // local distance to object, 0 if not intersection
    virtual Float distance(Ray ray) = 0;

    std::optional<Hit> intersect(Ray ray) {

        // transform to local coordinates
        auto tRay = ray.transformed(inverse);
        auto t = distance(tRay);

        if (t < step) { return {}; }

        auto point = tRay.at(t);

        // ray/world coordinates
        auto wpoint = transformVec(transform, point);
        auto wnormal = Vec3{inverseTranspose * normal(point)};
        Float wt = glm::length(wpoint - ray.eye);

        return Hit{wt, wpoint, glm::normalize(wnormal)};
    }
};

}
