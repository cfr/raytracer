#pragma once

#include "parser/args.hpp"
#include "parser/common.hpp"
#include "parser/geometry.hpp"
#include "parser/lights.hpp"
#include "parser/materials.hpp"
#include "parser/transform.hpp"
#include "scene.hpp"
#include "transforms.hpp"

#include <glm/matrix.hpp>

#include <format>
#include <fstream>
#include <istream>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace aktis::parser {

inline void tokenize(std::string_view line, Tokens& tokens) {
    constexpr std::string_view space = " \t\n\v\f\r";

    tokens.clear();
    for (size_t pos = 0;;) {
        pos = line.find_first_not_of(space, pos);
        if (pos == std::string_view::npos || line[pos] == '#') {
            return;
        }
        auto const stop = line.find_first_of(space, pos);
        tokens.push_back(line.substr(pos, stop - pos));
        if (stop == std::string_view::npos) {
            return;
        }
        pos = stop;
    }
}

inline std::tuple<Scene, Camera, Settings> parseScene(std::istream& input) {
    std::string line;
    Tokens tokens;
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
        tokenize(line, tokens);
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

}  // namespace aktis::parser
