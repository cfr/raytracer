#pragma once

#include "values.hpp"
#include "sampler.hpp"
#include "importance.hpp"
#include "brdf.hpp"
#include "object.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <optional>

namespace raytracer {

struct Integrator {
    enum class Type: int {
        Whitted,
        AnalyticDirect,
        Direct,
        PathTracer
    };
    Type type = Type::Whitted;
    size_t lightSamples = 1;
    size_t samplesPerPixel = 1;
    bool stratify = false; // light
    bool nextEvent = false;
    bool russianRoulette = false;
    Importance::Type importanceSampling = Importance::Type::Cosine;

    Sampler sampler(Seed seed) {
        return Sampler(seed, Stratify2D(lightSamples), stratify);
    }

    std::optional<Sample> sample(const Material& m, Vec3 wo, Float uc, Vec2 u2) const {
        switch (importanceSampling) {
        case Importance::Type::Uniform:
            return importance::Uniform(m).sample(wo, uc, u2);
        case Importance::Type::Cosine:
            return importance::Cosine(m).sample(wo, uc, u2);
        case Importance::Type::BRDF:
            return importance::BRDF(m).sample(wo, uc, u2);
        }
    }
};

}  // namespace raytracer
