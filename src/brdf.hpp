#pragma once

#include "camera.hpp"
#include "material.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <optional>

namespace aktis {

struct Sample {
    Vec3 wi;
    Color f;
    Float pdf;
    bool delta = false;
};

namespace brdf {

// wo/wi are local space

namespace phong {

inline Vec3 mirror(Vec3 w) {
    // n = {0, 0, 1}
    return {-w.x, -w.y, w.z};
}

inline Float lobeCos(Vec3 wo, Vec3 wi) {
    return glm::max(Float(0), glm::dot(mirror(wo), wi));
}

inline Color eval(Material const& m, Vec3 wo, Vec3 wi) {
    if (!sameHemisphere(wo, wi))
        return colors::black;

    Color const diff = m.diffuse / pi;

    Float const rwi = lobeCos(wo, wi);
    Float const s = m.shininess;
    Color const spec = m.specular * ((s + 2) / (2 * pi)) * glm::pow(rwi, s);

    return diff + spec;
}

inline Float pdf(Material const& m, Vec3 wo, Vec3 wi) {
    if (!sameHemisphere(wo, wi))
        return 0;

    Float const diff = (1 - m.t) * glm::abs(wi.z) / pi;

    Float const rwi = lobeCos(wo, wi);
    Float const s = m.shininess;
    Float const spec = m.t * ((s + 1) / (2 * pi)) * glm::pow(rwi, s);

    return diff + spec;
}

inline std::optional<Sample> sample(Material const& m, Vec3 wo, Float uc, Vec2 u2) {
    if (wo.z == 0)
        return {};
    Float const phi = 2 * pi * u2.x;
    Vec3 wi;
    if (uc <= m.t) {
        Float const s = m.shininess;
        Float const cosA = glm::pow(u2.y, 1 / (s + 1));
        Float const sinA = glm::sqrt(glm::max(Float(0), 1 - (cosA * cosA)));
        wi = Basis(mirror(wo)).toWorld({glm::cos(phi) * sinA, glm::sin(phi) * sinA, cosA});
    } else {
        Float const cosT = glm::sqrt(u2.y);
        Float const sinT = glm::sqrt(glm::max(Float(0), 1 - u2.y));
        wi = {glm::cos(phi) * sinT, glm::sin(phi) * sinT, cosT};
    }
    if (!sameHemisphere(wo, wi))
        return {};
    Float const p = pdf(m, wo, wi);
    if (p <= 0)
        return {};
    return Sample{.wi = wi, .f = eval(m, wo, wi), .pdf = p};
}

}  // namespace phong

namespace ggx {

inline Float lobeT(Material const& m) {
    return glm::max(Float(0.25), m.t);
}

inline Float d(Float alpha, Vec3 h) {
    Float const c = h.z;
    if (c <= 0)
        return 0;
    Float const a2 = alpha * alpha;
    Float const k = (c * c * (a2 - 1)) + 1;
    return a2 / (pi * k * k);
}

inline Float g1(Float alpha, Vec3 w) {
    Float const c = w.z;
    if (c <= 0)
        return 0;
    Float const a2 = alpha * alpha;
    return 2 * c / (c + glm::sqrt(a2 + (c * c * (1 - a2))));
}

inline Color fresnel(Material const& m, Vec3 w, Vec3 h) {
    Float const c = glm::clamp(glm::dot(w, h), Float(0), Float(1));
    return m.specular + (Color(1) - m.specular) * glm::pow(1 - c, Float(5));
}

inline Color eval(Material const& m, Vec3 wo, Vec3 wi) {
    if (wo.z <= 0 || wi.z <= 0)
        return colors::black;
    Vec3 const h = halfvec(wo, wi);

    Float const a = m.roughness;
    Color const spec = fresnel(m, wo, h) * g1(a, wi) * g1(a, wo) * d(a, h) / (4 * wi.z * wo.z);
    return m.diffuse / pi + spec;
}

inline Float pdf(Material const& m, Vec3 wo, Vec3 wi) {
    if (wo.z <= 0 || wi.z <= 0)
        return 0;
    Vec3 const h = halfvec(wo, wi);
    Float const woDotH = glm::dot(wo, h);
    if (woDotH <= 0)
        return 0;

    Float const a = m.roughness;
    return ((1 - lobeT(m)) * wi.z / pi) + (lobeT(m) * d(a, h) * h.z / (4 * woDotH));
}

inline std::optional<Sample> sample(Material const& m, Vec3 wo, Float uc, Vec2 u2) {
    if (wo.z <= 0)
        return {};
    Float const phi = 2 * pi * u2.x;
    Vec3 wi;

    if (uc <= lobeT(m)) {
        Float const a2 = m.roughness * m.roughness;
        Float const cosT = glm::sqrt((1 - u2.y) / (1 + u2.y * (a2 - 1)));
        Float const sinT = glm::sqrt(glm::max(Float(0), 1 - (cosT * cosT)));
        Vec3 const h{glm::cos(phi) * sinT, glm::sin(phi) * sinT, cosT};
        wi = glm::reflect(-wo, h);
    } else {
        Float const cosT = glm::sqrt(u2.y);
        Float const sinT = glm::sqrt(glm::max(Float(0), 1 - u2.y));
        wi = {glm::cos(phi) * sinT, glm::sin(phi) * sinT, cosT};
    }

    if (wi.z <= 0)
        return {};
    Float const p = pdf(m, wo, wi);
    if (p <= 0)
        return {};
    return Sample{.wi = wi, .f = eval(m, wo, wi), .pdf = p};
}

}  // namespace ggx

inline Color eval(Material const& m, Vec3 wo, Vec3 wi) {
    switch (m.brdfType) {
    case Type::Phong:
        return phong::eval(m, wo, wi);
    case Type::GGX:
        return ggx::eval(m, wo, wi);
    }
    return colors::black;
}

inline Float pdf(Material const& m, Vec3 wo, Vec3 wi) {
    switch (m.brdfType) {
    case Type::Phong:
        return phong::pdf(m, wo, wi);
    case Type::GGX:
        return ggx::pdf(m, wo, wi);
    }
    return 0;
}

inline std::optional<Sample> sample(Material const& m, Vec3 wo, Float uc, Vec2 u2) {
    switch (m.brdfType) {
    case Type::Phong:
        return phong::sample(m, wo, uc, u2);
    case Type::GGX:
        return ggx::sample(m, wo, uc, u2);
    }
    return {};
}

}  // namespace brdf

}  // namespace aktis
