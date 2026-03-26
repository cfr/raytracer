#pragma once

#include "node.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace raytracer {

struct Hit {
    float t;
    vec3 point;
    vec3 normal;
};

struct Hittable: Node {
    static constexpr float step = 0.0001f; // min distance for intersection

    Hittable(const Node& n) : Node{n} {};
    virtual ~Hittable() = default;

    virtual std::optional<Hit> intersect(Ray ray) = 0;
    // TODO: local normal, local intersect
    virtual vec3 normal(vec3 point) = 0;
};

}
