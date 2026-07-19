#pragma once

#include "values.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "bvh.hpp"
#include "quad.hpp"
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
    int depth = 5;
    size_t threads = 0;
    Integrator integrator;
    std::string output = "out.ppm";
};

struct Scene {
    Attenuation attenuation;
    std::vector<Light> lights;
    std::vector<std::shared_ptr<Quad>> areaLights;
    BoundingVolumeHierarchy<ManagedObject> bvh;
};

}  // namespace raytracer
