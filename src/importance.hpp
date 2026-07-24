#pragma once

#include "values.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <optional>

namespace raytracer {

struct Sample {
    Vec3  wi;
    Color f;
    Float pdf;
    bool  specular = false;
};

class ImportanceSampling {
 public:
    enum class Type: int {
        Uniform,
        Cosine,
        BRDF
    };
    virtual ~ImportanceSampling() = default;
    virtual std::optional<Sample> sample(Vec3 wo, Float uc, Vec2 u2) const = 0;
    virtual Float pdf(Vec3 wo, Vec3 wi) const = 0;
    virtual Color eval(Vec3 wo, Vec3 wi) const = 0;
};

namespace importance {

inline Float cosTheta(Vec3 w) {
    return w.z;
}

inline bool sameHemisphere(Vec3 a, Vec3 b) {
    return a.z * b.z > 0;
}

class Uniform final : public ImportanceSampling {
    static constexpr Float p = 1 / (2 * pi);
    Color albedo_;

 public:
    explicit Uniform(Color albedo) : albedo_(albedo) {}

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
        return sameHemisphere(wo, wi) ? p : 0;
    }

    Color eval(Vec3 wo, Vec3 wi) const override {
        return sameHemisphere(wo, wi) ? albedo_ / pi : colors::black;
    }
};

class Cosine final : public ImportanceSampling {
    Color albedo_;

 public:
    explicit Cosine(Color albedo) : albedo_(albedo) {}

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
        return sameHemisphere(wo, wi) ? albedo_ / pi : colors::black;
    }
};

class BRDF final : public ImportanceSampling {
    // TBD
 public:
    std::optional<Sample> sample(Vec3 wo, Float uc, Vec2 u2) const override {
        return {};
    }

    Float pdf(Vec3 wo, Vec3 wi) const override {
        return 0;
    }

    Color eval(Vec3 wo, Vec3 wi) const override {
        return colors::black;
    }
};

}  // namespace importance

}  // namespace raytracer

