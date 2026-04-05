#include "values.hpp"
#include "parser.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "ray.hpp"
#include "trace.hpp"
#include "pool.hpp"
#include "row.hpp"

#include <print>
#include <fstream>
#include <stdexcept>
#include <ranges>
#include <future>
#include <thread>
#include <vector>
#include <string>
#include <utility>

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

Row traceRow(const Scene& scene, RayCaster& caster, const Settings& settings, size_t y) {
    Row row(y, caster.size());

    for (auto point : row) {
        if (settings.samples > 1) {
            auto rays = caster.castMultisample(point, settings.samples);
            Color res{0};
            for (auto ray : rays) {
                Color color = trace(ray, caster.eye(), scene, settings.depth);
                auto clamped = Color{glm::clamp(color, Color{0}, Color{1})};
                res += clamped;
            }
            res /= static_cast<Float>(settings.samples);
            row.set(point, res);
        } else {
            auto ray = caster.cast(point);
            Color color = trace(ray, caster.eye(), scene, settings.depth);
            auto clamped = Color{glm::clamp(color, Color{0}, Color{1})};
            row.set(point, clamped);
        }
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

        ThreadPool pool{settings.threads};

        std::vector<std::future<Row>> rows;
        for (auto y : std::views::iota(0ul, settings.size.height)) {
            auto row = pool.submit(traceRow, scene, caster, settings, y);
            rows.push_back(std::move(row));
        }

        for (auto& r : rows) {
            auto row = r.get();
            for (auto point : row) {
                image.set(point, row.get(point));
            }
        }

        /* single threaded version for refrence:
        for (auto point : Frame{settings.size}) {
            auto ray = caster.cast(point);
            Color color = trace(ray, caster.eye(), scene, settings.depth);
            auto clamped = Color{glm::clamp(color, Color{0}, Color{1})};
            image.set(point, clamped);
        }
        */
        write(settings.output, image);
    } catch (const std::exception& e) {
        std::println("{}", e.what());
    }
}
