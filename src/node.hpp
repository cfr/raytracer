#pragma once

#include "values.hpp"

#include <string>

namespace raytracer {

inline Vec4 transformVec(Transform m, Vec4 v) {
    return m * v;
}

inline Vec3 transformVec(Transform m, Vec3 v) {
    auto v4 = Vec4(v, 1);
    auto tv = m * v4;
    return {tv / tv.w};
}

struct Material {
    enum class Type: int {
        Diffuse,
        Specular,
        Shininess,
        Emission,
        Refraction
    };
    ColorA diffuse = {0, 0, 0, 1};
    ColorA specular = {0, 0, 0, 1};
    ColorA emission = {0, 0, 0, 1};
    Float shininess = 0;
    Float refraction = 0;
};

struct Node {
    size_t id;
    Material material;
    Transform transform = Transform{1};
    Transform inverse = Transform{1};
    Transform inverseTranspose = Transform{1};
    ColorA ambient;  // ambient light may change
};

}  // namespace raytracer
