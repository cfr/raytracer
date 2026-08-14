#include "integrator.hpp"
#include "values.hpp"
#include "parser.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "ray.hpp"
#include "trace.hpp"
#include "pool.hpp"
#include "bvh.hpp"
#include "frame.hpp"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <functional>
#include <future>
#include <print>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace raytracer;

std::string write(std::string path, const Image& image, bool ascii) {
    if (!path.ends_with(".ppm")) { path += ".ppm"; }
    std::ofstream file;
    file.open(path, std::ios::out | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Can't open file '" + path + "'");
    }
    image.writePPM(file, ascii);
    return path;
}

Row traceRow(const Scene& scene, const RayCaster& caster, const Settings& settings, Seed seed, size_t y) {
    Row row(y, caster.size());
    auto sampler = settings.integrator.sampler(seed);
    auto pixels = settings.integrator.pixels();

    for (auto point : row) {
        Color color = tracePixel(caster, point, scene, settings.integrator, sampler, pixels);
        color = gamma(color, settings.gamma);
        auto clamped = Color{glm::clamp(color, Color{0}, Color{1})};
        row.set(point, clamped);
    }
    return row;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println("usage: raytracer [--quiet] [--seed n] [--spp s] [--width w] [--out path] [--p3] [--jitter] scene.test");
        return 0;
    }

    try {
        auto args = parser::parseArgs(argc, argv);
        auto [scene, camera, settings] = parser::readScene(args.path);
        settings.integrator.jitter = settings.integrator.jitter || args.jitter;
        if (args.seed) { settings.seed = args.seed; }
        if (args.spp) { settings.integrator.samplesPerPixel = *args.spp; }
        if (args.width) {
            auto w = *args.width;
            settings.size.height = settings.size.height * w / settings.size.width;
            settings.size.width = w;
        }
        if (args.out) { settings.output = *args.out; }
        auto image = Image{settings.size};
        auto caster = RayCaster{camera, settings.size};

        size_t threads = settings.threads ? settings.threads : std::thread::hardware_concurrency();
        ThreadPool pool{threads};
        std::random_device rd;

        if (!args.quiet) {
            std::println("Rendering {}, {}x{}, {} spp, depth {}, rr {}, jitter {}, seed = {}",
                args.path,
                settings.size.width, settings.size.height, settings.integrator.samplesPerPixel,
                settings.integrator.depth,
                settings.integrator.russianRoulette ? "on" : "off",
                settings.integrator.jitter ? "on" : "off",
                settings.seed ? std::to_string(*settings.seed) : "random");
        }

        std::vector<std::future<Row>> rows;
        for (auto y : std::views::iota(0uz, settings.size.height)) {
            auto seed = settings.seed ? splitmix(*settings.seed + y) : seed64(rd);
            auto row = pool.submit(traceRow, std::cref(scene), std::cref(caster), std::cref(settings), seed, y);
            rows.push_back(std::move(row));
        }

        for (size_t y = 0; auto& r : rows) {
            const auto row = r.get();
            for (auto point : row) {
                image.set(point, row.get(point));
            }
            if (!args.quiet) {
                std::print("\r{}%", ++y * 100 / settings.size.height);
                std::fflush(stdout);
            }
        }
        auto saved = write(settings.output, image, args.p3);
        if (!args.quiet) {
            std::println("\nSaved {}.", saved);
        }
        return 0;
    } catch (const std::exception& ex) {
        std::println(stderr, "{}", ex.what());
        return 1;
    }
}
