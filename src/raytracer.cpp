
#include "parser.hpp"
#include "scene.hpp"
#include "image.hpp"

#include <print>

using namespace raytracer;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println("usage: raytracer scene.test");
    }

    try {
        auto scene = parser::readScene(argv[1]);
        for(auto pixel: scene.image) {
            //std::print("({},{}) ", pixel.x, pixel.y);
        }
        std::println("\ndone");
    } catch (const std::exception& e) {
        std::println("{}", e.what());
    }
}
