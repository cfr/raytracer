#pragma once

#define GLM_FORCE_PRECISION_HIGHP_INT
#define GLM_FORCE_PRECISION_HIGH_FLOAT
#define GLM_FORCE_PRECISION_HIGHP_DOUBLE

#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/constants.hpp>

#include <limits>
#include <random>

namespace raytracer {

using Float = glm::float64_t;

static constexpr Float pi = glm::pi<Float>();
static constexpr Float inf = std::numeric_limits<Float>::infinity();

using Vec2 = glm::tvec2<Float, glm::defaultp>;
using Vec3 = glm::tvec3<Float, glm::defaultp>;
using Vec4 = glm::tvec4<Float, glm::defaultp>;

using Transform = glm::tmat4x4<Float, glm::defaultp>;

using Color = Vec3;

namespace colors {
constexpr Color black = Color{0.0};
constexpr Color white = Color{1.0};
}

using Point = glm::ivec2;

using Seed = std::random_device::result_type;

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
