#pragma once

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

namespace raytracer {

using vec3 = glm::vec3;

struct Camera {
    vec3 eye = {0, 0, 0};
    vec3 center = {0, 0, -1};
    vec3 up = {0, 1, 0};
    float fovy = 90;
};

struct Basis {
    vec3 u;
    vec3 v;
    vec3 w;

    Basis(Camera cam) {
        w = glm::normalize(cam.eye - cam.center);
        u = glm::normalize(glm::cross(cam.up, w));
        v = glm::cross(w, u);
    }
};

}
