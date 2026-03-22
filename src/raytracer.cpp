
#include "parser.hpp"
#include "scene.hpp"
#include "image.hpp"

#include <print>
#include <fstream>
#include <stdexcept>

using namespace raytracer;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println("usage: raytracer scene.test");
        return 0;
    }

    try {
        auto scene = parser::readScene(argv[1]);
        for(auto px: scene.image) {
            auto r = static_cast<float>(px.x) / (scene.image.size().width-1);
            auto g = static_cast<float>(px.y) / (scene.image.size().height-1);
            auto b = 0.0;
            scene.image.set(px, {r, g, b});
            //std::print("({},{}) ", pixel.x, pixel.y);
        }
        std::ofstream file;
        file.open(scene.output);
        if (!file) { throw std::runtime_error("Can't open file '" + scene.output + "'"); }
        scene.image.writePPM(file);
        std::println("Saved {}.", scene.output);
    } catch (const std::exception& e) {
        std::println("{}", e.what());
    }
}
