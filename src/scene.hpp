#pragma once

#include "bvh.hpp"
#include "hittable.hpp"
#include "integrator.hpp"
#include "shape/quad.hpp"
#include "tolerance.hpp"
#include "values.hpp"

#include <glm/common.hpp>

#include <optional>
#include <string>
#include <vector>

namespace raytracer {

struct Attenuation {
    Float constant = 1;
    Float linear = 0;
    Float quadratic = 0;

    [[nodiscard]] Float factor(Float distance) const {
        Float const d = glm::max(distance, tol::tmin);
        return 1 / (constant + d * linear + d * d * quadratic);
    }
};

struct Light {
    Vec4 position = {0, 0, 0, 0};
    Color color = colors::black;

    [[nodiscard]] bool point() const {
        return position.w > 0;  // not directional light
    }
};

struct Settings {
    Size size = {.width = 640, .height = 480};
    size_t threads = 0;
    Float gamma = 1.0;
    Integrator integrator;
    std::string output = "out.ppm";
    std::optional<Seed> seed;  // {} = use random device, otherwise per-row seed + y
};

struct Scene {
    Attenuation attenuation;
    std::vector<Light> lights;
    std::vector<QuadPtr> areaLights;
    BoundingVolumeHierarchy<ObjectPtr> bvh;
};

}  // namespace raytracer
