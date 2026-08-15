#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "parser/common.hpp"

#include <memory>
#include <vector>
#include <string>

namespace raytracer::parser {

inline bool parseLights(const std::vector<std::string>& tokens, const Transforms& xf, Scene& scene) {
    auto cmd = tokens[0];
    if (cmd == "attenuation") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'attenuation <c> <l> <q>'");
        }
        auto c = parseNum<Float>(tokens[1]);
        auto l = parseNum<Float>(tokens[2]);
        auto q = parseNum<Float>(tokens[3]);
        if (c < 0 || l < 0 || q < 0) {
            throw ParseException("Expected 'attenuation <c> <l> <q>', coefficients >= 0");
        }
        if (c == 0 && l == 0 && q == 0) {
            throw ParseException("Expected 'attenuation <c> <l> <q>', at least one coefficient > 0");
        }
        scene.attenuation = {c, l, q};
        return true;
    }
    else if (cmd == "directional") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'directional <x> <y> <z> <r> <g> <b>'");
        }
        Vec4 pos = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3]), 0};
        Color rgb = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6])};
        auto tpos = xf.m * pos;
        scene.lights.emplace_back(tpos, rgb);
        return true;
    }
    else if (cmd == "point") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'point <x> <y> <z> <r> <g> <b>'");
        }
        Vec4 pos = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3]), 1};
        Color rgb = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6])};
        auto tpos = xf.m * pos;
        scene.lights.emplace_back(tpos, rgb);
        return true;
    }
    else if (cmd == "quadLight") {
        if (tokens.size() != 13) {
            throw ParseException("Expected 'quadLight <x> <y> <z> <elx> <ely> <elz> <erx> <ery> <erz> <r> <g> <b>'");
        }
        Vec3 position = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3])};
        Vec3 edge1 = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6])};
        Vec3 edge2 = {parseNum<Float>(tokens[7]), parseNum<Float>(tokens[8]), parseNum<Float>(tokens[9])};
        Color rad = {parseNum<Float>(tokens[10]), parseNum<Float>(tokens[11]), parseNum<Float>(tokens[12])};
        auto v0 = transformPoint(xf.m, position);
        auto v1 = transformPoint(xf.m, position + edge1);
        auto v2 = transformPoint(xf.m, position + edge1 + edge2);
        auto v3 = transformPoint(xf.m, position + edge2);
        Vec3 e1 = v1 - v0, e2 = v3 - v0;
        Float l1 = glm::length(e1), l2 = glm::length(e2);
        if (l1 == 0 || l2 == 0 || glm::length(glm::cross(e2, e1)) < 1e-6 * l1 * l2) {
            throw ParseException("Degenerate quadLight: edges are parallel or zero-length");
        }
        Material emissive;
        emissive.emission = rad;  // occluding emitter
        auto quad = std::make_shared<Quad>(makeMaterial(emissive), v0, v1, v2, v3);
        scene.areaLights.push_back(quad);
        return true;
    }
    return false;
}

}  // namespace raytracer::parser
