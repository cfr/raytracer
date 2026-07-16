#pragma once

#include "values.hpp"
#include "hit.hpp"
#include "sphere.hpp"
#include "triangle.hpp"
#include "parser/common.hpp"

#include <string>
#include <vector>
#include <memory>

namespace raytracer::parser {

bool parseGeometry(const std::vector<std::string>& tokens, std::vector<Vec3>& vertices, Object& obj, std::vector<ManagedObject>& objects, size_t& objId) {
    auto cmd = tokens.at(0);
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
        auto id0 = parseNum<size_t>(tokens[1]);
        auto id1 = parseNum<size_t>(tokens[2]);
        auto id2 = parseNum<size_t>(tokens[3]);
        try {
            auto a = vertices.at(id0);
            auto b = vertices.at(id1);
            auto c = vertices.at(id2);
            if (tokens.size() >= 5) {
                obj.id = parseNum<size_t>(tokens[4]);
            } else {
                obj.id = objId++;
            }
            auto tri = std::make_shared<Triangle>(obj, a, b, c);
            objects.push_back(tri);
        } catch (const std::exception& e) {
            throw ParseException(e.what());
        }
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
            obj.id = parseNum<size_t>(tokens[5]);
        } else {
            obj.id = objId++;
        }
        auto sphere = std::make_shared<Sphere>(obj, c, r);
        objects.push_back(sphere);
        return true;
    }
    // TODO: maxvertnorms, vertexnormal, trinormal
    return false;
}

}  // namespace raytracer::parser
