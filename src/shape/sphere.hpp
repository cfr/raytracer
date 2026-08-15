#pragma once

#include "values.hpp"
#include "tolerance.hpp"
#include "ray.hpp"
#include "hittable.hpp"

#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

namespace raytracer {

class Sphere: public Hittable {
    Vec3 center_ = {0, 0, 0};
    Float radius_ = 1;

  public:
    Sphere(MaterialPtr m, Vec3 center, Float radius, TransformsPtr xf = nullptr)
        : Hittable{std::move(m), std::move(xf)}, center_{center}, radius_{radius} {}

    Box aabb() const override {
        Box local{center_ - Vec3{radius_}, center_ + Vec3{radius_}};
        return transforms ? local.transformed(*transforms) : local;
    }

    Vec4 normal(Vec3 point) const override {
        return Vec4{point - center_, 0};
    }

    Float tlocal(Ray ray) const override {
        auto rc = ray.origin - center_;
        auto a  = glm::dot(ray.dir, ray.dir);
        auto b  = 2 * glm::dot(ray.dir, rc);
        auto c  = glm::dot(rc, rc) - radius_*radius_;

        auto disc = b*b - 4*a*c;
        if (disc < 0) { return 0; }

        auto sq   = glm::sqrt(disc);
        auto near = (-b - sq) / (2 * a);
        auto far  = (-b + sq) / (2 * a);

        if (near > tol::tmin) { return near; }
        if (far  > tol::tmin) { return far; }
        return 0;
    }
};

}  // namespace raytracer
