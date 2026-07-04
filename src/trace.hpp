#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "light.hpp"

#include <glm/geometric.hpp>

#include <limits>
#include <memory>

namespace raytracer {

Color trace(const Ray ray, const Vec3 eye, const Scene& scene, int depth, const Integrator& integrator) {
    Color color{0, 0, 0, 1};
    if (depth <= 0) {
        return color;
    }

    // TODO: implement hit and light parametrized by integrator
    // put areaLights in bvh?
    auto hit = scene.bvh.intersect(ray);

    if (!hit) {
        if (integrator.type != Integrator::Type::Whitted) {
            Float nearest = inf;
            for (const auto& quad: scene.areaLights) {
                auto t = quad.intersect(ray);
                if (t < nearest) {
                    color = quad.radiance;
                }
            }
        }
        return color;
    }

    const Hittable* object = hit->object;

    switch (integrator.type) {
    case Integrator::Type::Whitted:
        color = whitted(eye, *object, *hit, scene);
        break;
    case Integrator::Type::AnalyticDirect:
        return analytic(*object, *hit, scene);
    case Integrator::Type::Direct:
        return direct(eye, *object, *hit, scene, integrator);
    }

    if (object->material.refraction > 0) {
        bool front = glm::dot(ray.dir, hit->normal) < 0;
        Vec3 normal = front ? hit->normal : -hit->normal;

        Float ri = front ? 1.0/object->material.refraction
                         : object->material.refraction;
        Float cosT = std::fmin(glm::dot(-ray.dir, normal), 1.0);
        Float sinT = std::sqrt(1.0 - cosT*cosT);

        Float r0 = (1 - ri) / (1 + ri);
        r0 *= r0;
        Float fr = r0 + (1-r0) * std::pow(1 - cosT, 5);  // Schlick

        Vec3 dir = (ri*sinT > 1.0 || fr > 1.0) ? glm::reflect(ray.dir, normal)
                                               : glm::refract(ray.dir, normal, ri);
        Float offset = Hittable::step;
        auto scattered = Ray{hit->point + offset*dir, dir};
        return color + trace(scattered, hit->point, scene, depth - 1, integrator);
    }

    Float reflectivity = glm::length(Vec3{object->material.specular});
    if (reflectivity > 0) {
        Vec3 incoming = glm::normalize(ray.dir);
        Vec3 reflect = glm::reflect(incoming, hit->normal);

        Float offset = Hittable::step;
        Ray reflectionRay{hit->point + offset*reflect, reflect};

        Color reflected = trace(reflectionRay, hit->point, scene, depth - 1, integrator);
        color += object->material.specular * reflected;
    }

    return color;
}

}  // namespace raytracer

