#pragma once

#define GLM_FORCE_PRECISION_HIGHP_INT
#define GLM_FORCE_PRECISION_HIGHP_FLOAT
#define GLM_FORCE_PRECISION_HIGHP_DOUBLE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_precision.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>

namespace aktis {

#ifdef USE_FLOAT32
using Float = glm::float32_t;
#else
using Float = glm::float64_t;
#endif

static constexpr Float pi = glm::pi<Float>();
static constexpr Float inf = std::numeric_limits<Float>::infinity();

using Vec2 = glm::tvec2<Float, glm::defaultp>;
using Vec3 = glm::tvec3<Float, glm::defaultp>;
using Vec4 = glm::tvec4<Float, glm::defaultp>;

using Color = Vec3;

namespace colors {
constexpr Color black = Color{0};
constexpr Color white = Color{1};
}  // namespace colors

using Point = glm::ivec2;

using Seed = std::uint64_t;

struct Size {
    size_t width = 0;
    size_t height = 0;
};

struct Hittable;

struct Hit {
    Vec3 wo;
    Vec3 point;
    Vec3 normal;
    Float t = inf;
    Hittable const* object = nullptr;
    bool front = true;
};

inline Vec3 halfvec(Vec3 a, Vec3 b) {
    Vec3 const h = a + b;
    return glm::dot(h, h) == 0 ? Vec3{0} : glm::normalize(h);
}

inline Color gamma(Color c, Float g) {
    return glm::pow(glm::max(c, colors::black), Color{1 / g});
}

inline int maxAxis(Vec3 v) {
    if (v.x >= v.y && v.x >= v.z) {
        return 0;
    }
    return v.y >= v.z ? 1 : 2;
}

inline bool sameHemisphere(Hit const& h, Vec3 b) {
    return glm::dot(h.normal, h.wo) * glm::dot(h.normal, b) > 0;
}

inline Float cosTheta(Hit const& h, Vec3 w) {
    return glm::dot(h.normal, w);
}

inline Float sinAngle(Vec3 a, Vec3 b) {
    Float const la = glm::length(a), lb = glm::length(b);
    if (la == 0 || lb == 0) {
        return 0;
    }
    return glm::length(glm::cross(a, b)) / (la * lb);
}

// local space

inline bool sameHemisphere(Vec3 a, Vec3 b) {
    return a.z * b.z > 0;
}

inline Float cosTheta(Vec3 w) {
    return w.z;
}

}  // namespace aktis
