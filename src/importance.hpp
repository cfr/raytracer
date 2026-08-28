#pragma once

#include "brdf.hpp"
#include "shapes.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <concepts>
#include <cstdint>
#include <optional>
#include <span>

namespace aktis {

template <class S>
concept Importance =
    requires(S const& s, Material const& m, Hit const& h, Float uc, Vec2 u2, Vec3 wi) {
        { s.sample(m, h, uc, u2) } -> std::same_as<std::optional<Sample>>;
        { s.pdf(m, h, wi) } -> std::same_as<Float>;
    };

namespace importance {

enum class Type : std::uint8_t {
    Uniform,
    Cosine,
    BRDF  // phong or ggx depending on material
};

struct Uniform {
    static constexpr Float inv2pi = 1 / (2 * pi);

    [[nodiscard]] static std::optional<Sample> sample(Material const& material, Hit const& hit,
                                                      Float /*uc*/, Vec2 u2) {
        auto b = Basis(hit.normal);
        auto wo = b.toLocal(hit.wo);
        Float const phi = 2 * pi * u2.x;
        Float const cosT = u2.y;
        Float const sinT = glm::sqrt(glm::max(Float(0), 1 - (cosT * cosT)));
        Vec3 const wi{glm::cos(phi) * sinT, glm::sin(phi) * sinT, cosT};
        Vec3 const wiWorld = b.toWorld(wi);
        Float const p = pdf(material, hit, wiWorld);
        if (p <= 0)
            return {};
        auto f = brdf::eval(material, wo, wi);
        return Sample{.wi = wiWorld, .f = f, .pdf = p};
    }

    [[nodiscard]] static Float pdf(Material const& /*material*/, Hit const& hit, Vec3 wi) {
        return sameHemisphere(hit, wi) ? inv2pi : 0;
    }
};

struct Cosine {
    [[nodiscard]] static std::optional<Sample> sample(Material const& material, Hit const& hit,
                                                      Float /*uc*/, Vec2 u2) {
        auto b = Basis(hit.normal);
        auto wo = b.toLocal(hit.wo);
        Float const phi = 2 * pi * u2.x;
        Float const cosT = glm::sqrt(u2.y);
        Float const sinT = glm::sqrt(glm::max(Float(0), 1 - u2.y));
        Vec3 const wi = {glm::cos(phi) * sinT, glm::sin(phi) * sinT, cosT};
        Vec3 const wiWorld = b.toWorld(wi);
        Float const p = pdf(material, hit, wiWorld);
        if (p <= 0)
            return {};
        auto f = brdf::eval(material, wo, wi);
        return Sample{.wi = wiWorld, .f = f, .pdf = p};
    }

    [[nodiscard]] static Float pdf(Material const& /*material*/, Hit const& hit, Vec3 wi) {
        return sameHemisphere(hit, wi) ? glm::abs(cosTheta(hit, wi)) / pi : 0;
    }
};

struct BRDF {
    [[nodiscard]] static std::optional<Sample> sample(Material const& material, Hit const& hit,
                                                      Float uc, Vec2 u2) {
        auto b = Basis(hit.normal);
        auto wo = b.toLocal(hit.wo);
        auto s = brdf::sample(material, wo, uc, u2);
        if (!s) {
            return {};
        }
        auto wiWorld = b.toWorld(s->wi);
        return Sample{.wi = wiWorld, .f = s->f, .pdf = s->pdf};
    }

    [[nodiscard]] static Float pdf(Material const& material, Hit const& hit, Vec3 wi) {
        auto b = Basis(hit.normal);
        auto woLocal = b.toLocal(hit.wo);
        auto wiLocal = b.toLocal(wi);
        return brdf::pdf(material, woLocal, wiLocal);
    }
};

static_assert(Importance<Uniform> && Importance<Cosine> && Importance<BRDF>);

inline Float pdfLight(Hit const& light, std::span<Shape const> shapes) {
    if (!light.front)
        return 0;  // single-sided emitter
    Float const pa = shapes[light.shapeId].pdfArea();
    if (pa <= 0)
        return 0;
    Float const cos = glm::dot(light.normal, light.wo);
    return (light.t * light.t) * pa / cos;
}

// beta = 2
inline Float misWeight(Float p, Float other) {
    if (p <= 0)
        return 0;
    Float const r = other / p;
    return 1 / (1 + r * r);
}

}  // namespace importance

}  // namespace aktis
