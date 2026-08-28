#pragma once

#include "parser/common.hpp"
#include "shapes.hpp"
#include "transforms.hpp"
#include "values.hpp"

#include <glm/trigonometric.hpp>

#include <vector>

namespace aktis::parser {

// NOLINTBEGIN(readability-function-cognitive-complexity)
inline bool parseGeometry(Tokens const& tokens, std::vector<Vec3>& vertices, Material const& mat,
                          Transforms const& xf, std::vector<Transforms>& tstore,
                          std::vector<Material>& materials, std::vector<Shape>& objects) {
    auto const& cmd = tokens.at(0);
    // vertices.at() throws std::out_of_range, the shape factories throw
    // std::invalid_argument; both surface as ParseException with the line number
    auto addShape = [&objects](auto&& make) {
        try {
            objects.push_back(make());
        } catch (std::exception const& ex) {
            throw ParseException(ex.what());
        }
    };
    if (cmd == "maxverts") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'maxverts <count>'");
        }
        int const count = parseNum<int>(tokens[1]);
        if (count != 0) {
            vertices.reserve(count);
        }
        return true;
    }
    if (cmd == "vertex") {
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
    if (cmd == "tri") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'tri <idx1> <idx2> <idx3>'");
        }
        auto id0 = parseNum<size_t>(tokens[1]);
        auto id1 = parseNum<size_t>(tokens[2]);
        auto id2 = parseNum<size_t>(tokens[3]);
        addShape([&] {
            auto a = vertices.at(id0);
            auto b = vertices.at(id1);
            auto c = vertices.at(id2);
            // bake the transform
            return Shape::triangle(makeMaterial(mat, materials), transformPoint(xf.m, a),
                                   transformPoint(xf.m, b), transformPoint(xf.m, c));
        });
        return true;
    }
    if (cmd == "sphere") {
        if (tokens.size() != 5) {
            throw ParseException("Expected 'sphere <x> <y> <z> <r>'");
        }
        Vec3 c;
        c.x = parseNum<Float>(tokens[1]);
        c.y = parseNum<Float>(tokens[2]);
        c.z = parseNum<Float>(tokens[3]);
        Float const r = parseNum<Float>(tokens[4]);
        if (r <= 0) {
            throw ParseException("sphere requires r > 0");
        }
        objects.push_back(
            Shape::sphere(makeMaterial(mat, materials), c, r, makeTransform(xf, tstore)));
        return true;
    }
    if (cmd == "quadric") {
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
        Float const j = parseNum<Float>(tokens[7]);
        Vec2 e;
        e.x = parseNum<Float>(tokens[8]);
        e.y = parseNum<Float>(tokens[9]);
        addShape([&] {
            return Shape::quadric(makeMaterial(mat, materials), makeTransform(xf, tstore),
                                  Quadric::of(q, l, j, e));
        });
        return true;
    }
    if (cmd == "cone") {
        if (tokens.size() != 3) {
            throw ParseException("Expected 'cone <halfAngle> <h>'");
        }
        Float const halfAngle = glm::radians(parseNum<Float>(tokens[1]));
        Float const h = parseNum<Float>(tokens[2]);
        addShape([&] {
            return Shape::quadric(makeMaterial(mat, materials), makeTransform(xf, tstore),
                                  Quadric::cone(halfAngle, h));
        });
        return true;
    }
    if (cmd == "conerh") {
        if (tokens.size() != 3) {
            throw ParseException("Expected 'conerh <r> <h>'");
        }
        Float const r = parseNum<Float>(tokens[1]);
        Float const h = parseNum<Float>(tokens[2]);
        addShape([&] {
            return Shape::quadric(makeMaterial(mat, materials), makeTransform(xf, tstore),
                                  Quadric::coneRH(r, h));
        });
        return true;
    }
    if (cmd == "cylinder") {
        if (tokens.size() != 3) {
            throw ParseException("Expected 'cylinder <r> <h>'");
        }
        Float const r = parseNum<Float>(tokens[1]);
        Float const h = parseNum<Float>(tokens[2]);
        if (h <= 0) {
            throw ParseException("cylinder requires h > 0");
        }
        addShape([&] {
            return Shape::quadric(makeMaterial(mat, materials), makeTransform(xf, tstore),
                                  Quadric::cylinder(r, Vec2{-h / 2, h / 2}));
        });
        return true;
    }
    if (cmd == "paraboloid") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'paraboloid <h>'");
        }
        Float const h = parseNum<Float>(tokens[1]);
        if (h <= 0) {
            throw ParseException("paraboloid requires h > 0");
        }
        addShape([&] {
            return Shape::quadric(makeMaterial(mat, materials), makeTransform(xf, tstore),
                                  Quadric::paraboloid(Vec2{0, h}));
        });
        return true;
    }
    // TODO: maxvertnorms, vertexnormal, trinormal
    return false;
}
// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace aktis::parser
