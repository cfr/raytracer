#pragma once

#include "values.hpp"

#include <glm/vector_relational.hpp>

#include <cstdint>
#include <memory>

namespace aktis {

namespace brdf {

enum class Type : std::uint8_t { Phong, GGX };

}

constexpr Float minRoughness = 1e-3;

struct Material {
    brdf::Type brdfType = brdf::Type::Phong;
    Color diffuse = colors::black;   // kd
    Color specular = colors::black;  // ks
    Color emission = colors::black;
    Color ambient = colors::black;  // whitted-only per object
    Float t = 0;                    // t = avg(ks) / (avg(ks) + avg(kd))
    Float shininess = 0;
    Float refraction = 0;
    Float roughness = minRoughness;

    void precomputeT() {
        auto& s = specular;
        auto& d = diffuse;
        auto ks = (s.x + s.y + s.z) / Float(3);
        auto kd = (d.x + d.y + d.z) / Float(3);
        if (ks == 0 && kd == 0) {
            t = 1;
        } else {
            t = ks / (ks + kd);
        }
    }

    [[nodiscard]] bool emissive() const {
        return glm::any(glm::greaterThan(emission, Color{0}));
    }

    [[nodiscard]] bool reflective() const {
        return glm::any(glm::greaterThan(specular, Color{0}));
    }

    [[nodiscard]] bool refractive() const {
        return refraction > 1;
    }
};

using MaterialPtr = std::shared_ptr<Material const>;

}  // namespace aktis
