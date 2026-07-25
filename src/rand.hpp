#pragma once

#include "values.hpp"

#include <cstdint>
#include <limits>
#include <random>

namespace raytracer {

class Gen {
    // TODO: switch to pcg
    using Engine = std::mt19937_64;

    static constexpr int digits = std::numeric_limits<Float>::digits;
    static constexpr int shift = 64 - digits;
    static constexpr Float scale = Float(1) / Float(std::uint64_t(1) << digits);

    Engine engine_;

 public:
    explicit Gen(Seed seed) : engine_(seed) {}

    Float operator()() {
        return Float(engine_() >> shift) * scale;
    }
};

}  // namespace raytracer
