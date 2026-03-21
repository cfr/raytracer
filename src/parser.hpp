#pragma once

#include "scene.hpp"
#include "parser/common.hpp"
#include "parser/materials.hpp"
#include "parser/geometry.hpp"

#include <glm/vec3.hpp>

#include <fstream>
#include <string>
#include <string_view>
#include <format>
#include <algorithm>
#include <regex>

namespace raytracer::parser {

// NOTE: possible optimization using string_view
inline std::vector<std::string> tokenize(const std::string& line) {
    const auto re = std::regex{R"(\s+)"};
    const auto vec = std::vector<std::string>(
        std::sregex_token_iterator{begin(line), end(line), re, -1},
        std::sregex_token_iterator{}
    );
    return vec;
}

// https://medium.com/@ryan_forrester_/using-switch-statements-with-strings-in-c-a-complete-guide-efa12f64a59d
inline Scene parseScene(std::istream& input) {

    std::string line;
    int lineNo = 1;

    Scene scene;
    Node node;
    Material material;

    while (std::getline(input, line)) {
        auto tokens = tokenize(line);
        if (tokens.empty()) continue;
        if (tokens[0][0] == '#') { continue; }

        try {

            if (parseScene(tokens, scene)) { continue; }
            if (parseGeometry(tokens, node, scene)) { continue; }
            if (parseMaterial(tokens, material)) {
                node.material = material;
                continue;
            }

            continue;
            //throw ParseException(std::format("Unknown token: '{}'", tokens[0]));
        } catch (const ParseException& e) {
            // Add line number
            throw ParseException(std::format("{}: {}", lineNo, e.what()));
        }
        lineNo++;
    }

    return scene;
}

inline Scene readScene(const std::string& path) {
    std::ifstream file;
    file.open(path, std::ios::in);
    if (!file) { throw ParseException("Can't open file '" + path + "'"); }
    return parseScene(file);
}

} // raytracer::parser
