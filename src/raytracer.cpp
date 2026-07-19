#include "integrator.hpp"
#include "values.hpp"
#include "parser.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "ray.hpp"
#include "trace.hpp"
#include "pool.hpp"
#include "row.hpp"
#include "bvh.hpp"

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

void write(const std::string& path, const Image& image) {
    std::ofstream file;
    file.open(path);
    if (!file) {
        throw std::runtime_error("Can't open file '" + path + "'");
    }
    image.writePPM(file);
    std::println("Saved {}.", path);
}

Row traceRow(const Scene& scene, const RayCaster& caster, Integrator integrator, int depth, Seed seed, size_t y) {
    Row row(y, caster.size());
    auto sampler = integrator.sampler(seed);

    for (auto point : row) {
        auto ray = caster.cast(point);
        Color color = trace(ray, scene, integrator, sampler, depth);
        auto clamped = Color{glm::clamp(color, Color{0}, Color{1})};
        row.set(point, clamped);
    }
    return row;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println("usage: raytracer scene.test");
        return 0;
    }

    try {
        auto [scene, camera, settings] = parser::readScene(argv[1]);
        auto image = Image{settings.size};
        auto caster = RayCaster{camera, settings.size};

        size_t threads = settings.threads ? settings.threads : std::thread::hardware_concurrency();
        ThreadPool pool{threads};
        std::random_device rd;

        std::vector<std::future<Row>> rows;
        for (auto y : std::views::iota(0uz, settings.size.height)) {
            auto seed = rd();
            auto row = pool.submit(traceRow, std::cref(scene), std::cref(caster), settings.integrator, settings.depth, seed, y);
            rows.push_back(std::move(row));
        }

        for (size_t y = 0; auto& r : rows) {
            auto row = r.get();
            for (auto point : row) {
                image.set(point, row.get(point));
            }
            std::print("\r{}%", ++y * 100 / settings.size.height);
            std::fflush(stdout);
        }
        std::print("\n");
        write(settings.output, image);
    } catch (const std::exception& e) {
        std::println("{}", e.what());
    }
}
