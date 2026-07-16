#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "parser/common.hpp"

#include <memory>
#include <vector>
#include <string>

namespace raytracer::parser {

bool parseLights(const std::vector<std::string>& tokens, Object& obj, Scene& scene, size_t& objId) {
    auto cmd = tokens[0];
    if (cmd == "ambient") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'ambient <r> <g> <b>'");
        }
        Color rgb = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3])};
        obj.ambient = rgb;
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
        Color rgb = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6])};
        auto tpos = obj.transform * pos;
        scene.lights.emplace_back(tpos, rgb);
        return true;
    }
    else if (cmd == "point") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'point <x> <y> <z> <r> <g> <b>'");
        }
        Vec4 pos = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3]), 1};
        Color rgb = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]), parseNum<Float>(tokens[6])};
        auto tpos = obj.transform * pos;
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
        auto v0 = transformVec3(obj.transform, position);
        auto v1 = transformVec3(obj.transform, position + edge1);
        auto v2 = transformVec3(obj.transform, position + edge1 + edge2);
        auto v3 = transformVec3(obj.transform, position + edge2);
        if (glm::length(glm::cross(v3 - v0, v1 - v0)) < Hittable::step) {
            throw ParseException("Degenerate quadLight: edges are parallel or zero-length");
        }
        Object light;
        light.id = objId++;
        light.material.emission = rad;  // occluding emitter
        auto quad = std::make_shared<Quad>(light, v0, v1, v2, v3, rad);
        scene.areaLights.push_back(quad);
        return true;
    }
    return false;
}

}  // namespace raytracer::parser
