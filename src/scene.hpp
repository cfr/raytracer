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
using vec4 = glm::vec4;
using mat4 = glm::mat4;

inline vec4 transformVec(mat4 m, vec4 v) {
    return m * v;
}

inline vec3 transformVec(mat4 m, vec3 v) {
    auto v4 = vec4(v, 1);
    auto tv = m * v4;
    return {tv / tv.w};
}

/*inline vec3 transformRay(mat4 m, vec3 r) {
    auto r4 = vec4(r, 0);
    auto tr = m * r4;
    return {tr};
}*/

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
    vec3 rgb = {0, 0, 0};
    //mat4 transform = mat4{1};
};

// TODO: rename to object or props, compose into shapes
struct Node {
    Material material;
    mat4 transform = mat4{1};
    vec3 ambient; // ambient light changes between objs
};

// TODO: move to hit
struct Hittable: Node {
    static constexpr float step = 0.0001f; // min distance for intersection

    Hittable(const Node& n) : Node{n} {};
    virtual std::optional<Hit> intersect(Ray ray) = 0;
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
