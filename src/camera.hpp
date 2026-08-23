#pragma once

#include "tolerance.hpp"
#include "values.hpp"

#include <glm/geometric.hpp>

#include <cassert>
#include <cmath>

namespace raytracer {

struct Camera {
    Vec3 eye = {0, 0, 0};
    Vec3 center = {0, 0, -1};
    Vec3 up = {0, 1, 0};
    Float fovy = 90;
};

struct Basis {
    Vec3 u;
    Vec3 v;
    Vec3 w;

    static bool degenerate(Camera const& cam) {
        return sinAngle(cam.up, cam.eye - cam.center) < tol::basis;
    }

    explicit Basis(Camera const& cam) {
        w = glm::normalize(cam.eye - cam.center);
        Vec3 const t = glm::cross(cam.up, w);
        assert(!degenerate(cam));
        u = t / glm::length(t);
        v = glm::cross(w, u);
    }

    explicit Basis(Vec3 n) : w{n} {
        assert(std::abs(glm::length(n) - 1) < tol::unit);
        // Tom Duff et al, 2017, "Building an Orthonormal Basis, Revisited"
        Float const sign = std::copysign(Float(1), n.z);
        Float const a = -1 / (sign + n.z);
        Float const b = n.x * n.y * a;
        u = Vec3(1 + (sign * n.x * n.x * a), sign * b, -sign * n.x);
        v = Vec3(b, sign + (n.y * n.y * a), -n.y);
    }

    [[nodiscard]] Vec3 toWorld(Vec3 s) const {
        return s.x * u + s.y * v + s.z * w;
    }

    [[nodiscard]] Vec3 toLocal(Vec3 d) const {
        return Vec3(glm::dot(d, u), glm::dot(d, v), glm::dot(d, w));
    }
};

}  // namespace raytracer
