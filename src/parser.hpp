#pragma once

#include "parser/args.hpp"
#include "parser/common.hpp"
#include "parser/geometry.hpp"
#include "parser/lights.hpp"
#include "parser/materials.hpp"
#include "parser/transform.hpp"
#include "scene.hpp"
#include "transforms.hpp"

#include <algorithm>
#include <glm/matrix.hpp>

#include <format>
#include <fstream>
#include <istream>
#include <regex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace raytracer::parser {

inline std::vector<std::string> tokenize(std::string const& line) {
    static auto const re = std::regex{R"(\s+)"};
    auto vec = std::vector<std::string>(std::sregex_token_iterator{begin(line), end(line), re, -1},
                                        std::sregex_token_iterator{});

    // skip empty tokens
    std::erase_if(vec, [](auto const& s) { return s.empty(); });
    // remove comments
    auto cit = std::ranges::find_if(vec, [](auto const& s) { return s[0] == '#'; });
    if (cit != vec.end()) {
        vec.erase(cit, vec.end());
    }

    return vec;
}

inline std::tuple<Scene, Camera, Settings> parseScene(std::istream& input) {
    std::string line;
    int lineNo = 0;

    Settings settings;
    Camera camera;
    Scene scene;
    std::vector<ObjectPtr> objects;

    Material material;
    TStack stack;
    Transforms xf;
    auto current = makeMaterial(material);
    std::vector<Vec3> vertices;

    while (std::getline(input, line)) {
        lineNo++;
        auto tokens = tokenize(line);
        if (tokens.empty())
            continue;

        try {
            if (parseSettings(tokens, settings)) {
                continue;
            }
            if (parseCamera(tokens, camera)) {
                continue;
            }
            if (parseGeometry(tokens, vertices, current, xf, objects)) {
                continue;
            }
            if (parseLights(tokens, xf, scene)) {
                continue;
            }
            if (parseMaterial(tokens, material)) {
                current = makeMaterial(material);
                continue;
            }
            if (parseTransform(tokens, stack)) {
                xf.m = stack.top();
                if (glm::determinant(xf.m) == 0) {
                    throw ParseException("Singular transform");
                }
                xf.inv = glm::inverse(xf.m);
                xf.invT = glm::transpose(xf.inv);
                continue;
            }
            throw ParseException(std::format("Unknown token: '{}'", tokens[0]));
        } catch (ParseException const& e) {
            // add line number
            throw ParseException(std::format("{}: {}", lineNo, e.what()));
        }
    }

    // area lights are also geometry
    for (auto const& light : scene.areaLights) {
        objects.push_back(light);
    }

    BoundingVolumeHierarchy<ObjectPtr> bvh{objects};
    scene.bvh = std::move(bvh);
    return {scene, camera, settings};
}

inline std::tuple<Scene, Camera, Settings> readScene(std::string const& path) {
    std::ifstream file;
    file.open(path);
    if (!file) {
        throw ParseException("Failed to open file '" + path + "'");
    }
    return parseScene(file);
}

}  // namespace raytracer::parser
