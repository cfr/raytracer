#pragma once

#include "image.hpp"
#include "camera.hpp"
#include "node.hpp"
#include "ray.hpp"
#include "hit.hpp"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <array>
#include <memory>
#include <optional>

namespace raytracer {

using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;

struct Attenuation {
    float constant  = 1;
    float linear    = 0;
    float quadratic = 0;

    float factor(float distance) const {
        return 1.0f / (constant + distance * linear + distance * distance * quadratic);
    }
};

struct Light {
    enum class Type: int {
        Directional,
        Point
    };
    //Type type;
    vec4 position = {0, 0, 0, 0};
    vec4 color = {0, 0, 0, 1};
    //mat4 transform = mat4{1};
};

struct Scene {
    Image image {0, 0};
    size_t depth = 5;
    std::string output = "out.ppm";
    Camera camera;
    Attenuation attenuation;
    std::vector<vec3> vertices; // TODO: store in parser
    std::vector<Light> lights;
    std::vector<std::shared_ptr<Hittable>> nodes;
};

}
