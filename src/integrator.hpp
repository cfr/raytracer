#pragma once

#include "brdf.hpp"
#include "importance.hpp"
#include "material.hpp"
#include "sampler.hpp"
#include "values.hpp"

#include <cstdint>
#include <optional>
#include <utility>

namespace raytracer {

struct Integrator {
    enum class Type : std::uint8_t { Whitted, AnalyticDirect, Direct, PathTracer };
    enum class NEE : std::uint8_t { Off, On, MIS };
    Type type = Type::Whitted;
    size_t lightSamples = 1;
    size_t samplesPerPixel = 1;
    bool jitter = false;    // expects composite spp
    bool stratify = false;  // light
    NEE nextEvent = NEE::Off;
    bool russianRoulette = false;
    importance::Type importanceSampling = importance::Type::Cosine;
    int depth = 5;  // clamped to 0..maxBounces in parser

    [[nodiscard]] Sampler sampler(Seed seed) const {
        return Sampler(seed, Stratify2D(lightSamples), stratify);
    }

    [[nodiscard]] Stratify2D pixels() const {
        return Stratify2D(samplesPerPixel);
    }

    template <class F> [[nodiscard]] auto dispatch(F const& f) const {
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

    [[nodiscard]] Float pdf(Hit const& hit, Vec3 wi) const {
        return dispatch([&](auto s) { return s.pdf(hit, wi); });
    }
    [[nodiscard]] std::optional<Sample> sample(Hit const& hit, Float uc, Vec2 u2) const {
        return dispatch([&](auto s) { return s.sample(hit, uc, u2); });
    }
};

}  // namespace raytracer
