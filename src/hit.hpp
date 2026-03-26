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

    virtual std::optional<Hit> intersect(Ray ray) = 0;
    // TODO: local normal, local intersect
    virtual Vec3 normal(Vec3 point) = 0;
};

}
