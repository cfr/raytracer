#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "hit.hpp"

#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

namespace raytracer {

class Sphere: public Hittable {

    Vec3 center_ = {0, 0, 0};
    Float radius_ = 1;

public:

    Sphere(const Node& n, Vec3 center, Float radius) : Hittable{n}, center_{center}, radius_{radius} {}

    Vec4 normal(Vec3 point) override {
        return Vec4{point - center_, 0};
    }

    Float distance(Ray ray) override {

        auto rc = ray.eye - center_;
        auto a = glm::dot(ray.dir, ray.dir);
        auto b = 2 * glm::dot(ray.dir, rc);
        auto c = glm::dot(rc, rc) - radius_*radius_;

        auto det = b*b - 4*a*c;
        if (det < 0) { return 0; }

        auto r1 = (-b + glm::sqrt(det)) / (2 * a);
        auto r2 = (-b - glm::sqrt(det)) / (2 * a);

        auto t = std::min(r1, r2);
        return t;
    }
};

}
