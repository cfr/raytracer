#pragma once

#include "values.hpp"
#include "image.hpp"
#include "camera.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace raytracer {

struct Ray {
    Vec3 eye = {0, 0, 0};
    Vec3 dir = {0, 0, -1};

    Vec3 at(Float t) const {
        return eye + t*dir;
    }

    Ray transformed(Transform transform) const {
        auto tEye = transform * Vec4(eye, 1);
        auto tDir = transform * Vec4(dir, 0);
        return Ray{Vec3{tEye/tEye.w}, Vec3{tDir}};
    }
};

class RayCaster {
    Vec3 eye_;
    Basis basis_;
    Size size_;

    Float hwidth_;   // width/2
    Float hheight_;  // height/2

    Float thfovy_;  // tan(fovy/2)
    Float thfovx_;  // tan(fovx/2)

 public:
    RayCaster(Camera cam, Size size) : eye_{cam.eye}, basis_{cam}, size_(size) {
        hwidth_ = static_cast<Float>(size.width) / 2;
        hheight_ = static_cast<Float>(size.height) / 2;

        auto aspect = hwidth_/hheight_;
        auto fovy = glm::radians(cam.fovy);
        thfovy_ = glm::tan(fovy / 2);
        thfovx_ = aspect * thfovy_;
    }

    Ray cast(Point pixel) const {
        auto x = static_cast<Float>(pixel.x) + 0.5;
        auto y = static_cast<Float>(pixel.y) + 0.5;

        auto alpha = thfovx_ * (x - hwidth_) / hwidth_;
        auto beta = thfovy_ * (hheight_ - y) / hheight_;

        auto dir = alpha * basis_.u + beta * basis_.v - basis_.w;

        return {eye_, glm::normalize(dir)};
    }

    Size size() const { return size_; }

    Vec3 eye() const { return eye_; }
};

}  // namespace raytracer
