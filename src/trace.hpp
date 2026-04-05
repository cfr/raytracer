#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "light.hpp"

#include <glm/geometric.hpp>

#include <limits>
#include <memory>

namespace raytracer {

ColorA trace(const Ray ray, const Vec3 eye, const Scene& scene, int depth) {
    if (depth <= 0) {
        return ColorA{0, 0, 0, 1};
    }

    std::shared_ptr<Hittable> object = nullptr;
    Hit hit { std::numeric_limits<Float>::max(), {}, {} };

    for (const auto& node : scene.nodes) {
        auto h = node->intersect(ray);
        if (h && h->t < hit.t) {
            object = node;
            hit = *h;
        }
    }

    if (!object) {
        return ColorA{0, 0, 0, 1};
    }

    if (object->material.refraction > 0) {
        // https://raytracing.github.io/books/RayTracingInOneWeekend.html#dielectrics/snell'slaw
        bool front = glm::dot(ray.dir, hit.normal) < 0;
        auto normal = front ? hit.normal : -hit.normal;

        Float ri = front ? (1.0/object->material.refraction)
                         : object->material.refraction;
        Float cosTheta = std::fmin(glm::dot(-ray.dir, normal), 1.0);
        Float sinTheta = std::sqrt(1.0 - cosTheta*cosTheta);

        Vec3 dir;
        if (ri*sinTheta > 1) {
            dir = glm::reflect(ray.dir, normal);
        } else {
            dir = glm::refract(ray.dir, normal, ri);
        }

        auto scattered = Ray{hit.point, dir};

        ColorA refractedColor = trace(scattered, hit.point, scene, depth - 1);
        return refractedColor;
    }

    ColorA color = colorOf(eye, *object, hit, scene);

    Float reflectivity = glm::length(Color{object->material.specular});
    if (reflectivity > 0) {
        Vec3 incoming = glm::normalize(ray.dir);
        Vec3 reflect = glm::reflect(incoming, hit.normal);

        Float offset = Hittable::step;
        Ray reflectionRay{hit.point + offset*reflect, reflect};

        ColorA reflectedColor = trace(reflectionRay, hit.point, scene, depth - 1);
        color += object->material.specular * reflectedColor;
        // color = glm::mix(color, reflectedColor, reflectivity);
    }

    return color;
}

}  // namespace raytracer

