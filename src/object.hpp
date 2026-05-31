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
    Color diffuse = {0, 0, 0, 1};
    Color specular = {0, 0, 0, 1};
    Color emission = {0, 0, 0, 1};
    Float shininess = 0;
    Float refraction = 0;
};

struct Object {
    size_t id;
    Material material;
    Transform transform = Transform{1};
    Transform inverse = Transform{1};
    Transform inverseTranspose = Transform{1};
    Color ambient = {0, 0, 0, 1};  // per object
};

}  // namespace raytracer
