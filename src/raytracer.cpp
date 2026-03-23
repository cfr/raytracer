#include "parser.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "ray.hpp"

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
            Image::RGB color = {0, 0, 0};
            for (const auto& node: scene.nodes) {
                auto h = node->hit(ray);
                if (h) {
                    color = node->ambient;
                }
            }
            scene.image.set(px, color);
        }
        write(scene.output, scene.image);
    } catch (std::exception& e) {
        std::println("{}", e.what());
    }
}

