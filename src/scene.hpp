#pragma once

#include "values.hpp"
#include "tolerance.hpp"
#include "hittable.hpp"
#include "bvh.hpp"
#include "shape/quad.hpp"
#include "integrator.hpp"

#include <glm/common.hpp>

#include <vector>
#include <optional>
#include <string>

namespace raytracer {

struct Attenuation {
    Float constant  = 1;
    Float linear    = 0;
    Float quadratic = 0;

    Float factor(Float distance) const {
        Float d = glm::max(distance, tol::tmin);
        return 1 / (constant + d * linear + d * d * quadratic);
    }
};

struct Light {
    Vec4 position = {0, 0, 0, 0};
    Color color = colors::black;

    bool point() const {
        return position.w > 0;  // not directional light
    }
};

struct Settings {
    Size size = {640, 480};
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
