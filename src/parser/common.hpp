#pragma once

#include "scene.hpp"

#include <stdexcept>
#include <charconv>
#include <format>
#include <string>
#include <string_view>

namespace raytracer::parser {

class ParseException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

template <typename T>
T parseNum(std::string_view sv) {
    T value;
    if (!sv.empty() && sv[0] == '+') { sv = sv.substr(1); } // skip leading +
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

    if (ec != std::errc{}) {
        throw ParseException(std::format("Invalid number format: '{}'", sv));
    }
    if (ptr != sv.data() + sv.size()) {
        throw ParseException(std::format("Trailing characters in number: '{}'", sv));
    }
    return value;
}

inline bool parseScene(const std::vector<std::string>& tokens, Scene& scene) {
    auto cmd = tokens[0];
    if (cmd == "size") {
        if (tokens.size() != 3) {
            throw ParseException("Expected 'size <width> <height>'");
        }
        auto width = parseNum<int>(tokens[1]);
        auto height = parseNum<int>(tokens[2]);
        scene.image = Image(width, height);
        return true;
    }
    else if (cmd == "maxdepth") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'maxdepth <depth>'");
        }
        scene.depth = parseNum<int>(tokens[1]);
        return true;
    }
    else if (cmd == "output") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'output <filename>'");
        }
        scene.output = tokens[1] + ".ppm";
        return true;
    }
    else if (cmd == "camera") {
        if (tokens.size() != 11) {
            throw ParseException("Expected 'camera <eyex> <eyey> <eyez> <cx> <cy> <cz> <upx> <upy> <upz> <fovy>'");
        }
        Camera c;
        c.eye = { parseNum<float>(tokens[1]), parseNum<float>(tokens[2]), parseNum<float>(tokens[3]) };
        c.center = { parseNum<float>(tokens[4]), parseNum<float>(tokens[5]), parseNum<float>(tokens[6]) };
        c.up = { parseNum<float>(tokens[7]), parseNum<float>(tokens[8]), parseNum<float>(tokens[9]) };
        c.fovy = parseNum<float>(tokens[10]);
        scene.camera = c;
        return true;
    }
    return false;
}

}
