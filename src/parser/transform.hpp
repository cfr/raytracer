#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "tstack.hpp"
#include "parser/common.hpp"

namespace raytracer::parser {

bool parseTransform(const std::vector<std::string>& tokens, TStack& stack) {
    auto cmd = tokens[0];
    if (cmd == "translate") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'translate <x> <y> <z>'");
        }
        Vec3 t = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3])};
        stack.translate(t);
        return true;
    }
    else if (cmd == "scale") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'scale <x> <y> <z>'");
        }
        Vec3 t = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3])};
        stack.scale(t);
        return true;
    }
    else if (cmd == "rotate") {
        if (tokens.size() != 5) {
            throw ParseException("Expected 'rotate <x> <y> <z> <a>'");
        }
        Vec3 axis = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]), parseNum<Float>(tokens[3])};
        auto angle = parseNum<Float>(tokens[4]);
        stack.rotate(axis, angle);
        return true;
    }
    else if (cmd == "pushTransform") {
        stack.push();
        return true;
    }
    else if (cmd == "popTransform") {
        stack.pop();
        return true;
    }
    return false;
}

}

