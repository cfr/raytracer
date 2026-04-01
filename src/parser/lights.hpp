#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "parser/common.hpp"

#include <vector>
#include <string>

namespace raytracer::parser {

bool parseLights(const std::vector<std::string>& tokens, Node& node, Scene& scene) {
    auto cmd = tokens[0];
    if (cmd == "ambient") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'ambient <r> <g> <b>'");
        }
        ColorA rgba = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3]), 1};
        node.ambient = rgba;
        return true;
    }
    else if (cmd == "attenuation") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'attenuation <c> <l> <q>'");
        }
        scene.attenuation.constant = parseNum<Float>(tokens[1]);
        scene.attenuation.linear = parseNum<Float>(tokens[2]);
        scene.attenuation.quadratic = parseNum<Float>(tokens[3]);
        return true;
    }
    else if (cmd == "directional") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'directional <x> <y> <z> <r> <g> <b>'");
        }
        Vec4 pos = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3]), 0};
        ColorA rgba = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6]), 1};
        auto tpos = transformVec(node.transform, pos);
        scene.lights.emplace_back(tpos, rgba);
        return true;
    }
    else if (cmd == "point") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'point <x> <y> <z> <r> <g> <b>'");
        }
        Vec4 pos = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3]), 1};
        ColorA rgba = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6]), 1};
        auto tpos = transformVec(node.transform, pos);
        scene.lights.emplace_back(tpos, rgba);
        return true;
    }
    return false;
}

}  //  namespace raytracer::parser
