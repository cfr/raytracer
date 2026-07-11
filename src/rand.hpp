#pragma once

#include "values.hpp"

#include <random>

namespace raytracer {

class Gen {
    // TODO: switch to pcg
    std::mt19937_64 engine;
    std::uniform_real_distribution<Float> dist{0.0, 1.0};

  public:
    explicit Gen(std::random_device& r) : engine(r()) {}

    Float operator()() {
        return dist(engine);
    }
};

}  // namespace raytracer
