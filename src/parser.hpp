#pragma once

#include "scene.hpp"
#include "tstack.hpp"
#include "parser/common.hpp"
#include "parser/materials.hpp"
#include "parser/geometry.hpp"
#include "parser/transform.hpp"
#include "parser/lights.hpp"

#include <fstream>
#include <string>
#include <string_view>
#include <format>
#include <algorithm>
#include <regex>
#include <tuple>

namespace raytracer::parser {

// TODO: take string_view
std::vector<std::string> tokenize(const std::string& line) {
    const auto re = std::regex{R"(\s+)"};
    auto vec = std::vector<std::string>(
        std::sregex_token_iterator{begin(line), end(line), re, -1},
        std::sregex_token_iterator{}
    );

    // skip empty tokens
    std::erase_if(vec, [](const auto& s) { return s.empty(); });
    // remove comments
    auto cit = std::find_if(vec.begin(), vec.end(), [](const auto& s) { return s[0] == '#'; });
    if (cit != vec.end()) { vec.erase(cit, vec.end()); }

    return vec;
}

std::tuple<Scene, Camera, Settings> parseScene(std::istream& input) {

    std::string line;
    int lineNo = 0;

    Settings settings;
    Camera camera;
    Scene scene;
    Material material;
    TStack stack;
    Node node;
    std::vector<Vec3> vertices;

    while (std::getline(input, line)) {
        lineNo++;
        auto tokens = tokenize(line);
        if (tokens.empty()) continue;

        try {
            if (parseSettings(tokens, settings)) { continue; }
            if (parseCamera(tokens, camera)) { continue; }
            if (parseGeometry(tokens, vertices, node, scene)) { continue; }
            if (parseLights(tokens, node, scene)) { continue; }
            if (parseMaterial(tokens, material)) {
                node.material = material;
                continue;
            }
            if (parseTransform(tokens, stack)) {
                node.transform = stack.top();
                node.inverse = glm::inverse(node.transform);
                node.inverseTranspose = glm::transpose(node.inverse);
                continue;
            }
            throw ParseException(std::format("Unknown token: '{}'", tokens[0]));
        } catch (const ParseException& e) {
            // Add line number
            throw ParseException(std::format("{}: {}", lineNo, e.what()));
        }
    }

    return {scene, camera, settings};
}

std::tuple<Scene, Camera, Settings> readScene(const std::string& path) {
    std::ifstream file;
    file.open(path);
    if (!file) { throw ParseException("Can't open file '" + path + "'"); }
    return parseScene(file);
}

} // raytracer::parser
