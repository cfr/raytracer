#pragma once

#include "scene.hpp"

#include <glm/vec3.hpp>

namespace raytracer {

using vec3 = glm::vec3;

struct Triangle: Hittable {

    vec3 a_;
    vec3 b_;
    vec3 c_;

    vec3 na_;
    vec3 nb_;
    vec3 nc_;

    Triangle(const Node& n, vec3 a, vec3 b, vec3 c) : Hittable{n}, a_(a), b_(b), c_(c) {
        auto ab = b_ - a_;
        auto ac = c_ - a_;
        auto normal = glm::normalize(glm::cross(ab, ac));
        na_ = normal;
        nb_ = normal;
        nc_ = normal;
    }

    std::optional<Hit> intersect(Ray ray) override {

        auto inv = glm::inverse(transform);
        auto tEye = vec3(inv * vec4(ray.eye, 1));
        auto tDir = glm::normalize(vec3(inv * vec4(ray.dir, 0)));
        Ray tRay{tEye, tDir};

        auto edge1 = b_ - a_;
        auto edge2 = c_ - a_;

        auto h = glm::cross(tRay.dir, edge2);
        auto a = glm::dot(edge1, h);

        if (std::abs(a) < step) {
            return {}; // parallel
        }

        auto f = 1.0f / a;
        auto s = tRay.eye - a_;

        auto u = f * glm::dot(s, h);
        if (u < 0.0f || u > 1.0f) {
            return {};
        }

        auto q = glm::cross(s, edge1);
        auto v = f * glm::dot(tRay.dir, q);
        if (v < 0.0f || u + v > 1.0f) {
            return {};
        }

        auto t = f * glm::dot(edge2, q);
        if (t < step) {
            return {};  // behind
        }

        auto tPoint = tRay.at(t);

        auto point = transformVec(transform, tPoint);
        auto tNormal = vec4(na_, 0);
        auto normal = glm::normalize(vec3(glm::transpose(inv) * tNormal));

        float wt = glm::length(point - ray.eye);

        return Hit{wt, point, normal};
    }
};

}
