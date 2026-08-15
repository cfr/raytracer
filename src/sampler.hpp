#pragma once

#include "values.hpp"
#include "rand.hpp"

#include <glm/common.hpp>
#include <glm/exponential.hpp>

namespace raytracer {

class Stratify2D {
 private:
    size_t samples_ = 1;
    size_t xsamples_ = 1;
    size_t ysamples_ = 1;

 public:
    explicit Stratify2D(size_t samples) : samples_(glm::max(1uz, samples)) {
        auto xsamples = size_t(glm::sqrt(Float(samples_)));
        while (xsamples > 1 && samples_ % xsamples != 0) { xsamples--; }
        ysamples_ = samples_ / xsamples;
        xsamples_ = xsamples;
    }

    Vec2 unit2(Vec2 u, size_t index) const {
        size_t sx = index % xsamples_;
        size_t sy = index / xsamples_;
        return {(sx + u.x) / xsamples_, (sy + u.y) / ysamples_};
    }

    size_t samples() const {
        return samples_;
    }
};

class Sampler {
    Gen gen_;
    Stratify2D stratify2d_;
    bool stratify_;

 public:
    explicit Sampler(Seed seed, Stratify2D stratify2d, bool stratify)
        : gen_(seed), stratify2d_(stratify2d), stratify_(stratify) {}

    Float unit() {
        return gen_();
    }

    Vec2 unit2() {
        return {gen_(), gen_()};
    }

    Vec3 unit3() {
        return {gen_(), gen_(), gen_()};
    }

    Vec2 unit2stratified(size_t index) {
        return stratify2d_.unit2(unit2(), index);
    }

    Vec2 unit2(size_t index) {
        return stratify_ ? unit2stratified(index) : unit2();
    }

    size_t samples() {
        return stratify2d_.samples();
    }
};

}  // namespace raytracer

