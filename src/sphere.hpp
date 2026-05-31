#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "hit.hpp"

#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

#include <algorithm>

namespace raytracer {

class Sphere: public Hittable {
    Vec3 center_ = {0, 0, 0};
    Float radius_ = 1;

 public:
    Sphere(const Object& obj, Vec3 center, Float radius) : Hittable{obj}, center_{center}, radius_{radius} {}

    Box aabb() const override {
        Vec3 lo = center_ - Vec3{radius_};
        Vec3 hi = center_ + Vec3{radius_};
        Box w;
        for (int i = 0; i < 8; ++i) {
            Vec3 corner{ (i&1) ? hi.x : lo.x,
                         (i&2) ? hi.y : lo.y,
                         (i&4) ? hi.z : lo.z };
            Vec3 p = transformVec3(transform, corner);
            w.min = glm::min(w.min, p);
            w.max = glm::max(w.max, p);
        }
        return w;
    }

    Vec4 normal(Vec3 point) const override {
        return Vec4{point - center_, 0};
    }

    Float distance(Ray ray) const override {
        auto rc = ray.eye - center_;
        auto a  = glm::dot(ray.dir, ray.dir);
        auto b  = 2 * glm::dot(ray.dir, rc);
        auto c  = glm::dot(rc, rc) - radius_*radius_;

        auto det = b*b - 4*a*c;
        if (det < 0) { return 0; }

        auto sq   = glm::sqrt(det);
        auto near = (-b - sq) / (2 * a);
        auto far  = (-b + sq) / (2 * a);

        if (near > step) { return near; }
        if (far  > step) { return far; }
        return 0;
    }
};

}  // namespace raytracer
