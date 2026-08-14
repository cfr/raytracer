#pragma once

#include "values.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "hittable.hpp"
#include "bvh.hpp"
#include "shape/quad.hpp"
#include "integrator.hpp"

#include <vector>
#include <array>
#include <memory>
#include <optional>
#include <string>

namespace raytracer {

struct Attenuation {
    Float constant  = 1;
    Float linear    = 0;
    Float quadratic = 0;

    Float factor(Float distance) const {
        return 1 / (constant + distance * linear + distance * distance * quadratic);
    }
};

struct Light {
    enum class Type: int {
        Directional,
        Point
    };
    Vec4 position = {0, 0, 0, 0};
    Color color = colors::black;
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
