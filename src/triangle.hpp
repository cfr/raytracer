#pragma once

#include "scene.hpp"

#include <glm/vec3.hpp>

namespace raytracer {

using vec3 = glm::vec3;

struct Triangle: Hittable {
    std::array<int, 3> indices;
    // std::array<vec3, 3> normals;

    Triangle(const Node& n, std::array<int, 3> ids) : Hittable{n}, indices{ids} {}

    std::optional<Hit> hit(Ray /*ray*/) override {
        return {};
    }
};

}
