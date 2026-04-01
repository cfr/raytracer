#pragma once

#define GLM_FORCE_PRECISION_HIGHP_INT
#define GLM_FORCE_PRECISION_HIGH_FLOAT
#define GLM_FORCE_PRECISION_HIGHP_DOUBLE

#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>

namespace raytracer {

using Float = glm::float64_t;

using Vec3 = glm::tvec3<Float, glm::defaultp>;
using Vec4 = glm::tvec4<Float, glm::defaultp>;

using Transform = glm::tmat4x4<Float, glm::defaultp>;

using Color = Vec3;
using ColorA = Vec4;

using Point = glm::ivec2;

struct Size {
    size_t width = 0;
    size_t height = 0;
};

}  // namespace raytracer
