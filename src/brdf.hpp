#pragma once

#include "values.hpp"
#include "object.hpp"
#include "camera.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace raytracer {

struct Sample {
    Vec3 wi;
    Color f;
    Float pdf;
    bool delta = false;
};

namespace brdf {

namespace phong {

inline Vec3 mirror(Vec3 w) {
    // local space, n={0,0,1}
    return {-w.x, -w.y, w.z};
}

inline Float lobeCos(Vec3 wo, Vec3 wi) {
    return glm::max(Float(0), glm::dot(mirror(wo), wi));
}

Color eval(const Material& m, Vec3 wo, Vec3 wi) {
    if (!sameHemisphere(wo, wi)) return colors::black;

    Color diff = m.diffuse / pi;

    Float rwi = lobeCos(wo, wi);
    Float s = m.shininess;
    Color spec = m.specular * ((s + 2) / (2 * pi)) * glm::pow(rwi, s);

    return diff + spec;
}

Float pdf(const Material& m, Vec3 wo, Vec3 wi) {
    if (!sameHemisphere(wo, wi)) return 0;

    Float diff = (1 - m.t) * glm::abs(wi.z) / pi;

    Float rwi = lobeCos(wo, wi);
    Float s = m.shininess;
    Float spec = m.t * ((s + 1) / (2 * pi)) * std::pow(rwi, s);

    return diff + spec;
}

std::optional<Sample> sample(const Material& m, Vec3 wo, Float uc, Vec2 u2) {
    if (wo.z == 0) return {};
    Float phi = 2 * pi * u2.x;
    Vec3 wi;
    if (uc < m.t) {
        Float s = m.shininess;
        Float cosA = std::pow(u2.y, 1 / (s + 1));
        Float sinA = std::sqrt(glm::max(Float(0), 1 - cosA * cosA));
        wi = Basis(mirror(wo)).toWorld({std::cos(phi) * sinA, std::sin(phi) * sinA, cosA});
    } else {
        Float cosT = std::sqrt(u2.y);
        Float sinT = std::sqrt(glm::max(Float(0), 1 - u2.y));
        wi = {std::cos(phi) * sinT, std::sin(phi) * sinT, cosT};
        if (wo.z < 0) wi.z = -wi.z;
    }
    if (!sameHemisphere(wo, wi)) return {};
    Float p = pdf(m, wo, wi);
    if (p <= 0) return {};
    return Sample{wi, eval(m, wo, wi), p};
}

} // namespace phong


namespace ggx {

Color eval(const Material& m, Vec3 wo, Vec3 wi) {
    return colors::black;
}

Float pdf(const Material& m, Vec3 wo, Vec3 wi) {
    return 0;
}

std::optional<Sample> sample(const Material& m, Vec3 wo, Float uc, Vec2 u2) {
    return {};
}

} // namespace ggx

Color eval(const Material& m, Vec3 wo, Vec3 wi) {
    switch (m.brdfType) {
    case Type::Phong:
        return phong::eval(m, wo, wi);
    case Type::GGX:
        return ggx::eval(m, wo, wi);
    }
}

Float pdf(const Material& m, Vec3 wo, Vec3 wi) {
    switch (m.brdfType) {
    case Type::Phong:
        return phong::pdf(m, wo, wi);
    case Type::GGX:
        return ggx::pdf(m, wo, wi);
    }
}

std::optional<Sample> sample(const Material& m, Vec3 wo, Float uc, Vec2 u2) {
    switch (m.brdfType) {
    case Type::Phong:
        return phong::sample(m, wo, uc, u2);
    case Type::GGX:
        return ggx::sample(m, wo, uc, u2);
    }
}

}  // namespace brdf

/*class BRDF {
 public:
    enum class Type : int {
        Phong,
        GGX
    }
    virtual ~BRDF() = default;
    virtual Color eval(Vec3 wo, Vec3 wi) const = 0;
};

class Phong final : public BRDF {
    Material material_;
 public:
    Color eval(Vec3 wo, Vec3 wi) const {
        Color diff = material.diffuse / pi;

        Vec3 r = glm::reflect(-wi, n);
        Float rDotV = std::max<Float>(glm::dot(r, wo), 0);
        Float s = material.shininess;
        Color spec = material.specular * ((s + 2) / (2 * pi)) * std::pow(rDotV, s);

        return diff + spec;
    }
}*/

}  // namespace raytracer

