#pragma once

#include "scene.hpp"
#include "tstack.hpp"
#include "parser/common.hpp"

namespace raytracer::parser {

inline bool parseTransform(const std::vector<std::string>& tokens, TStack& stack) {
    auto cmd = tokens[0];
    if (cmd == "translate") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'translate <x> <y> <z>'");
        }
        vec3 t = { parseNum<float>(tokens[1]), parseNum<float>(tokens[2]), parseNum<float>(tokens[3]) };
        stack.translate(t);
        return true;
    }
    else if (cmd == "scale") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'scale <x> <y> <z>'");
        }
        vec3 t = { parseNum<float>(tokens[1]), parseNum<float>(tokens[2]), parseNum<float>(tokens[3]) };
        stack.scale(t);
        return true;
    }
    else if (cmd == "rotate") {
        if (tokens.size() != 5) {
            throw ParseException("Expected 'rotate <x> <y> <z> <a>'");
        }
        vec3 axis = { parseNum<float>(tokens[1]), parseNum<float>(tokens[2]), parseNum<float>(tokens[3]) };
        float angle = parseNum<float>(tokens[4]);
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

