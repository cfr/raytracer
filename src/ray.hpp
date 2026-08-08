#pragma once

#include "values.hpp"
#include "transforms.hpp"
#include "image.hpp"
#include "camera.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace raytracer {

struct Ray {
    struct SkipInv {};

    Vec3 origin;
    Vec3 dir;
    Vec3 inv;

    explicit Ray(const Vec3& origin, const Vec3& direction)
        : origin{origin}, dir{direction},
          inv{Vec3{1.0} / direction} {}

    // local-space rays, skip inv calculation
    Ray(const Vec3& origin, const Vec3& direction, SkipInv)
        : origin{origin}, dir{direction}, inv{0} {}

    Vec3 at(Float t) const {
        return origin + t*dir;
    }

    Ray transformed(Transform transform) const {
        auto tOrigin = transform * Vec4(origin, 1);
        auto tDir = transform * Vec4(dir, 0);
        return Ray{Vec3{tOrigin / tOrigin.w}, Vec3{tDir}, SkipInv{}};
    }
};

class RayCaster {
    Vec3 eye_;
    Basis basis_;
    Size size_;

    Float halfwidth_;
    Float halfheight_;

    Float thfovy_;  // tan(fovy/2)
    Float thfovx_;  // tan(fovx/2)

 public:
    RayCaster(Camera cam, Size size) : eye_{cam.eye}, basis_{cam}, size_(size) {
        halfwidth_ = static_cast<Float>(size.width) / 2;
        halfheight_ = static_cast<Float>(size.height) / 2;

        auto aspect = halfwidth_/halfheight_;
        auto fovy = glm::radians(cam.fovy);
        thfovy_ = glm::tan(fovy / 2);
        thfovx_ = aspect * thfovy_;
    }

    Ray cast(Point pixel) const {
        return cast(pixel, Vec2{0.5, 0.5});
    }

    Ray cast(Point pixel, Vec2 jitter) const {
        auto x = static_cast<Float>(pixel.x) + jitter.x;
        auto y = static_cast<Float>(pixel.y) + jitter.y;

        auto alpha = thfovx_ * (x - halfwidth_) / halfwidth_;
        auto beta = thfovy_ * (halfheight_ - y) / halfheight_;

        auto dir = alpha * basis_.u + beta * basis_.v - basis_.w;

        return Ray{eye_, glm::normalize(dir)};
    }

    Size size() const { return size_; }
};

}  // namespace raytracer
