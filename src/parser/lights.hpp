#pragma once

#include "parser/common.hpp"
#include "scene.hpp"
#include "shapes.hpp"
#include "tolerance.hpp"
#include "transforms.hpp"
#include "values.hpp"

#include <vector>

namespace aktis::parser {

inline bool parseLights(Tokens const& tokens, Transforms const& xf, Scene& scene,
                        std::vector<Shape>& lightShapes) {
    auto const& cmd = tokens[0];
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
            throw ParseException(
                "Expected 'attenuation <c> <l> <q>', at least one coefficient > 0");
        }
        scene.attenuation = {.constant = c, .linear = l, .quadratic = q};
        return true;
    }
    if (cmd == "directional") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'directional <x> <y> <z> <r> <g> <b>'");
        }
        Vec4 const pos = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]),
                          parseNum<Float>(tokens[3]), 0};
        Color const rgb = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]),
                           parseNum<Float>(tokens[6])};
        auto const tpos = xf.m * pos;
        scene.lights.emplace_back(tpos, rgb);
        return true;
    }
    if (cmd == "point") {
        if (tokens.size() != 7) {
            throw ParseException("Expected 'point <x> <y> <z> <r> <g> <b>'");
        }
        Vec4 const pos = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]),
                          parseNum<Float>(tokens[3]), 1};
        Color const rgb = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]),
                           parseNum<Float>(tokens[6])};
        auto const tpos = xf.m * pos;
        scene.lights.emplace_back(tpos, rgb);
        return true;
    }
    if (cmd == "quadLight") {
        if (tokens.size() != 13) {
            throw ParseException(
                "Expected 'quadLight <x> <y> <z> <elx> <ely> <elz> <erx> <ery> <erz> <r> <g> <b>'");
        }
        Vec3 const position = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]),
                               parseNum<Float>(tokens[3])};
        Vec3 const edge1 = {parseNum<Float>(tokens[4]), parseNum<Float>(tokens[5]),
                            parseNum<Float>(tokens[6])};
        Vec3 const edge2 = {parseNum<Float>(tokens[7]), parseNum<Float>(tokens[8]),
                            parseNum<Float>(tokens[9])};
        Color const rad = {parseNum<Float>(tokens[10]), parseNum<Float>(tokens[11]),
                           parseNum<Float>(tokens[12])};
        auto const v0 = transformPoint(xf.m, position);
        auto const v1 = transformPoint(xf.m, position + edge1);
        auto const v3 = transformPoint(xf.m, position + edge2);
        Vec3 const e1 = v1 - v0, e2 = v3 - v0;
        if (sinAngle(e2, e1) < tol::collinear) {
            throw ParseException("Degenerate quadLight: edges are parallel or zero-length");
        }
        Material emissive;
        emissive.emission = rad;  // occluding emitter
        lightShapes.push_back(Shape::quad(makeMaterial(emissive, scene.materials), v0, e1, e2));
        return true;
    }
    return false;
}

}  // namespace aktis::parser
