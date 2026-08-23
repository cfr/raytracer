#pragma once

#include "parser/common.hpp"
#include "transforms.hpp"
#include "values.hpp"

#include <string>
#include <vector>

namespace aktis::parser {

inline bool parseTransform(Tokens const& tokens, TStack& stack) {
    auto const& cmd = tokens[0];
    if (cmd == "translate") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'translate <x> <y> <z>'");
        }
        Vec3 const t = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]),
                        parseNum<Float>(tokens[3])};
        stack.translate(t);
        return true;
    }
    if (cmd == "scale") {
        if (tokens.size() != 4) {
            throw ParseException("Expected 'scale <x> <y> <z>'");
        }
        Vec3 const t = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]),
                        parseNum<Float>(tokens[3])};
        stack.scale(t);
        return true;
    }
    if (cmd == "rotate") {
        if (tokens.size() != 5) {
            throw ParseException("Expected 'rotate <x> <y> <z> <a>'");
        }
        Vec3 const axis = {parseNum<Float>(tokens[1]), parseNum<Float>(tokens[2]),
                           parseNum<Float>(tokens[3])};
        auto angle = parseNum<Float>(tokens[4]);
        stack.rotate(axis, angle);
        return true;
    }
    if (cmd == "pushTransform") {
        stack.push();
        return true;
    }
    if (cmd == "popTransform") {
        stack.pop();
        return true;
    }
    return false;
}

}  // namespace aktis::parser
