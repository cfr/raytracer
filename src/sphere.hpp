#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "hit.hpp"

#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

namespace raytracer {

struct Sphere: Hittable {

    Vec3 center_ = {0, 0, 0};
    Float radius_ = 1;

    Sphere(const Node& n, Vec3 center, Float radius) : Hittable{n}, center_{center}, radius_{radius} {}

    Vec3 normal(Vec3 point) override {
        auto tNormal = Vec4{point - center_, 0};
        // TODO: store inverse and transposed transforms
        auto ntransform = glm::transpose(glm::inverse(transform));
        return glm::normalize(Vec3{ntransform*tNormal});
    }

    std::optional<Hit> intersect(Ray ray) override {

        // TODO: extract inv transform of point and normal
        auto tRay = ray.transformed(inverse);
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
        auto tNormal = Vec4{tPoint - center_, 0};
        auto normal = glm::normalize(Vec3{inverseTranspose*tNormal});

        auto point = transformVec(transform, tPoint);
        float wt = glm::length(point - ray.eye);

        return Hit { wt, point, normal };
    }
};

}
