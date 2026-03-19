#pragma once

#include "scene.hpp"
#include "parser/common.hpp"

namespace raytracer::parser {

inline bool parseMesh(const std::vector<std::string>& tokens, MeshNode& node) {
    auto cmd = tokens[0];
    if (cmd == "maxverts") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'maxverts <count>'");
        }
        int count = parseNum<int>(tokens[1]);
        if (count > 0) node.vertices.reserve(count);
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
        node.vertices.push_back(v);
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
        node.triangles.push_back(t);
        return true;
    }
    return false;
}

}
