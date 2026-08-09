#pragma once

#include "values.hpp"
#include "sampler.hpp"
#include "importance.hpp"
#include "brdf.hpp"
#include "material.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <optional>
#include <utility>

namespace raytracer {

struct Integrator {
    enum class Type: int {
        Whitted,
        AnalyticDirect,
        Direct,
        PathTracer
    };
    enum class NEE: int { Off, On, MIS };
    Type type = Type::Whitted;
    size_t lightSamples = 1;
    size_t samplesPerPixel = 1;
    bool jitter = false;  // expects composite spp
    bool stratify = false; // light
    NEE nextEvent = NEE::Off;
    bool russianRoulette = false;
    importance::Type importanceSampling = importance::Type::Cosine;

    Sampler sampler(Seed seed) const {
        return Sampler(seed, Stratify2D(lightSamples), stratify);
    }

    Stratify2D pixels() const {
        return Stratify2D(samplesPerPixel);
    }

    template <class F> auto dispatch(F&& f) const {
        switch (importanceSampling) {
        case importance::Type::Uniform:
            return f(importance::Uniform{});
        case importance::Type::Cosine:
            return f(importance::Cosine{});
        case importance::Type::BRDF:
            return f(importance::BRDF{});
        }
        std::unreachable();
    }

    Float pdf(const Hit& hit, Vec3 wi) const {
        return dispatch([&](auto s) { return s.pdf(hit, wi); });
    }
    std::optional<Sample> sample(const Hit& hit, Float uc, Vec2 u2) const {
        return dispatch([&](auto s) { return s.sample(hit, uc, u2); });
    }
};

}  // namespace raytracer
