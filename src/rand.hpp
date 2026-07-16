#pragma once

#include "values.hpp"

#include <random>

namespace raytracer {

class Gen {
    // TODO: switch to pcg
    std::mt19937_64 engine;
    std::uniform_real_distribution<Float> dist{0.0, 1.0};

 public:
    explicit Gen(Seed seed) : engine(seed) {}

    Float operator()() {
        return dist(engine);
    }
};

}  // namespace raytracer
