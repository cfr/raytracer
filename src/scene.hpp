#pragma once

#include "image.hpp"
#include "camera.hpp"
#include "ray.hpp"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <array>
#include <memory>
#include <optional>

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
    vec3 diffuse = {0, 0, 0};
    vec3 specular = {0, 0, 0};
    vec3 emission = {0, 0, 0};
    float shininess = 0;
};

struct Attenuation {
    float constant  = 1;
    float linear    = 0;
    float quadratic = 0;
};

struct Light {
    enum class Type: int {
        Directional,
        Point
    };
    Type type;
    vec3 position = {0, 0, 0};
    vec3 rgb = {0, 0, 0};
    mat4 transform = mat4{1};
};

struct Node {
    Material material;
    mat4 transform = mat4{1};
    vec3 ambient; // ambient light changes between objs
};

struct Hittable: Node {
    Hittable(const Node& n) : Node{n} {};
    virtual std::optional<Hit> hit(Ray ray) = 0;
    virtual ~Hittable() = default;
};

struct Scene {
    Image image {0, 0};
    size_t depth = 5;
    std::string output = "out.ppm";
    Camera camera;
    Attenuation attenuation;
    std::vector<vec3> vertices;
    std::vector<Light> lights;
    std::vector<std::shared_ptr<Hittable>> nodes;
};

}
