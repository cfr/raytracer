#pragma once

#include "values.hpp"
#include "brdf.hpp"
#include "hittable.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <optional>
#include <cassert>

namespace raytracer {

/* TODO: switch CRTP to concept:

template<class S>
concept Importance = requires(const S& s, const Hit& h, Vec2 u, Vec3 wi) {
    { s.sample(h, u) } -> std::same_as<Sample>;
    { s.pdf(h, wi)   } -> std::same_as<Float>;
};
*/

template <class T> class Importance {
 public:
    std::optional<Sample> sample(const Hit& hit, Float uc, Vec2 u2) const {
        return static_cast<const T*>(this)->sample_(hit, uc, u2);
    }

    Float pdf(const Hit& hit, Vec3 wi) const {
        return static_cast<const T*>(this)->pdf_(hit, wi);
    }
};

namespace importance {

enum class Type: int {
    Uniform,
    Cosine,
    BRDF  // phong or ggx depending on material
};

struct Uniform final : public Importance<Uniform> {
    static constexpr Float inv2pi = 1 / (2 * pi);

    std::optional<Sample> sample_(const Hit& hit, Float /*uc*/, Vec2 u2) const {
        auto b = Basis(hit.normal);
        auto wo = b.toLocal(hit.wo);
        Float phi = 2 * pi * u2.x;
        Float cosT = u2.y;
        Float sinT = glm::sqrt(glm::max(Float(0), 1 - cosT * cosT));
        Vec3 wi{glm::cos(phi) * sinT, glm::sin(phi) * sinT, cosT};
        Vec3 wiWorld = b.toWorld(wi);
        Float p = pdf(hit, wiWorld);
        if (p <= 0) return {};
        auto f = brdf::eval(hit.object->material, wo, wi);
        return Sample{wiWorld, f, p};
    }

    Float pdf_(const Hit& hit, Vec3 wi) const {
        return sameHemisphere(hit, wi) ? inv2pi : 0;
    }
};

struct Cosine final : public Importance<Cosine> {
    std::optional<Sample> sample_(const Hit& hit, Float /*uc*/, Vec2 u2) const {
        auto b = Basis(hit.normal);
        auto wo = b.toLocal(hit.wo);
        Float phi = 2 * pi * u2.x;
        Float cosT = glm::sqrt(u2.y);
        Float sinT = glm::sqrt(glm::max(Float(0), 1 - u2.y));
        Vec3 wi = {glm::cos(phi)*sinT, glm::sin(phi)*sinT, cosT};
        Vec3 wiWorld = b.toWorld(wi);
        Float p = pdf(hit, wiWorld);
        if (p <= 0) return {};
        auto f = brdf::eval(hit.object->material, wo, wi);
        return Sample{wiWorld, f, p};
    }

    Float pdf_(const Hit& hit, Vec3 wi) const {
        return sameHemisphere(hit, wi) ? glm::abs(cosTheta(hit, wi)) / pi : 0;
    }
};

struct BRDF final : public Importance<BRDF> {
    std::optional<Sample> sample_(const Hit& hit, Float uc, Vec2 u2) const {
        auto b = Basis(hit.normal);
        auto wo = b.toLocal(hit.wo);
        auto s = brdf::sample(hit.object->material, wo, uc, u2);
        if (!s) {
            return {};
        }
        auto wiWorld = b.toWorld(s->wi);
        return Sample{wiWorld, s->f, s->pdf};
    }

    Float pdf_(const Hit& hit, Vec3 wi) const {
        auto b = Basis(hit.normal);
        auto woLocal = b.toLocal(hit.wo);
        auto wiLocal = b.toLocal(wi);
        return brdf::pdf(hit.object->material, woLocal, wiLocal);
    }
};

inline Float pdfLight(const Hit& light) {
    if (!light.front) return 0;  // single-sided light
    Float pa = light.object->pdfArea();
    if (pa <= 0) return 0;
    Float cos = glm::dot(light.normal, light.wo);
    return (light.t * light.t) * pa / cos;
}

// beta = 2
inline Float misWeight(Float p, Float other) {
    if (p <= 0) return 0;
    const Float r = other / p;
    return 1 / (1 + r * r);
}

}  // namespace importance

}  // namespace raytracer

