#pragma once

#include "values.hpp"
#include "parser/common.hpp"

#include <string>
#include <optional>
#include <format>
#include <string_view>

namespace raytracer::parser {

struct Args {
    std::string path;
    std::optional<Seed> seed;
    std::optional<size_t> spp;
    std::optional<size_t> width;
    std::optional<size_t> threads;
    std::optional<std::string> out;
    bool jitter = false;
    bool quiet = false;
    bool p3 = false;
};

inline Args parseArgs(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "--jitter") { a.jitter = true; }
        else if (arg == "--quiet") { a.quiet = true; }
        else if (arg == "--p3") { a.p3 = true; }
        else if (arg == "--seed") {
            if (++i == argc) { throw ParseException("Expected '--seed <value>'"); }
            a.seed = parseNum<Seed>(argv[i]);
        }
        else if (arg == "--spp") {
            if (++i == argc) { throw ParseException("Expected '--spp <value>'"); }
            a.spp = parseNum<size_t>(argv[i]);
            if (*a.spp == 0) { throw ParseException("--spp must be > 0"); }
        }
        else if (arg == "--width") {
            if (++i == argc) { throw ParseException("Expected '--width <value>'"); }
            a.width = parseNum<size_t>(argv[i]);
            if (*a.width == 0) { throw ParseException("--width must be > 0"); }
        }
        else if (arg == "--threads") {
            if (++i == argc) { throw ParseException("Expected '--threads <value>'"); }
            a.threads = parseNum<size_t>(argv[i]);
        }
        else if (arg == "--out") {
            if (++i == argc) { throw ParseException("Expected '--out <path>'"); }
            a.out = argv[i];
        }
        else if (arg.starts_with("--")) { throw ParseException(std::format("Unknown option: '{}'", arg)); }
        else if (!a.path.empty()) { throw ParseException("Multiple scene files given"); }
        else { a.path = arg; }
    }
    if (a.path.empty()) { throw ParseException("No scene file given"); }
    return a;
}

}  // namespace raytracer::parser
