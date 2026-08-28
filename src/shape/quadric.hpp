#pragma once

#include "box.hpp"
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

namespace aktis {

struct Quadric {
    static constexpr Vec2 noclip = {-inf, inf};
    static constexpr Float linearRatio = 1e-4;  // |a/b| below this: solve as linear

    Vec3 q;        // quadratic [A B C]
    Vec3 l;        // linear [G H I]
    Float j;       // constant [J]
    Vec2 yExtent;  // clip range along y: [lo, hi]
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
        return y >= yExtent.x && y <= yExtent.y;
    }

    [[nodiscard]] static Quadric of(Vec3 q, Vec3 l, Float j, Vec2 extent = noclip) {
        if (l.x != 0 || l.z != 0) {
            throw std::invalid_argument("quadric requires y-centered cross-section, g = i = 0");
        }
        if (q.x <= 0 || q.z <= 0) {
            throw std::invalid_argument("quadric requires a > 0 and c > 0");
        }
        if (extent.x > extent.y) {
            throw std::invalid_argument("quadric requires a non-empty y extent");
        }
        if (q.y <= 0 && (!std::isfinite(extent.x) || !std::isfinite(extent.y))) {
            throw std::invalid_argument("quadric with b <= 0 requires a finite y extent");
        }
        Quadric qu;
        qu.q = q;
        qu.l = l;
        qu.j = j;
        qu.yExtent = extent;
        return qu;
    }

    [[nodiscard]] static Quadric sphere(Float r = 1) {
        if (r <= 0)
            throw std::invalid_argument("sphere requires r > 0");
        return of({1, 1, 1}, {0, 0, 0}, -r * r, noclip);
    }
    [[nodiscard]] static Quadric cylinder(Float r = 1, Vec2 extent = {-0.5, 0.5}) {
        if (r <= 0)
            throw std::invalid_argument("cylinder requires r > 0");
        return of({1, 0, 1}, {0, 0, 0}, -r * r, extent);
    }
    [[nodiscard]] static Quadric cone(Float halfAngle = pi / 4, Float h = 1) {
        if (h <= 0)
            throw std::invalid_argument("cone requires h > 0");
        if (halfAngle <= 0 || halfAngle >= pi / 2)
            throw std::invalid_argument("cone requires 0 < halfAngle < pi/2");
        Float const t = glm::tan(halfAngle);
        return of({1, -t * t, 1}, {0, 0, 0}, 0, {-h, 0});
    }
    [[nodiscard]] static Quadric coneRH(Float r = 1, Float h = 1) {
        if (r <= 0)
            throw std::invalid_argument("coneRH requires r > 0");
        if (h <= 0)
            throw std::invalid_argument("coneRH requires h > 0");
        return of({1, -(r * r) / (h * h), 1}, {0, 0, 0}, 0, {-h, 0});
    }
    [[nodiscard]] static Quadric paraboloid(Vec2 extent = {0, 1}) {
        return of({1, 0, 1}, {0, -1, 0}, 0, extent);
    }

    [[nodiscard]] Box aabb() const {
        Float ya = yExtent.x, yb = yExtent.y;

        if (q.y > 0) {
            Float const disc = (l.y * l.y) - (Float(4) * q.y * j);
            if (disc < 0) {
                return Box();
            }
            Float const s = glm::sqrt(disc);
            Float const r0 = (-l.y - s) / (Float(2) * q.y);
            Float const r1 = (-l.y + s) / (Float(2) * q.y);
            ya = glm::max(ya, glm::min(r0, r1));
            yb = glm::min(yb, glm::max(r0, r1));
        } else if (q.y == 0) {
            if (l.y == 0) {
                if (-j < 0) {
                    return Box();
                }
            } else if (l.y > 0) {
                yb = glm::min(yb, -j / l.y);
            } else {
                ya = glm::max(ya, -j / l.y);
            }
        }

        if (ya > yb) {
            return Box();
        }

        auto r2 = [this](Float y) { return -((q.y * y * y) + (l.y * y) + j); };
        Float maxR = glm::max(r2(ya), r2(yb));
        if (q.y > 0) {
            Float const yc = -l.y / (Float(2) * q.y);
            if (yc > ya && yc < yb) {
                maxR = glm::max(maxR, r2(yc));
            }
        }
        if (maxR <= 0) {
            return Box();
        }

        Float const hx = glm::sqrt(maxR / q.x);
        Float const hz = glm::sqrt(maxR / q.z);
        return {.min = Vec3(-hx, ya, -hz), .max = Vec3(hx, yb, hz)};
    }

    [[nodiscard]] static Float pdfArea() {
        return 0;  // not samplable
    }

    [[nodiscard]] Vec4 normal(Vec3 point) const {
        Vec3 const g = Float(2) * q * point + l;
        Float const len2 = glm::dot(g, g);
        if (len2 == 0) {
            return Vec4(0, 1, 0, 0);
        }  // apex
        return Vec4(g * glm::inversesqrt(len2), 0);
    }

    [[nodiscard]] Float tlocal(Ray ray) const {
        Float const a = glm::dot(q, ray.dir * ray.dir);
        Float const b = (Float(2) * glm::dot(q, ray.origin * ray.dir)) + glm::dot(l, ray.dir);
        Float const c = glm::dot(q, ray.origin * ray.origin) + glm::dot(l, ray.origin) + j;

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
            Float const qq = Float(-0.5) * (b + (b < 0 ? -sq : sq));
            t0 = qq / a;
            t1 = (qq != 0) ? c / qq : t0;
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

}  // namespace aktis
