#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "hit.hpp"

#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

namespace raytracer {

struct Triangle: Hittable {

    Vec3 a_;
    Vec3 b_;
    Vec3 c_;

    Vec3 na_;
    Vec3 nb_;
    Vec3 nc_;

    Vec3 normal(Vec3 /*point*/) override {
        auto tNormal = Vec4{na_, 0};
        auto ntransform = glm::transpose(glm::inverse(transform));
        return glm::normalize(Vec3{ntransform*tNormal});
    }

    Triangle(const Node& n, Vec3 a, Vec3 b, Vec3 c) : Hittable{n}, a_(a), b_(b), c_(c) {
        auto ab = b_ - a_;
        auto ac = c_ - a_;
        auto normal = glm::normalize(glm::cross(ab, ac));
        na_ = normal;
        nb_ = normal;
        nc_ = normal;
    }

    std::optional<Hit> intersect(Ray ray) override {

        Ray tRay = ray.transformed(inverse);

        auto edge1 = b_ - a_;
        auto edge2 = c_ - a_;

        auto h = glm::cross(tRay.dir, edge2);
        auto a = glm::dot(edge1, h);

        if (std::abs(a) < Hittable::step) {
            return {}; // parallel
        }

        auto f = 1 / a;
        auto s = tRay.eye - a_;

        auto u = f * glm::dot(s, h);
        if (u < 0 || u > 1) {
            return {};
        }

        auto q = glm::cross(s, edge1);
        auto v = f * glm::dot(tRay.dir, q);
        if (v < 0 || u + v > 1) {
            return {};
        }

        auto t = f * glm::dot(edge2, q);
        if (t < Hittable::step) {
            return {};  // behind
        }

        auto tPoint = tRay.at(t);

        auto point = transformVec(transform, tPoint);
        auto tNormal = Vec4{na_, 0};
        auto normal = glm::normalize(Vec3{inverseTranspose * tNormal});

        Float wt = glm::length(point - ray.eye);

        return Hit{wt, point, normal};
    }
};

}
