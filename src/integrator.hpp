#pragma once

#include "values.hpp"
#include "rand.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

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

    size_t samples() const {
        return samples_;
    }

    Vec2 unit2(size_t index) {
        if (stratify_) {
            size_t sx = index % xsamples_;
            size_t sy = index / xsamples_;
            return {(sx + gen_()) / xsamples_, (sy + gen_()) / ysamples_};
        } else {
            return {gen_(), gen_()};
        }
    }

    // TODO: Duff 2017, branchless basis
    Vec3 hemisphere(Vec3 normal, size_t /*index*/) {
        Vec2 u2 = {gen_(), gen_()}; //unit2(index);
        Float phi = 2.0 * pi * u2.x;
        Float cosT = glm::sqrt(u2.y);
        Float sinT = glm::sqrt(glm::max(0.0, 1.0 - u2.y));
        Vec3 s = {glm::cos(phi)*sinT, glm::sin(phi)*sinT, cosT};
        Vec3 w = normal;
        Vec3 a = (glm::abs(normal.x) > 0.9) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
        Vec3 u = glm::normalize(glm::cross(a, w));
        Vec3 v = glm::cross(w, u);
        return s.x * u + s.y * v + s.z * w;
    }
};

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
    bool stratify = false;

    Sampler sampler(Seed seed) {
        return Sampler(lightSamples, stratify, seed);
    }
};

}  // namespace raytracer
