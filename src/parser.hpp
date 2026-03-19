#pragma once

#include "scene.hpp"
#include "parser/common.hpp"
#include "parser/materials.hpp"

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
    Scene scene;
    std::string line;
    int lineNo = 0;

    Material current;

    while (std::getline(input, line)) {
        lineNo++;
        auto tokens = tokenize(line);
        if (tokens.empty()) continue;

        std::string_view cmd = tokens[0];

        try {
            // TODO: separate parsers
            if (cmd[0] == '#') { continue; }
            if (cmd == "size") {
                if (tokens.size() != 3) {
                    throw ParseException("Expected 'size <w> <h>'");
                }
                auto width = parseNum<int>(tokens[1]);
                auto height = parseNum<int>(tokens[2]);
                scene.image = Image(width, height);
            }
            else if (cmd == "camera") {
                // camera x y z ux uy uz vx vy vz fov (10 numbers)
                if (tokens.size() != 11) {
                    throw ParseException("Expected 'camera <10 floats>'");
                }

                Camera cam;
                cam.position = { parseNum<float>(tokens[1]), parseNum<float>(tokens[2]), parseNum<float>(tokens[3]) };
                cam.right    = { parseNum<float>(tokens[4]), parseNum<float>(tokens[5]), parseNum<float>(tokens[6]) };
                cam.up       = { parseNum<float>(tokens[7]), parseNum<float>(tokens[8]), parseNum<float>(tokens[9]) };
                cam.fov      = parseNum<float>(tokens[10]);
                scene.camera = cam;
            }
            else if (cmd == "maxverts") {
                if (tokens.size() != 2)
                    throw ParseException("Expected 'maxverts <count>'");
                int count = parseNum<int>(tokens[1]);
                if (count > 0) scene.vertices.reserve(count);
            }
            else if (cmd == "vertex") {
                if (tokens.size() != 4) {
                    throw ParseException("Expected 'vertex <x> <y> <z>'");
                }
                vec3 v;
                v.x = parseNum<float>(tokens[1]);
                v.y = parseNum<float>(tokens[2]);
                v.z = parseNum<float>(tokens[3]);
                scene.vertices.push_back(v);
            }
            else if (cmd == "tri") {
                if (tokens.size() != 4) {
                    throw ParseException("Expected 'tri <i1> <i2> <i3>'");
                }
                Triangle t;
                t.indices[0] = parseNum<int>(tokens[1]);
                t.indices[1] = parseNum<int>(tokens[2]);
                t.indices[2] = parseNum<int>(tokens[3]);
                scene.triangles.push_back(t);
            }
            else if (isMaterial(tokens)) {
                parseUpdateMaterial(tokens, current);
            }
            else {
                continue;
                //throw ParseException(std::format("Unknown command: '{}'", cmd));
            }
        } catch (const ParseException& e) {
            // Add line number
            throw ParseException(std::format("{}: {}", lineNo, e.what()));
        }
    }

    return scene;
}

inline Scene readScene(const std::string& path) {
    std::ifstream file;
    file.open(path, std::ios::in);
    return parseScene(file);
}

} // raytracer::parser
