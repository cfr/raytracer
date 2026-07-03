#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "parser/common.hpp"

#include <vector>
#include <string>

namespace raytracer::parser {

bool parseLights(const std::vector<std::string>& tokens, Object& obj, Scene& scene) {
    auto cmd = tokens[0];
    if (cmd == "ambient") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'ambient <r> <g> <b>'");
        }
        Color rgba = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3]), 1};
        obj.ambient = rgba;
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
        Color rgba = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6]), 1};
        auto tpos = obj.transform * pos;
        scene.lights.emplace_back(tpos, rgba);
        return true;
    }
    else if (cmd == "point") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'point <x> <y> <z> <r> <g> <b>'");
        }
        Vec4 pos = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3]), 1};
        Color rgba = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6]), 1};
        auto tpos = obj.transform * pos;
        scene.lights.emplace_back(tpos, rgba);
        return true;
    }
    else if (cmd == "quadLight") {
        if (tokens.size() != 13) {
            throw ParseException("Expected 'quadLight <x> <y> <z> <elx> <ely> <elz> <erx> <ery> <erz> <r> <g> <b>'");
        }
        Vec3 pos = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3])};
        Vec3 el = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6])};
        Vec3 er = {parseNum<Float>(tokens[7]), parseNum<Float>(tokens[8]), parseNum<Float>(tokens[9])};
        Color rgba = {parseNum<Float>(tokens[10]), parseNum<Float>(tokens[11]), parseNum<Float>(tokens[12]), 1};
        auto tpos = transformVec3(obj.transform, pos);
        scene.areaLights.emplace_back(tpos, el, er, rgba);
        return true;
    }
    return false;
}

}  //  namespace raytracer::parser
