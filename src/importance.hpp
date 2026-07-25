#pragma once

#include "values.hpp"
#include "brdf.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <optional>

namespace raytracer {

class Importance {
 public:
    enum class Type: int {
        Uniform,
        Cosine,
        BRDF // phong or ggx depending on material
    };
    virtual ~Importance() = default;
    virtual std::optional<Sample> sample(Vec3 wo, Float uc, Vec2 u2) const = 0;
    virtual Float pdf(Vec3 wo, Vec3 wi) const = 0;
    virtual Color eval(Vec3 wo, Vec3 wi) const = 0;
};

namespace importance {

inline Float cosTheta(Vec3 w) {
    return w.z;  // local
}

class Uniform final : public Importance {
    static constexpr Float inv2pi = 1 / (2 * pi);
    const Material& m_;

 public:
    explicit Uniform(const Material& m) : m_(m) {}

    std::optional<Sample> sample(Vec3 wo, Float /*uc*/, Vec2 u2) const override {
        if (wo.z == 0) return {};
        Float phi = 2 * pi * u2.x;
        Float cosT = u2.y;
        Float sinT = glm::sqrt(glm::max(Float(0), 1 - cosT * cosT));
        Vec3 wi{glm::cos(phi) * sinT, glm::sin(phi) * sinT, cosT};
        if (wo.z < 0) wi.z = -wi.z;
        return Sample{wi, eval(wo, wi), pdf(wo, wi)};
    }

    Float pdf(Vec3 wo, Vec3 wi) const override {
        return sameHemisphere(wo, wi) ? inv2pi : 0;
    }

    Color eval(Vec3 wo, Vec3 wi) const override {
        return brdf::eval(m_, wo, wi);
    }
};

class Cosine final : public Importance {
    const Material& m_;
 public:
    explicit Cosine(const Material& m) : m_(m) {}

    std::optional<Sample> sample(Vec3 wo, Float /*uc*/, Vec2 u2) const override {
        if (wo.z == 0) return {};
        Float phi = 2 * pi * u2.x;
        Float cosT = glm::sqrt(u2.y);
        Float sinT = glm::sqrt(glm::max(Float(0), 1 - u2.y));
        Vec3 wi = {glm::cos(phi)*sinT, glm::sin(phi)*sinT, cosT};
        if (wo.z < 0) wi.z = -wi.z;
        return Sample{wi, eval(wo, wi), pdf(wo, wi)};
    }

    Float pdf(Vec3 wo, Vec3 wi) const override {
        return sameHemisphere(wo, wi) ? glm::abs(cosTheta(wi)) / pi : 0;
    }

    Color eval(Vec3 wo, Vec3 wi) const override {
        return brdf::eval(m_, wo, wi);
    }
};

class BRDF final : public Importance {
    const Material& m_;
 public:
    explicit BRDF(const Material& m) : m_(m) {}

    Color eval(Vec3 wo, Vec3 wi) const override {
        return brdf::eval(m_, wo, wi);
    }

    Float pdf(Vec3 wo, Vec3 wi) const override {
        return brdf::pdf(m_, wo, wi);
    }

    std::optional<Sample> sample(Vec3 wo, Float uc, Vec2 u2) const override {
        return brdf::sample(m_, wo, uc, u2);
    }
};

}  // namespace importance

}  // namespace raytracer

