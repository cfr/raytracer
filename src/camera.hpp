#pragma once

#include "values.hpp"

#include <glm/geometric.hpp>

namespace raytracer {


struct Camera {
    Vec3 eye = {0, 0, 0};
    Vec3 center = {0, 0, -1};
    Vec3 up = {0, 1, 0};
    Float fovy = 90;
};

struct Basis {
    Vec3 u;
    Vec3 v;
    Vec3 w;

    explicit Basis(Camera cam) {
        w = glm::normalize(cam.eye - cam.center);
        u = glm::normalize(glm::cross(cam.up, w));
        v = glm::cross(w, u);
    }
};

}  // namespace raytracer
