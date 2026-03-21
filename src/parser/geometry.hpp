#pragma once

#include "scene.hpp"
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
        Triangle t;
        t.indices[0] = parseNum<int>(tokens[1]);
        t.indices[1] = parseNum<int>(tokens[2]);
        t.indices[2] = parseNum<int>(tokens[3]);
        auto tri = std::make_shared<TriangleNode>(node, t);
        scene.nodes.push_back(tri);
        return true;
    }
    else if (cmd == "sphere") {
        if (tokens.size() != 5) {
            throw ParseException("Expected 'sphere <x> <y> <z> <r>'");
        }
        Sphere s;
        s.position.x = parseNum<float>(tokens[1]);
        s.position.y = parseNum<float>(tokens[2]);
        s.position.z = parseNum<float>(tokens[3]);
        s.radius = parseNum<float>(tokens[4]);
        auto sphere = std::make_shared<SphereNode>(node, s);
        scene.nodes.push_back(sphere);
        return true;
    }
    // TODO: maxvertnorms, vertexnormal, trinormal
    return false;
}

}
