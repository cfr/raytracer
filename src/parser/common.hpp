#pragma once

#include "camera.hpp"
#include "scene.hpp"
#include "values.hpp"

#include <glm/common.hpp>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace raytracer::parser {

class ParseException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

inline MaterialPtr makeMaterial(Material m) {
    m.precomputeT();
    return std::make_shared<Material>(m);
}

inline TransformsPtr sharedTransforms(Transforms const& xf) {
    return xf.m == identity ? nullptr : std::make_shared<Transforms>(xf);
}

inline constexpr size_t maxPixels = size_t(1) << 26;  // 8k x 8k

template <typename T> T parseNum(std::string_view sv) {
    T value;
    if (!sv.empty() && sv[0] == '+') {
        sv = sv.substr(1);
    }  // skip leading +
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

    if (ec == std::errc::result_out_of_range) {
        throw ParseException(std::format("Number out of range: '{}'", sv));
    }
    if (ec != std::errc{}) {
        throw ParseException(std::format("Invalid number format: '{}'", sv));
    }
    if (ptr != sv.data() + sv.size()) {
        throw ParseException(std::format("Trailing characters in number: '{}'", sv));
    }
    if constexpr (std::is_floating_point_v<T>) {
        if (!std::isfinite(value)) {
            throw ParseException(std::format("Non-finite number: '{}'", sv));
        }
    }
    return value;
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
inline bool parseSettings(std::vector<std::string> const& tokens, Settings& settings) {
    const auto& cmd = tokens[0];
    if (cmd == "size") {
        if (tokens.size() != 3) {
            throw ParseException("Expected 'size <width> <height>'");
        }
        auto width = parseNum<size_t>(tokens[1]);
        auto height = parseNum<size_t>(tokens[2]);
        if (width == 0 || height == 0) {
            throw ParseException("Expected 'size <width> <height>', width > 0, height > 0");
        }
        if (width > maxPixels / height) {
            throw ParseException(
                std::format("Expected 'size <width> <height>', at most {} pixels", maxPixels));
        }
        settings.size = Size{.width = width, .height = height};
        return true;
    }
    if (cmd == "maxdepth") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'maxdepth <depth>'");
        }
        constexpr int maxBounces = 512;
        int depth = parseNum<int>(tokens[1]);
        if (depth < 0) {
            depth = maxBounces;
        }
        settings.integrator.depth = glm::min(depth, maxBounces);
        return true;
    }
    if (cmd == "threads") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'threads <count>'");
        }
        settings.threads = parseNum<size_t>(tokens[1]);
        return true;
    }
    if (cmd == "gamma") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'gamma <value>'");
        }
        settings.gamma = glm::max(Float(0.1), parseNum<Float>(tokens[1]));
        return true;
    }
    if (cmd == "output") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'output <filename>'");
        }
        settings.output = tokens[1];
        return true;
    }
    if (cmd == "integrator") {
        if (tokens.size() != 2) {
            throw ParseException(
                "Expected 'integrator <whitted/raytracer/direct/analyticdirect/pathtracer>'");
        }
        const auto& integrator = tokens[1];
        if (integrator == "whitted" || integrator == "raytracer") {
            settings.integrator.type = Integrator::Type::Whitted;
        } else if (integrator == "direct") {
            settings.integrator.type = Integrator::Type::Direct;
        } else if (integrator == "analyticdirect") {
            settings.integrator.type = Integrator::Type::AnalyticDirect;
        } else if (integrator == "pathtracer") {
            settings.integrator.type = Integrator::Type::PathTracer;
        } else {
            throw ParseException(
                "Expected 'integrator <whitted/raytracer/direct/analyticdirect/pathtracer>'");
        }
        return true;
    }
    if (cmd == "lightsamples") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'lightsamples <count>'");
        }
        settings.integrator.lightSamples = glm::max(1uz, parseNum<size_t>(tokens[1]));
        return true;
    }
    if (cmd == "lightstratify") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'lightstratify <on/off>'");
        }
        const auto& onoff = tokens[1];
        if (onoff != "on" && onoff != "off") {
            throw ParseException("Expected 'lightstratify <on/off>'");
        }
        settings.integrator.stratify = onoff == "on";
        return true;
    }
    if (cmd == "jitter") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'jitter <on/off>'");
        }
        const auto& onoff = tokens[1];
        if (onoff != "on" && onoff != "off") {
            throw ParseException("Expected 'jitter <on/off>'");
        }
        settings.integrator.jitter = onoff == "on";
        return true;
    }
    if (cmd == "nexteventestimation") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'nexteventestimation <on/off/mis>'");
        }
        const auto& nee = tokens[1];
        if (nee == "off") {
            settings.integrator.nextEvent = Integrator::NEE::Off;
        } else if (nee == "on") {
            settings.integrator.nextEvent = Integrator::NEE::On;
        } else if (nee == "mis") {
            settings.integrator.nextEvent = Integrator::NEE::MIS;
        } else {
            throw ParseException("Expected 'nexteventestimation <on/off/mis>'");
        }
        return true;
    }
    if (cmd == "russianroulette") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'russianroulette <on/off>'");
        }
        const auto& onoff = tokens[1];
        if (onoff != "on" && onoff != "off") {
            throw ParseException("Expected 'russianroulette <on/off>'");
        }
        settings.integrator.russianRoulette = onoff == "on";
        return true;
    }
    if (cmd == "seed") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'seed <value>'");
        }
        settings.seed = parseNum<Seed>(tokens[1]);
        return true;
    }
    if (cmd == "spp") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'spp <count>'");
        }
        auto spp = parseNum<size_t>(tokens[1]);
        if (spp == 0) {
            throw ParseException("Expected 'spp <count>', spp > 0");
        }
        settings.integrator.samplesPerPixel = spp;
        return true;
    }
    if (cmd == "importancesampling") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'importancesampling <hemisphere/cosine/brdf>'");
        }
        const auto& importance = tokens[1];
        if (importance == "hemisphere") {
            settings.integrator.importanceSampling = importance::Type::Uniform;
        } else if (importance == "cosine") {
            settings.integrator.importanceSampling = importance::Type::Cosine;
        } else if (importance == "brdf") {
            settings.integrator.importanceSampling = importance::Type::BRDF;
        } else {
            throw ParseException("Expected 'importancesampling <hemisphere/cosine/brdf>'");
        }
        return true;
    }
    return false;
}
// NOLINTEND(readability-function-cognitive-complexity)

inline bool parseCamera(std::vector<std::string> const& tokens, Camera& camera) {
    const auto& cmd = tokens[0];
    if (cmd == "camera") {
        if (tokens.size() != 11) {
            throw ParseException(
                "Expected 'camera <eyex> <eyey> <eyez> <cx> <cy> <cz> <upx> <upy> <upz> <fovy>'");
        }
        Camera c;
        c.eye = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]),
                 parseNum<Float>(tokens[3])};
        c.center = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]),
                    parseNum<Float>(tokens[6])};
        c.up = {parseNum<Float>(tokens[7]), parseNum<Float>(tokens[8]), parseNum<Float>(tokens[9])};
        c.fovy = parseNum<Float>(tokens[10]);
        if (Basis::degenerate(c)) {
            throw ParseException("Degenerate camera: up parallel to view direction");
        }
        if (c.fovy <= 0 || c.fovy >= 180) {
            throw ParseException("Expected 'camera ... <fovy>', 0 < fovy < 180");
        }
        camera = c;
        return true;
    }
    return false;
}

}  // namespace raytracer::parser
