#pragma once

#include "values.hpp"
#include "rand.hpp"

namespace raytracer {

class Sampler {
    size_t samples_;
    bool stratify_;
    Gen gen_;

    size_t xsamples_ = 1;
    size_t ysamples_ = 1;

 public:
    Sampler(size_t samples, bool stratify, Seed seed) : samples_(samples), stratify_(stratify), gen_(seed) {
        if (stratify) {
            auto xsamples = size_t(std::sqrt(Float(samples)));
            while (xsamples > 1 && samples % xsamples != 0) { xsamples--; }
            ysamples_ = samples / xsamples;
            xsamples_ = xsamples;
        }
    }

    Vec2 gen(size_t sample) {
        if (stratify_) {
            size_t sx = sample % xsamples_;
            size_t sy = sample / xsamples_;
            return {(sx + gen_()) / xsamples_, (sy + gen_()) / ysamples_};
        } else {
            return {gen_(), gen_()};
        }
    }

    size_t samples() const {
        return samples_;
    }
};

struct Integrator {
    enum class Type: int {
        Whitted,
        AnalyticDirect,
        Direct
    };
    Type type = Type::Whitted;
    size_t samples = 1;
    bool stratify = false;

    Sampler sampler(Seed seed) {
        return Sampler(samples, stratify, seed);
    }
};

}  // namespace raytracer
