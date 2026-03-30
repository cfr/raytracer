#include "values.hpp"
#include "parser.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "frame.hpp"
#include "ray.hpp"
#include "light.hpp"
#include "trace.hpp"

#include <print>
#include <fstream>
#include <stdexcept>

#include <new>
#include <future>
#include <thread>
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

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println("usage: raytracer scene.test");
        return 0;
    }

    try {
        auto [scene, camera, settings] = parser::readScene(argv[1]);

        auto image = Image{settings.size};
        auto caster = RayCaster{camera, settings.size};
        auto frame = Frame{settings.size};

        for(auto point: frame) {
            auto ray = caster.cast(point);
            Color color = trace(ray, camera.eye, scene, settings.depth);
            auto clamped = Color{glm::clamp(color, Color{0}, Color{1})};
            image.set(point, clamped);
        }
        write(settings.output, image);
    } catch (const std::exception& e) {
        std::println("{}", e.what());
    }
}
