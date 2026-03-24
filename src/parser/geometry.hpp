#pragma once

#include "scene.hpp"
#include "sphere.hpp"
#include "triangle.hpp"
#include "parser/common.hpp"

namespace raytracer::parser {

inline bool parseGeometry(const std::vector<std::string>& tokens, const Node& node, Scene& scene) {
    auto cmd = tokens[0];
    if (cmd == "maxverts") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'maxverts <count>'");
        }
        int count = parseNum<int>(tokens[1]);
        if (count) { scene.vertices.reserve(count); }
        return true;
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
        return true;
    }
    else if (cmd == "tri") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'tri <idx1> <idx2> <idx3>'");
        }
        auto id0 = parseNum<int>(tokens[1]);
        auto id1 = parseNum<int>(tokens[2]);
        auto id2 = parseNum<int>(tokens[3]);
        auto transform = mat4(1); // node.transform
        auto a = transformVec(transform, scene.vertices[id0]);
        auto b = transformVec(transform, scene.vertices[id1]);
        auto c = transformVec(transform, scene.vertices[id2]);
        auto tri = std::make_shared<Triangle>(node, a, b, c);
        scene.nodes.push_back(tri);
        return true;
    }
    else if (cmd == "sphere") {
        if (tokens.size() != 5) {
            throw ParseException("Expected 'sphere <x> <y> <z> <r>'");
        }
        vec3 c;
        c.x = parseNum<float>(tokens[1]);
        c.y = parseNum<float>(tokens[2]);
        c.z = parseNum<float>(tokens[3]);
        float r = parseNum<float>(tokens[4]);
        auto sphere = std::make_shared<Sphere>(node, c, r);
        scene.nodes.push_back(sphere);
        return true;
    }
    // TODO: maxvertnorms, vertexnormal, trinormal
    return false;
}

}
