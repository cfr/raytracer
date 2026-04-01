#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "hit.hpp"

#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

namespace raytracer {

class Triangle: public Hittable {
    Vec3 a_;
    Vec3 b_;
    Vec3 c_;

    Vec3 na_;
    Vec3 nb_;
    Vec3 nc_;

 public:
    Triangle(const Node& n, Vec3 a, Vec3 b, Vec3 c) : Hittable{n}, a_(a), b_(b), c_(c) {
        auto ab = b_ - a_;
        auto ac = c_ - a_;
        auto normal = glm::normalize(glm::cross(ab, ac));
        na_ = normal;
        nb_ = normal;
        nc_ = normal;
    }

    Vec4 normal(Vec3 /*point*/) const override {
        return Vec4{na_, 0};
    }

    Float distance(Ray ray) const override {
        auto edge1 = b_ - a_;
        auto edge2 = c_ - a_;

        auto h = glm::cross(ray.dir, edge2);
        auto a = glm::dot(edge1, h);

        if (std::abs(a) < Hittable::step) {
            return 0;  // parallel
        }

        auto f = 1 / a;
        auto s = ray.eye - a_;

        auto u = f * glm::dot(s, h);
        if (u < 0 || u > 1) {
            return 0;
        }

        auto q = glm::cross(s, edge1);
        auto v = f * glm::dot(ray.dir, q);
        if (v < 0 || u + v > 1) {
            return 0;
        }

        auto t = f * glm::dot(edge2, q);
        return t;
    }
};

}  // namespace raytracer
