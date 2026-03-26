#pragma once

#include "scene.hpp"
#include "ray.hpp"
#include "light.hpp"

#include <glm/geometric.hpp>

#include <limits>

namespace raytracer {

vec4 trace(const Ray ray, const vec3 eye, const Scene& scene, int depth) {

    if (depth <= 0) {
        return vec4{0, 0, 0, 1};
    }

    std::shared_ptr<Hittable> object = nullptr;
    Hit hit { std::numeric_limits<float>::max(), {}, {} };

    for (const auto& node : scene.nodes) {
        auto h = node->intersect(ray);
        if (h && h->t < hit.t) {
            object = node;
            hit = *h;
        }
    }

    if (!object) {
        return vec4{0, 0, 0, 1};
    }

    vec4 color = colorOf(eye, *object, hit, scene);

    float reflectivity = glm::length(vec3{object->material.specular});
    if (reflectivity > 0) {
        vec3 incoming = glm::normalize(ray.dir);
        vec3 reflect = glm::reflect(incoming, hit.normal);

        float offset = Hittable::step;
        Ray reflectionRay{hit.point + offset*reflect, reflect};

        vec4 reflectedColor = trace(reflectionRay, hit.point, scene, depth - 1);
        color += object->material.specular * reflectedColor;
        //color = glm::mix(color, reflectedColor, reflectivity);
    }

    return color;
}

} // namespace raytracer

