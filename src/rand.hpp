#pragma once

#include "values.hpp"

#include <bit>
#include <cstdint>
#include <limits>
#include <type_traits>

#ifdef USE_PCG
#include "pcg_random.hpp"
#else
#include <random>
#endif

namespace aktis {

template <typename device> Seed seed64(device& rd) {
    std::uint64_t const hi = rd();
    std::uint64_t const lo = rd();
    return (hi << 32) | lo;
}

constexpr Seed splitmix(Seed x) {
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
}

class Gen {
    static constexpr int digits = std::numeric_limits<Float>::digits;

#ifdef USE_PCG
    using Engine = std::conditional_t<(digits <= 32), pcg32, pcg64>;
#else
    using Engine = std::conditional_t<(digits <= 32), std::mt19937, std::mt19937_64>;
#endif
    using Result = Engine::result_type;

    static constexpr int bits = std::bit_width(Engine::max());
    static constexpr int shift = bits - digits;
    static constexpr Float scale = Float(1) / Float(Result(1) << digits);

    static_assert(digits < bits, "engine output too narrow for Float mantissa");

    Engine engine_;

  public:
    explicit Gen(Seed seed) : engine_(seed) {}

    Float operator()() {
        return Float(engine_() >> shift) * scale;
    }
};

}  // namespace aktis
