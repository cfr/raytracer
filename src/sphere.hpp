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

    vec3 center_ = {0, 0, 0};
    float radius_ = 1;

    Sphere(const Node& n, vec3 center, float radius) : Hittable{n}, center_{center}, radius_{radius} {}

    std::optional<Hit> intersect(Ray ray) override {

        // TODO: extract inv transform of point and normal
        auto inv = glm::inverse(transform);
        auto tEye = inv * vec4(ray.eye, 1);
        auto tDir = inv * vec4(ray.dir, 0);
        auto tRay = Ray{vec3{tEye}, vec3{tDir}};
        auto rc = tRay.eye - center_;
        auto a = glm::dot(tRay.dir, tRay.dir);
        auto b = 2 * glm::dot(tRay.dir, rc);
        auto c = glm::dot(rc, rc) - radius_*radius_;

        auto det = b*b - 4*a*c;
        if (det < 0) { return {}; }

        auto r1 = (-b + glm::sqrt(det)) / (2 * a);
        auto r2 = (-b - glm::sqrt(det)) / (2 * a);

        auto t = std::min(r1, r2);
        if (t < Hittable::step) { return {}; }
        auto tPoint = tRay.at(t);
        auto tNormal = vec4(tPoint - center_, 0);
        auto normal = glm::normalize(vec3(glm::transpose(inv)*tNormal));

        auto point = transformVec(transform, tPoint);
        float wt = glm::length(point - ray.eye);

        return Hit { wt, point, normal };
    }
};

}
