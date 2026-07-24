#pragma once

#include "values.hpp"
#include "rand.hpp"
#include "camera.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace raytracer {

class Stratify2D {
 private:
    size_t samples_ = 1;
    size_t xsamples_ = 1;
    size_t ysamples_ = 1;

 public:
    explicit Stratify2D(size_t samples) : samples_(samples) {
        auto xsamples = size_t(std::sqrt(Float(samples)));
        while (xsamples > 1 && samples % xsamples != 0) { xsamples--; }
        ysamples_ = samples / xsamples;
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
    Stratify2D stratify_;

 public:
    explicit Sampler(Seed seed, Stratify2D stratify) : gen_(seed), stratify_(stratify) {}

    Float unit() {
        return gen_();
    }

    Vec2 unit2() {
        return {gen_(), gen_()};
    }

    Vec2 unit2stratified(size_t index) {
        return stratify_.unit2(unit2(), index);
    }

    size_t samples() {
        return stratify_.samples();
    }

    Vec3 hemisphere(Vec3 normal) { // cosine
        Vec2 u2 = {gen_(), gen_()};
        Float phi = 2.0 * pi * u2.x;
        Float cosT = glm::sqrt(u2.y);
        Float sinT = glm::sqrt(glm::max(0.0, 1.0 - u2.y));
        Vec3 s = {glm::cos(phi)*sinT, glm::sin(phi)*sinT, cosT};
        auto b = Basis(normal);
        return b.toWorld(s);
    }
};

}  // namespace raytracer

