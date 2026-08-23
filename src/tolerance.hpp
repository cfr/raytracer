#pragma once

#include "values.hpp"

#include <glm/common.hpp>

#include <algorithm>

namespace raytracer::tol {

// minimum ray distance / surface offset base (scene units)
constexpr Float tmin = 1e-4;

// offset for secondary ray
inline Float offset(Vec3 p) {
    // TODO: Wächter & Binder's integer-arithmetic offset
    return tmin * std::max({Float(1), glm::abs(p.x), glm::abs(p.y), glm::abs(p.z)});
}

// thresholds on sinAngle() between two vectors
constexpr Float collinear = 1e-6;  // parallel/zero edges
constexpr Float basis = 1e-4;      // ill-conditioned normalize(cross())

// vector normalization tolerance
constexpr Float unit = 1e-4;

}  // namespace raytracer::tol
