#pragma once

#include "values.hpp"
#include "hittable.hpp"
#include "shape/sphere.hpp"
#include "shape/triangle.hpp"
#include "shape/quadric.hpp"
#include "parser/common.hpp"

#include <string>
#include <vector>
#include <memory>

namespace raytracer::parser {

bool parseGeometry(const std::vector<std::string>& tokens, std::vector<Vec3>& vertices, const MaterialPtr& cur, const Transforms& xf, std::vector<ObjectPtr>& objects) {
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
                transformPoint(xf.m, a),
                transformPoint(xf.m, b),
                transformPoint(xf.m, c));
            objects.push_back(tri);
        } catch (const std::exception& ex) {
            throw ParseException(ex.what());
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
        objects.push_back(std::make_shared<Sphere>(cur, c, r, sharedTransforms(xf)));
        return true;
    }
    else if (cmd == "quadric") {
        if (tokens.size() != 10) {
            throw ParseException("Expected 'quadric <a> <b> <c> 0 <h> 0 <j> <el> <eh>'");
        }
        Vec3 q;
        q.x = parseNum<Float>(tokens[1]);
        q.y = parseNum<Float>(tokens[2]);
        q.z = parseNum<Float>(tokens[3]);
        Vec3 l;
        l.x = parseNum<Float>(tokens[4]);
        l.y = parseNum<Float>(tokens[5]);
        l.z = parseNum<Float>(tokens[6]);
        Float j = parseNum<Float>(tokens[7]);
        Vec2 e;
        e.x = parseNum<Float>(tokens[8]);
        e.y = parseNum<Float>(tokens[9]);
        try {
            objects.push_back(std::make_shared<Quadric>(cur, q, l, j, e, sharedTransforms(xf)));
        } catch (const std::exception& ex) {
            throw ParseException(ex.what());
        }
        return true;
    }
    else if (cmd == "cone") {
        if (tokens.size() != 3) {
            throw ParseException("Expected 'cone <halfAngle> <h>'");
        }
        Float halfAngle = glm::radians(parseNum<Float>(tokens[1]));
        Float h = parseNum<Float>(tokens[2]);
        try {
            objects.push_back(std::make_shared<Quadric>(Quadric::cone(cur, halfAngle, h, sharedTransforms(xf))));
        } catch (const std::exception& ex) {
            throw ParseException(ex.what());
        }
        return true;
    }
    else if (cmd == "conerh") {
        if (tokens.size() != 3) {
            throw ParseException("Expected 'conerh <r> <h>'");
        }
        Float r = parseNum<Float>(tokens[1]);
        Float h = parseNum<Float>(tokens[2]);
        try {
            objects.push_back(std::make_shared<Quadric>(Quadric::coneRH(cur, r, h, sharedTransforms(xf))));
        } catch (const std::exception& ex) {
            throw ParseException(ex.what());
        }
        return true;
    }
    else if (cmd == "cylinder") {
        if (tokens.size() != 3) {
            throw ParseException("Expected 'cylinder <r> <h>'");
        }
        Float r = parseNum<Float>(tokens[1]);
        Float h = parseNum<Float>(tokens[2]);
        if (h <= 0) {
            throw ParseException("cylinder requires h > 0");
        }
        try {
            objects.push_back(std::make_shared<Quadric>(
                Quadric::cylinder(cur, r, Vec2{-h/2, h/2}, sharedTransforms(xf))));
        } catch (const std::exception& ex) {
            throw ParseException(ex.what());
        }
        return true;
    }
    else if (cmd == "paraboloid") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'paraboloid <h>'");
        }
        Float h = parseNum<Float>(tokens[1]);
        if (h <= 0) {
            throw ParseException("paraboloid requires h > 0");
        }
        try {
            objects.push_back(std::make_shared<Quadric>(
                Quadric::paraboloid(cur, Vec2{0, h}, sharedTransforms(xf))));
        } catch (const std::exception& ex) {
            throw ParseException(ex.what());
        }
        return true;
    }
    // TODO: maxvertnorms, vertexnormal, trinormal
    return false;
}

}  // namespace raytracer::parser
