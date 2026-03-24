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

    // std::array<int, 3> indices;
    // std::array<vec3, 3> normals;

    Triangle(const Node& n, vec3 a, vec3 b, vec3 c) : Hittable{n}, a_(a), b_(b), c_(c) {
        auto ab = b_ - a_;
        auto ac = c_ - a_;
        auto normal = glm::normalize(glm::cross(ab, ac));
        na_ = normal;
        nb_ = normal;
        nc_ = normal;
    }

    bool contains(vec3 point) {
        auto u = b_ - a_;
        auto v = c_ - a_;
        auto w = point - a_;

        auto vw = glm::cross(v, w);
        auto vu = glm::cross(v, u);
        if (glm::dot(vw, vu) < 0) {
            return false;
        }

        auto uw = glm::cross(u, w);
        auto uv = glm::cross(u, v);

        if (glm::dot(uw, uv) < 0) {
            return false;
        }

        float d = glm::length(uv);
        float r = glm::length(vw) / d;
        float t = glm::length(uw) / d;

        return r + t <= 1;
    }

    std::optional<Hit> intersect(Ray ray) override {
        float denom = glm::dot(ray.dir, na_);
        if (std::abs(denom) < step) { return {}; } // parallel

        float t = (glm::dot(a_, na_) - glm::dot(ray.eye, na_)) / denom;
        if (t < step) { return {}; }; // behind

        auto p = ray.at(t);
        if (contains(p)) {
            return Hit { t, p, na_ };
        }
        return {};
    }
};

}
