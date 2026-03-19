#pragma once

#include "image.hpp"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <array>

namespace raytracer {

using vec3 = glm::vec3;
using mat4 = glm::mat4;

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
    vec3 center;
    vec3 up;
    float fov;
};

struct Sphere {
    vec3 position;
    float radius;
};

struct Node {
    Material material;
    mat4 transform;
};

struct SphereNode: Node {
    Sphere sphere;
};

struct Triangle {
    std::array<int, 3> indices;
};

struct MeshNode: Node {
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<Triangle> triangles;
};

struct Scene {
    Image image {0, 0};
    size_t depth = 5;
    std::string output;
    Camera camera;
    Ambient ambient;
    Attenuation attenuation;
    std::vector<Light> lights;
    std::vector<Node> nodes;
};

}
