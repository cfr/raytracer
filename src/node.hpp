#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

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

struct Material {
    enum class Type: int {
        Diffuse,
        Specular,
        Shininess,
        Emission
    };
    vec4 diffuse = {0, 0, 0, 1};
    vec4 specular = {0, 0, 0, 1};
    vec4 emission = {0, 0, 0, 1};
    float shininess = 0;
};

struct Node {
    Material material;
    mat4 transform = mat4{1};
    vec4 ambient; // ambient light may change
};

}
