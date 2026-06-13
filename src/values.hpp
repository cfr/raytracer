#pragma once

#define GLM_FORCE_PRECISION_HIGHP_INT
#define GLM_FORCE_PRECISION_HIGH_FLOAT
#define GLM_FORCE_PRECISION_HIGHP_DOUBLE

#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>

namespace raytracer {

using Float = glm::float64_t;

static constexpr Float inf = std::numeric_limits<Float>::infinity();

using Vec3 = glm::tvec3<Float, glm::defaultp>;
using Vec4 = glm::tvec4<Float, glm::defaultp>;

using Transform = glm::tmat4x4<Float, glm::defaultp>;

using Color = Vec4;
using Color3 = Vec3;

using Point = glm::ivec2;

struct Size {
    size_t width = 0;
    size_t height = 0;
};

inline Vec3 transformVec3(Transform m, Vec3 v) {
    auto v4 = Vec4(v, 1);
    auto tv = m * v4;
    return {tv / tv.w};
}

}  // namespace raytracer
