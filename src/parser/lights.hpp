#pragma once

#include "scene.hpp"
#include "parser/common.hpp"

namespace raytracer::parser {

inline bool parseLights(const std::vector<std::string>& tokens, Node& node, Scene& scene) {
    auto cmd = tokens[0];
    if (cmd == "ambient") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'ambient <r> <g> <b>'");
        }
        vec3 rgb = { parseNum<float>(tokens[1]), parseNum<float>(tokens[2]), parseNum<float>(tokens[3]) };
        node.ambient = rgb;
        return true;
    }
    else if (cmd == "attenuation") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'attenuation <c> <l> <q>'");
        }
        scene.attenuation.constant = parseNum<float>(tokens[1]);
        scene.attenuation.linear = parseNum<float>(tokens[2]);
        scene.attenuation.quadratic = parseNum<float>(tokens[3]);
        return true;
    }
    else if (cmd == "directional") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'directional <x> <y> <z> <r> <g> <b>'");
        }
        vec4 pos = { parseNum<float>(tokens[1]), parseNum<float>(tokens[2]), parseNum<float>(tokens[3]), 0};
        vec3 rgb = { parseNum<float>(tokens[4]), parseNum<float>(tokens[5]), parseNum<float>(tokens[6]) };
        scene.lights.emplace_back(transformVec(node.transform, pos), rgb);
        return true;
    }
    else if (cmd == "point") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'point <x> <y> <z> <r> <g> <b>'");
        }
        vec4 pos = { parseNum<float>(tokens[1]), parseNum<float>(tokens[2]), parseNum<float>(tokens[3]), 1};
        vec3 rgb = { parseNum<float>(tokens[4]), parseNum<float>(tokens[5]), parseNum<float>(tokens[6]) };
        scene.lights.emplace_back(transformVec(node.transform, pos), rgb);
        return true;
    }
    return false;
}

}

