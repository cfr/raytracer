#pragma once

#include "hittable.hpp"
#include "ray.hpp"
#include "tolerance.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <cmath>
#include <stdexcept>
#include <utility>

namespace raytracer {

class Quadric : public Hittable {
    static constexpr Vec2 noclip = {-inf, inf};
    static constexpr Float linearRatio = 1e-4;  // |a/b| below this: solve as linear

    Vec3 q_ = {1, 1, 1};     // quadratic [A B C]
    Vec3 l_ = {0, 0, 0};     // linear [G H I]
    Float j_ = -1;           // constant [J]
    Vec2 yExtent_ = noclip;  // clip range along y: [lo, hi]
    /*
        Shape (axis = y)            q                   j             l
        Unit sphere             (1, 1, 1)              -1        (0,  0, 0)
        Ellipsoid               (1/a², 1/b², 1/c²)     -1        (0,  0, 0)
        Cylinder                (1, 0, 1)              -r²       (0,  0, 0)
        Cone                    (1, -tan²θ, 1)          0        (0,  0, 0)
        Hyperboloid, 1-sheet    (1, -1, 1)             -1        (0,  0, 0)
        Hyperboloid, 2-sheet    (1, -1, 1)              1        (0,  0, 0)
        Circular paraboloid     (1, 0, 1)               0        (0, -1, 0)
        Elliptic paraboloid     (1/a², 0, 1/c²)         0        (0, -1, 0)

        A finite aabb requires:
          - bounded radial cross-section, A > 0 and C > 0
          - bounded along axis, B > 0 or a finite y extent.
    */

    [[nodiscard]] bool inExtent(Ray const& ray, Float t) const {
        Float const y = ray.origin.y + (t * ray.dir.y);
        return y >= yExtent_.x && y <= yExtent_.y;
    }

  public:
    static Quadric sphere(MaterialPtr m, Float r = 1, TransformsPtr xf = nullptr) {
        if (r <= 0)
            throw std::invalid_argument("sphere requires r > 0");
        return Quadric(std::move(m), {1, 1, 1}, {0, 0, 0}, -r * r, noclip, std::move(xf));
    }
    static Quadric cylinder(MaterialPtr m, Float r = 1, Vec2 extent = {-0.5, 0.5},
                            TransformsPtr xf = nullptr) {
        if (r <= 0)
            throw std::invalid_argument("cylinder requires r > 0");
        return Quadric(std::move(m), {1, 0, 1}, {0, 0, 0}, -r * r, extent, std::move(xf));
    }
    static Quadric cone(MaterialPtr m, Float halfAngle = pi / 4, Float h = 1,
                        TransformsPtr xf = nullptr) {
        if (h <= 0)
            throw std::invalid_argument("cone requires h > 0");
        if (halfAngle <= 0 || halfAngle >= pi / 2)
            throw std::invalid_argument("cone requires 0 < halfAngle < pi/2");
        Float const t = glm::tan(halfAngle);
        return Quadric(std::move(m), {1, -t * t, 1}, {0, 0, 0}, 0, {-h, 0}, std::move(xf));
    }
    static Quadric coneRH(MaterialPtr m, Float r = 1, Float h = 1, TransformsPtr xf = nullptr) {
        if (r <= 0)
            throw std::invalid_argument("coneRH requires r > 0");
        if (h <= 0)
            throw std::invalid_argument("coneRH requires h > 0");
        return Quadric(std::move(m), {1, -(r * r) / (h * h), 1}, {0, 0, 0}, 0, {-h, 0},
                       std::move(xf));
    }
    static Quadric paraboloid(MaterialPtr m, Vec2 extent = {0, 1}, TransformsPtr xf = nullptr) {
        return Quadric(std::move(m), {1, 0, 1}, {0, -1, 0}, 0, extent, std::move(xf));
    }

    Quadric(MaterialPtr m, Vec3 q, Vec3 l, Float j, Vec2 extent = noclip,
            TransformsPtr xf = nullptr)
        : Hittable{std::move(m), std::move(xf)}, q_{q}, l_{l}, j_{j}, yExtent_{extent} {
        if (l_.x != 0 || l_.z != 0) {
            throw std::invalid_argument("quadric requires y-centered cross-section, g = i = 0");
        }
        if (q_.x <= 0 || q_.z <= 0) {
            throw std::invalid_argument("quadric requires a > 0 and c > 0");
        }
        if (yExtent_.x > yExtent_.y) {
            throw std::invalid_argument("quadric requires a non-empty y extent");
        }
        if (q_.y <= 0 && (!std::isfinite(yExtent_.x) || !std::isfinite(yExtent_.y))) {
            throw std::invalid_argument("quadric with b <= 0 requires a finite y extent");
        }
    }

    [[nodiscard]] Box aabb() const override {
        Float ya = yExtent_.x, yb = yExtent_.y;

        if (q_.y > 0) {
            Float const disc = (l_.y * l_.y) - (Float(4) * q_.y * j_);
            if (disc < 0) {
                return Box();
            }
            Float const s = glm::sqrt(disc);
            Float const r0 = (-l_.y - s) / (Float(2) * q_.y);
            Float const r1 = (-l_.y + s) / (Float(2) * q_.y);
            ya = glm::max(ya, glm::min(r0, r1));
            yb = glm::min(yb, glm::max(r0, r1));
        } else if (q_.y == 0) {
            if (l_.y == 0) {
                if (-j_ < 0) {
                    return Box();
                }
            } else if (l_.y > 0) {
                yb = glm::min(yb, -j_ / l_.y);
            } else {
                ya = glm::max(ya, -j_ / l_.y);
            }
        }

        if (ya > yb) {
            return Box();
        }

        auto r2 = [this](Float y) { return -((q_.y * y * y) + (l_.y * y) + j_); };
        Float maxR = glm::max(r2(ya), r2(yb));
        if (q_.y > 0) {
            Float const yc = -l_.y / (Float(2) * q_.y);
            if (yc > ya && yc < yb) {
                maxR = glm::max(maxR, r2(yc));
            }
        }
        if (maxR <= 0) {
            return Box();
        }

        Float const hx = glm::sqrt(maxR / q_.x);
        Float const hz = glm::sqrt(maxR / q_.z);
        Box const local{.min = Vec3(-hx, ya, -hz), .max = Vec3(hx, yb, hz)};

        return transforms ? local.transformed(*transforms) : local;
    }

    [[nodiscard]] Vec4 normal(Vec3 point) const override {
        Vec3 const g = Float(2) * q_ * point + l_;
        Float const len2 = glm::dot(g, g);
        if (len2 == 0) {
            return Vec4(0, 1, 0, 0);
        }  // apex
        return Vec4(g * glm::inversesqrt(len2), 0);
    }

    [[nodiscard]] Float tlocal(Ray ray) const override {
        Float const a = glm::dot(q_, ray.dir * ray.dir);
        Float const b = (Float(2) * glm::dot(q_, ray.origin * ray.dir)) + glm::dot(l_, ray.dir);
        Float const c = glm::dot(q_, ray.origin * ray.origin) + glm::dot(l_, ray.origin) + j_;

        Float t0, t1;
        if (glm::abs(a) <= linearRatio * glm::abs(b)) {
            if (b == 0) {
                return 0;
            }
            t0 = t1 = -c / b;
        } else {
            Float const disc = (b * b) - (Float(4) * a * c);
            if (disc < 0) {
                return 0;
            }
            Float const sq = glm::sqrt(disc);
            Float const q = Float(-0.5) * (b + (b < 0 ? -sq : sq));
            t0 = q / a;
            t1 = (q != 0) ? c / q : t0;
            if (t0 > t1) {
                std::swap(t0, t1);
            }
        }

        if (t0 > tol::tmin && inExtent(ray, t0)) {
            return t0;
        }
        if (t1 > tol::tmin && inExtent(ray, t1)) {
            return t1;
        }
        return 0;
    }
};

}  // namespace raytracer
