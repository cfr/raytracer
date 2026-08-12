#pragma once

#include "values.hpp"
#include "scene.hpp"

#include <algorithm>
#include <stdexcept>
#include <charconv>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace raytracer::parser {

class ParseException : public std::runtime_error {
 public:
    using std::runtime_error::runtime_error;
};

inline MaterialPtr makeMaterial(Material m) {
    m.precomputeT();
    return std::make_shared<Material>(std::move(m));
}

inline TransformsPtr sharedTransforms(const Transforms& xf) {
    return xf.m == identity ? nullptr : std::make_shared<Transforms>(xf);
}

template <typename T>
T parseNum(std::string_view sv) {
    T value;
    if (!sv.empty() && sv[0] == '+') { sv = sv.substr(1); }  // skip leading +
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

    if (ec != std::errc{}) {
        throw ParseException(std::format("Invalid number format: '{}'", sv));
    }
    if (ptr != sv.data() + sv.size()) {
        throw ParseException(std::format("Trailing characters in number: '{}'", sv));
    }
    return value;
}

bool parseSettings(const std::vector<std::string>& tokens, Settings& settings) {
    auto cmd = tokens[0];
    if (cmd == "size") {
        if (tokens.size() != 3) {
            throw ParseException("Expected 'size <width> <height>'");
        }
        auto width = parseNum<size_t>(tokens[1]);
        auto height = parseNum<size_t>(tokens[2]);
        settings.size = Size{width, height};
        return true;
    }
    else if (cmd == "maxdepth") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'maxdepth <depth>'");
        }
        settings.depth = parseNum<int>(tokens[1]);
        return true;
    }
    else if (cmd == "threads") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'threads <count>'");
        }
        settings.threads = parseNum<size_t>(tokens[1]);
        return true;
    }
    else if (cmd == "gamma") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'gamma <value>'");
        }
        settings.gamma = glm::max(Float(0.1), parseNum<Float>(tokens[1]));
        return true;
    }
    else if (cmd == "output") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'output <filename>'");
        }
        settings.output = tokens[1] + ".ppm";
        return true;
    }
    else if (cmd == "integrator") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'integrator <whitted/raytracer/direct/analyticdirect/pathtracer>'");
        }
        auto integrator = tokens[1];
        if (integrator == "whitted" || integrator == "raytracer") {
            settings.integrator.type = Integrator::Type::Whitted;
        } else if (integrator == "direct") {
            settings.integrator.type = Integrator::Type::Direct;
        } else if (integrator == "analyticdirect") {
            settings.integrator.type = Integrator::Type::AnalyticDirect;
        } else if (integrator == "pathtracer") {
            settings.integrator.type = Integrator::Type::PathTracer;
        } else {
            throw ParseException("Expected 'integrator <whitted/raytracer/direct/analyticdirect/pathtracer>'");
        }
        return true;
    }
    else if (cmd == "lightsamples") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'lightsamples <count>'");
        }
        settings.integrator.lightSamples = glm::max(1uz, parseNum<size_t>(tokens[1]));
        return true;
    }
    else if (cmd == "lightstratify") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'lightstratify <on/off>'");
        }
        auto onoff = tokens[1];
        if (onoff != "on" && onoff != "off") {
            throw ParseException("Expected 'lightstratify <on/off>'");
        }
        settings.integrator.stratify = onoff == "on";
        return true;
    }
    else if (cmd == "jitter") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'jitter <on/off>'");
        }
        auto onoff = tokens[1];
        if (onoff != "on" && onoff != "off") {
            throw ParseException("Expected 'jitter <on/off>'");
        }
        settings.integrator.jitter = onoff == "on";
        return true;
    }
    else if (cmd == "nexteventestimation") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'nexteventestimation <on/off/mis>'");
        }
        auto nee = tokens[1];
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
    else if (cmd == "russianroulette") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'russianroulette <on/off>'");
        }
        auto onoff = tokens[1];
        if (onoff != "on" && onoff != "off") {
            throw ParseException("Expected 'russianroulette <on/off>'");
        }
        settings.integrator.russianRoulette = onoff == "on";
        return true;
    }
    else if (cmd == "seed") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'seed <value>'");
        }
        settings.seed = parseNum<Seed>(tokens[1]);
        return true;
    }
    else if (cmd == "spp") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'spp <count>'");
        }
        settings.integrator.samplesPerPixel = glm::max(1uz, parseNum<size_t>(tokens[1]));
        return true;
    }
    else if (cmd == "importancesampling") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'importancesampling <hemisphere/cosine/brdf>'");
        }
        auto importance = tokens[1];
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

bool parseCamera(const std::vector<std::string>& tokens, Camera& camera) {
    auto cmd = tokens[0];
    if (cmd == "camera") {
        if (tokens.size() != 11) {
            throw ParseException("Expected 'camera <eyex> <eyey> <eyez> <cx> <cy> <cz> <upx> <upy> <upz> <fovy>'");
        }
        Camera c;
        c.eye = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3])};
        c.center = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6])};
        c.up = {parseNum<Float>(tokens[7]), parseNum<Float>(tokens[8]), parseNum<Float>(tokens[9])};
        c.fovy = parseNum<Float>(tokens[10]);
        camera = c;
        return true;
    }
    return false;
}

}  // namespace raytracer::parser
