#pragma once

#include "hittable.hpp"
#include "ray.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/geometric.hpp>

namespace aktis {

class Triangle : public Hittable {
    Vec3 a_;
    Vec3 b_;
    Vec3 c_;

    Vec3 edge1_;  // b - a
    Vec3 edge2_;  // c - a

    Vec3 na_;
    Vec3 nb_;
    Vec3 nc_;

  public:
    Triangle(MaterialPtr m, Vec3 a, Vec3 b, Vec3 c)
        : Hittable{std::move(m)}, a_(a), b_(b), c_(c), edge1_(b - a), edge2_(c - a) {
        auto normal = glm::normalize(glm::cross(edge1_, edge2_));
        na_ = normal;
        nb_ = normal;
        nc_ = normal;
    }

    [[nodiscard]] Box aabb() const override {
        return {.min = glm::min(a_, glm::min(b_, c_)), .max = glm::max(a_, glm::max(b_, c_))};
    }

    [[nodiscard]] Vec4 normal(Vec3 /*point*/) const override {
        return Vec4{na_, 0};
    }

    [[nodiscard]] Float tlocal(Ray ray) const override {
        auto h = glm::cross(ray.dir, edge2_);
        auto a = glm::dot(edge1_, h);

        if (a == 0) {
            return 0;
        }

        auto f = 1 / a;
        auto s = ray.origin - a_;

        auto u = f * glm::dot(s, h);
        if (u < 0 || u > 1) {
            return 0;
        }

        auto q = glm::cross(s, edge1_);
        auto v = f * glm::dot(ray.dir, q);
        if (v < 0 || u + v > 1) {
            return 0;
        }

        auto t = f * glm::dot(edge2_, q);
        return t;
    }
};

}  // namespace aktis
