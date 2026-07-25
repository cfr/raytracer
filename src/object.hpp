#pragma once

#include "values.hpp"
#include "brdf.hpp"

#include <string>

namespace raytracer {

namespace brdf {

enum class Type : int { Phong, GGX };

}

struct Material {
    brdf::Type brdfType;
    Color diffuse = colors::white;   // kd
    Color specular = colors::black;  // ks
    Color emission = colors::black;
    Float t = 0;                     // t = avg(ks) / (avg(ks) + avg(kd))
    Float shininess = 0;
    Float refraction = 0;
    Float roughness = 0;

    void precomputeT() {
        auto& s = specular;
        auto& d = diffuse;
        auto ks = (s.x + s.y + s.z) / 3.0f;
        auto kd = (d.x + d.y + d.z) / 3.0f;
        if (ks == 0) {
            t = 0;
        } else {
            t = ks / (ks + kd);
        }
    }
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
