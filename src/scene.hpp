#pragma once

#include "image.hpp"

#include <glm/vec3.hpp>

#include <vector>
#include <array>

namespace raytracer {

using vec3 = glm::vec3;

struct Material {
    enum class Type: int {
        Diffuse,
        Specular,
        Shininess,
        Emission
    };
    vec3 diffuse = {0.0, 0.0, 0.0};
    vec3 specular = {0.0, 0.0, 0.0};
    vec3 emission = {0.0, 0.0, 0.0};
    float shininess = 0.0;
};

struct Attenuation {
    float constant  = 1.0;
    float linear    = 0.0;
    float quadratic = 0.0;
};

struct Light {
    enum class Type: int {
        Directional,
        Point
        // Ambient?
    };
    vec3 position;
    vec3 rgb;
};

struct Ambient {
    vec3 rgb = {0.2f, 0.2f, 0.2f};
};

struct Camera {
    vec3 position;
    vec3 right;
    vec3 up;
    float fov;
};

struct Triangle {
    std::array<int, 3> indices;
};

struct Scene {
    Image image {0, 0};
    Camera camera;
    std::vector<vec3> vertices;
    std::vector<Triangle> triangles;
};

}
