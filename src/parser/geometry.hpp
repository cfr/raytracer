#pragma once

#include "values.hpp"
#include "hittable.hpp"
#include "shape/sphere.hpp"
#include "shape/triangle.hpp"
#include "parser/common.hpp"

#include <string>
#include <vector>
#include <memory>

namespace raytracer::parser {

bool parseGeometry(const std::vector<std::string>& tokens, std::vector<Vec3>& vertices, const std::shared_ptr<const Material>& cur, const Transforms& xf, std::vector<ManagedObject>& objects) {
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
        if (tokens.size() != 4) {
            throw ParseException("Expected 'tri <idx1> <idx2> <idx3>'");
        }
        auto id0 = parseNum<size_t>(tokens[1]);
        auto id1 = parseNum<size_t>(tokens[2]);
        auto id2 = parseNum<size_t>(tokens[3]);
        try {
            auto a = vertices.at(id0);
            auto b = vertices.at(id1);
            auto c = vertices.at(id2);
            // bake the transform
            auto tri = std::make_shared<Triangle>(cur,
                transformVec3(xf.m, a),
                transformVec3(xf.m, b),
                transformVec3(xf.m, c));
            objects.push_back(tri);
        } catch (const std::exception& e) {
            throw ParseException(e.what());
        }
        return true;
    }
    else if (cmd == "sphere") {
        if (tokens.size() != 5) {
            throw ParseException("Expected 'sphere <x> <y> <z> <r>'");
        }
        Vec3 c;
        c.x = parseNum<Float>(tokens[1]);
        c.y = parseNum<Float>(tokens[2]);
        c.z = parseNum<Float>(tokens[3]);
        Float r = parseNum<Float>(tokens[4]);
        std::shared_ptr<const Transforms> sxf;
        if (xf.m != identity) {
            sxf = std::make_shared<Transforms>(xf);
        }
        objects.push_back(std::make_shared<Sphere>(cur, c, r, std::move(sxf)));
        return true;
    }
    // TODO: maxvertnorms, vertexnormal, trinormal
    return false;
}

}  // namespace raytracer::parser
