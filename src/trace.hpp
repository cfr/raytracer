#pragma once

#include "scene.hpp"
#include "ray.hpp"
#include "light.hpp"

#include <glm/geometric.hpp>

#include <limits>

namespace raytracer {

vec4 trace(Ray ray, const Scene& scene, int depth, const vec3& originalEye) {

    if (depth <= 0) {
        return vec4{0, 0, 0, 1};
    }

    std::shared_ptr<Hittable> object = nullptr;
    Hit hit { std::numeric_limits<float>::max(), {}, {} };

    for (const auto& node : scene.nodes) {
        auto h = node->intersect(ray);
        if (h && h->t > Hittable::step && h->t < hit.t) {
            object = node;
            hit = *h;
        }
    }

    if (!object) {
        return vec4{0, 0, 0, 1};
    }

    vec4 color = colorOf(originalEye, *object, hit, scene);

    float reflectivity = glm::length(object->material.specular);
    if (reflectivity > 0 && depth > 1) {
        vec3 incoming = glm::normalize(ray.dir);
        vec3 reflect = glm::reflect(incoming, hit.normal);

        float offset = 0.001f;
        Ray reflectionRay{hit.point + offset*reflect, reflect};

        vec4 reflectedColor = trace(reflectionRay, scene, depth - 1, originalEye);
        color += reflectivity*reflectedColor;
        //color = glm::mix(color, reflectedColor, reflectivity);
    }

    return color;
}

} // namespace raytracer

