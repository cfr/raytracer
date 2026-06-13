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
#include <vector>

namespace raytracer::parser {

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
    std::vector<ManagedObject> objects;

    Material material;
    TStack stack;
    Object obj;
    std::vector<Vec3> vertices;

    while (std::getline(input, line)) {
        lineNo++;
        auto tokens = tokenize(line);
        if (tokens.empty()) continue;

        try {
            if (parseSettings(tokens, settings)) { continue; }
            if (parseCamera(tokens, camera)) { continue; }
            if (parseGeometry(tokens, vertices, obj, objects)) { continue; }
            if (parseLights(tokens, obj, scene)) { continue; }
            if (parseMaterial(tokens, material)) {
                obj.material = material;
                continue;
            }
            if (parseTransform(tokens, stack)) {
                obj.transform = stack.top();
                obj.inverse = glm::inverse(obj.transform);
                obj.inverseTranspose = glm::transpose(obj.inverse);
                continue;
            }
            throw ParseException(std::format("Unknown token: '{}'", tokens[0]));
        } catch (const ParseException& e) {
            // Add line number
            throw ParseException(std::format("{}: {}", lineNo, e.what()));
        }
    }

    BoundingVolumeHierarchy<ManagedObject> bvh{objects};
    scene.bvh = std::move(bvh);
    return {scene, camera, settings};
}

std::tuple<Scene, Camera, Settings> readScene(const std::string& path) {
    std::ifstream file;
    file.open(path);
    if (!file) { throw ParseException("Can't open file '" + path + "'"); }
    return parseScene(file);
}

}  // namespace raytracer::parser
