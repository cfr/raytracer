#pragma once

#include "scene.hpp"
#include "image.hpp"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

namespace raytracer {

using vec3 = glm::vec3;
using vec4 = glm::vec4;

struct Sphere: Hittable {
    vec3 center = {0, 0, 0};
    float radius = 1;

    Sphere(const Node& n, vec3 c, float r) : Hittable{n}, center{c}, radius{r} {}

    std::optional<Hit> hit(Ray ray) override {

        auto inv = glm::inverse(transform);
        auto tEye = inv * vec4(ray.eye, 1);
        auto tDir = inv * vec4(ray.dir, 0);
        auto tRay = Ray{vec3{tEye}, vec3{tDir}};
        auto rc = tRay.eye - center;
        auto a = glm::dot(tRay.dir, tRay.dir);
        auto b = 2 * glm::dot(tRay.dir, rc);
        auto c = glm::dot(rc, rc) - radius*radius;

        auto det = b*b - 4*a*c;
        if (det < 0) {
            return {};
        }

        auto r1 = (-b + glm::sqrt(det)) / (2 * a);
        auto r2 = (-b - glm::sqrt(det)) / (2 * a);

        auto t = std::min(r1, r2);
        auto tPoint = tRay.eye + t * tRay.dir;
        auto point = transform * vec4(tPoint, 1);
        auto tNormal = vec4(tPoint - center, 0);
        auto normal = glm::normalize(vec3(glm::transpose(inv)*tNormal));

        return Hit { t, vec3(point/point.w), normal };
    }
};

}
