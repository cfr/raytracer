#pragma once

#include "values.hpp"
#include "sampler.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

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

    Sampler sampler(Seed seed) {
        return Sampler(seed, Stratify2D(lightSamples));
    }
};

}  // namespace raytracer
