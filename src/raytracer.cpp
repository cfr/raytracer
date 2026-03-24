#include "parser.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "ray.hpp"
#include "light.hpp"
#include "trace.hpp"

#include <print>
#include <fstream>
#include <stdexcept>

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
        auto scene = parser::readScene(argv[1]);
        auto caster = RayCaster{scene.camera, scene.image.size()};
        for(auto px: scene.image) {
            auto ray = caster.cast(px);
            vec4 color = trace(ray, scene, scene.depth, scene.camera.eye);
            Image::RGB rgb = {
                std::clamp(color.r, 0.0f, 1.0f),
                std::clamp(color.g, 0.0f, 1.0f),
                std::clamp(color.b, 0.0f, 1.0f)
            };
            scene.image.set(px, rgb);
        }
        write(scene.output, scene.image);
    } catch (std::exception& e) {
        std::println("{}", e.what());
    }
}

