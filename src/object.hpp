#pragma once

#include "values.hpp"

#include <string>

namespace raytracer {

struct Material {
    enum class Type: int {
        Diffuse,
        Specular,
        Shininess,
        Emission,
        Refraction
    };
    Color diffuse = colors::black;
    Color specular = colors::black;
    Color emission = colors::black;
    Float shininess = 0;
    Float refraction = 0;
};

struct Object {
    size_t id = 0;
    Material material;
    Transform transform = Transform{1};
    Transform inverse = Transform{1};
    Transform inverseTranspose = Transform{1};
    Color ambient = colors::black;  // per object
};

}  // namespace raytracer
