#pragma once

#include "values.hpp"
#include "scene.hpp"

#include <algorithm>
#include <stdexcept>
#include <charconv>
#include <format>
#include <string>
#include <string_view>
#include <vector>

namespace raytracer::parser {

class ParseException : public std::runtime_error {
 public:
    using std::runtime_error::runtime_error;
};

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
        settings.integrator.lightSamples = std::max(1uz, parseNum<size_t>(tokens[1]));
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
    else if (cmd == "nexteventestimation") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'nexteventestimation <on/off>'");
        }
        auto onoff = tokens[1];
        if (onoff != "on" && onoff != "off") {
            throw ParseException("Expected 'nexteventestimation <on/off>'");
        }
        settings.integrator.nextEvent = onoff == "on";
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
    else if (cmd == "spp") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'spp <count>'");
        }
        settings.integrator.samplesPerPixel = std::max(1uz, parseNum<size_t>(tokens[1]));
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
