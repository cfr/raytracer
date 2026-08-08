#pragma once

#include "values.hpp"

#include <glm/glm.hpp>

namespace raytracer {

namespace brdf {

enum class Type : int { Phong, GGX };

}

struct Material {
    brdf::Type brdfType = brdf::Type::Phong;
    Color diffuse = colors::black;   // kd
    Color specular = colors::black;  // ks
    Color emission = colors::black;
    Color ambient = colors::black;   // whitted-only per object
    Float t = 0;                     // t = avg(ks) / (avg(ks) + avg(kd))
    Float shininess = 0;
    Float refraction = 0;
    Float roughness = 0;

    void precomputeT() {
        auto& s = specular;
        auto& d = diffuse;
        auto ks = (s.x + s.y + s.z) / 3.0f;
        auto kd = (d.x + d.y + d.z) / 3.0f;
        if (ks == 0 && kd == 0) {
            t = 1.0;
        } else {
            t = ks / (ks + kd);
        }
    }

    bool emissive() const {
        return glm::any(glm::greaterThan(emission, Color{0}));
    }

    bool reflective() const {
        return glm::any(glm::greaterThan(specular, Color{0}));
    }

    bool refractive() const {
        return refraction > 0;
    }
};

}  // namespace raytracer
