#pragma once

#include "values.hpp"
#include "sampler.hpp"
#include "importance.hpp"
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
    ImportanceSampling::Type importanceSampling = ImportanceSampling::Type::Cosine;

    Sampler sampler(Seed seed) {
        return Sampler(seed, Stratify2D(lightSamples), stratify);
    }

    std::optional<Sample> sample(const Material& m, Vec3 wo, Vec2 u2) const {
        switch (importanceSampling) {
            case ImportanceSampling::Type::Uniform:
                return importance::Uniform(m.diffuse).sample(wo, 0, u2);
            case ImportanceSampling::Type::Cosine:
                return importance::Cosine(m.diffuse).sample(wo, 0, u2);
            case ImportanceSampling::Type::BRDF:
                return importance::BRDF().sample(wo, 0, u2);
        }
    }
};

}  // namespace raytracer
