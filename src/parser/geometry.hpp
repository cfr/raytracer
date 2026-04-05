#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "sphere.hpp"
#include "triangle.hpp"
#include "parser/common.hpp"

#include <string>
#include <vector>
#include <memory>

namespace raytracer::parser {

bool parseGeometry(const std::vector<std::string>& tokens, std::vector<Vec3>& vertices, Node& node, Scene& scene) {
    static size_t nodeId = 1;
    auto cmd = tokens[0];
    if (cmd == "maxverts") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'maxverts <count>'");
        }
        int count = parseNum<int>(tokens[1]);
        if (count) { vertices.reserve(count); }
        return true;
    }
    else if (cmd == "vertex") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'vertex <x> <y> <z>'");
        }
        Vec3 v;
        v.x = parseNum<Float>(tokens[1]);
        v.y = parseNum<Float>(tokens[2]);
        v.z = parseNum<Float>(tokens[3]);
        vertices.push_back(v);
        return true;
    }
    else if (cmd == "tri") {
        if (tokens.size() < 4) {
            throw ParseException("Expected 'tri <idx1> <idx2> <idx3>'");
        }
        auto id0 = parseNum<int>(tokens[1]);
        auto id1 = parseNum<int>(tokens[2]);
        auto id2 = parseNum<int>(tokens[3]);
        auto a = vertices[id0];
        auto b = vertices[id1];
        auto c = vertices[id2];
        if (tokens.size() >= 5) {
            node.id = parseNum<size_t>(tokens[4]);
        } else {
            node.id = nodeId++;
        }
        auto tri = std::make_shared<Triangle>(node, a, b, c);
        scene.nodes.push_back(tri);
        return true;
    }
    else if (cmd == "sphere") {
        if (tokens.size() < 5) {
            throw ParseException("Expected 'sphere <x> <y> <z> <r>'");
        }
        Vec3 c;
        c.x = parseNum<Float>(tokens[1]);
        c.y = parseNum<Float>(tokens[2]);
        c.z = parseNum<Float>(tokens[3]);
        Float r = parseNum<Float>(tokens[4]);
        if (tokens.size() >= 6) {
            node.id = parseNum<size_t>(tokens[5]);
        } else {
            node.id = nodeId++;
        }
        auto sphere = std::make_shared<Sphere>(node, c, r);
        scene.nodes.push_back(sphere);
        return true;
    }
    // TODO: maxvertnorms, vertexnormal, trinormal
    return false;
}

}  // namespace raytracer::parser
