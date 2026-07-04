#pragma once

#include "values.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "bvh.hpp"
#include "quad.hpp"

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
    Color color = {0, 0, 0, 1};
};

struct Integrator {
    enum class Type: int {
        Whitted,
        AnalyticDirect,
        Direct
    };
    Type type = Type::Whitted;
    size_t samples = 1;
    bool stratify = false;
};

struct Settings {
    Size size = {640, 480};
    size_t depth = 5;
    size_t threads = 0;
    Integrator integrator;
    std::string output = "out.ppm";
};

struct Scene {
    Attenuation attenuation;
    std::vector<Light> lights;
    std::vector<QuadLight> areaLights;
    BoundingVolumeHierarchy<ManagedObject> bvh;
};

}  // namespace raytracer
