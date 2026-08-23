#pragma once

#include "brdf.hpp"
#include "hittable.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <cstdint>
#include <optional>

namespace raytracer {

/* TODO: switch CRTP to concept:

template<class S>
concept Importance = requires(S const& s, Hit const& h, Vec2 u, Vec3 wi) {
    { s.sample(h, u) } -> std::same_as<Sample>;
    { s.pdf(h, wi)   } -> std::same_as<Float>;
};
*/

template <class T> class Importance {
    Importance() = default;

  public:
    [[nodiscard]] std::optional<Sample> sample(Hit const& hit, Float uc, Vec2 u2) const {
        return static_cast<T const*>(this)->sample_(hit, uc, u2);
    }

    [[nodiscard]] Float pdf(Hit const& hit, Vec3 wi) const {
        return static_cast<T const*>(this)->pdf_(hit, wi);
    }
    friend T;
};

namespace importance {

enum class Type : std::uint8_t {
    Uniform,
    Cosine,
    BRDF  // phong or ggx depending on material
};

struct Uniform final : public Importance<Uniform> {
    static constexpr Float inv2pi = 1 / (2 * pi);

    Uniform() = default;

    [[nodiscard]] std::optional<Sample> sample_(Hit const& hit, Float /*uc*/, Vec2 u2) const {
        auto b = Basis(hit.normal);
        auto wo = b.toLocal(hit.wo);
        Float const phi = 2 * pi * u2.x;
        Float const cosT = u2.y;
        Float const sinT = glm::sqrt(glm::max(Float(0), 1 - (cosT * cosT)));
        Vec3 const wi{glm::cos(phi) * sinT, glm::sin(phi) * sinT, cosT};
        Vec3 const wiWorld = b.toWorld(wi);
        Float const p = pdf(hit, wiWorld);
        if (p <= 0)
            return {};
        auto f = brdf::eval(*hit.object->material, wo, wi);
        return Sample{.wi = wiWorld, .f = f, .pdf = p};
    }

    [[nodiscard]] static Float pdf_(Hit const& hit, Vec3 wi) {
        return sameHemisphere(hit, wi) ? inv2pi : 0;
    }
};

struct Cosine final : public Importance<Cosine> {
    Cosine() = default;

    [[nodiscard]] std::optional<Sample> sample_(Hit const& hit, Float /*uc*/, Vec2 u2) const {
        auto b = Basis(hit.normal);
        auto wo = b.toLocal(hit.wo);
        Float const phi = 2 * pi * u2.x;
        Float const cosT = glm::sqrt(u2.y);
        Float const sinT = glm::sqrt(glm::max(Float(0), 1 - u2.y));
        Vec3 const wi = {glm::cos(phi) * sinT, glm::sin(phi) * sinT, cosT};
        Vec3 const wiWorld = b.toWorld(wi);
        Float const p = pdf(hit, wiWorld);
        if (p <= 0)
            return {};
        auto f = brdf::eval(*hit.object->material, wo, wi);
        return Sample{.wi = wiWorld, .f = f, .pdf = p};
    }

    [[nodiscard]] static Float pdf_(Hit const& hit, Vec3 wi) {
        return sameHemisphere(hit, wi) ? glm::abs(cosTheta(hit, wi)) / pi : 0;
    }
};

struct BRDF final : public Importance<BRDF> {
    BRDF() = default;

    [[nodiscard]] static std::optional<Sample> sample_(Hit const& hit, Float uc, Vec2 u2) {
        auto b = Basis(hit.normal);
        auto wo = b.toLocal(hit.wo);
        auto s = brdf::sample(*hit.object->material, wo, uc, u2);
        if (!s) {
            return {};
        }
        auto wiWorld = b.toWorld(s->wi);
        return Sample{.wi = wiWorld, .f = s->f, .pdf = s->pdf};
    }

    [[nodiscard]] static Float pdf_(Hit const& hit, Vec3 wi) {
        auto b = Basis(hit.normal);
        auto woLocal = b.toLocal(hit.wo);
        auto wiLocal = b.toLocal(wi);
        return brdf::pdf(*hit.object->material, woLocal, wiLocal);
    }
};

inline Float pdfLight(Hit const& light) {
    if (!light.front)
        return 0;  // single-sided emitter
    Float const pa = light.object->pdfArea();
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

}  // namespace raytracer
