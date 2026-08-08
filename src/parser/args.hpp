#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "parser/common.hpp"

#include <string>
#include <optional>
#include <format>
#include <string_view>

namespace raytracer::parser {

struct Args { std::string path; std::optional<Seed> seed; bool jitter = false; };

inline Args parseArgs(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "--jitter") { a.jitter = true; }
        else if (arg == "--seed") {
            if (++i == argc) { throw ParseException("Expected '--seed <value>'"); }
            a.seed = parseNum<Seed>(argv[i]);
        }
        else if (arg.starts_with("--")) { throw ParseException(std::format("Unknown option: '{}'", arg)); }
        else if (!a.path.empty()) { throw ParseException("Multiple scene files given"); }
        else { a.path = arg; }
    }
    if (a.path.empty()) { throw ParseException("No scene file given"); }
    return a;
}

}  // namespace raytracer::parser
