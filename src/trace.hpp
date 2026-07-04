#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "light.hpp"

#include <glm/geometric.hpp>

#include <limits>
#include <memory>

namespace raytracer {

Color intersectQuad(const Ray ray, const AreaLight& quad) {
    const Vec3 e1 = quad.v1 - quad.v0;
    const Vec3 e2 = quad.v3 - quad.v0;
    const Vec3 n = glm::cross(e1, e2);

    const Float denom = dot(n, ray.dir);
    Color black{0, 0, 0, 1};
    if (glm::abs(denom) < Hittable::step) return black;

    // TODO: ray-plane
    const Float t = dot(quad.v0 - ray.eye, n) / denom;
    if (t <= Hittable::step) return black;

    const Vec3 p = ray.at(t);

    const Float d0 = glm::dot(glm::cross(quad.v1 - quad.v0, p - quad.v0), n);
    const Float d1 = glm::dot(glm::cross(quad.v2 - quad.v1, p - quad.v1), n);
    const Float d2 = glm::dot(glm::cross(quad.v3 - quad.v2, p - quad.v2), n);
    const Float d3 = glm::dot(glm::cross(quad.v0 - quad.v3, p - quad.v3), n);

    // any
    const bool inside = (d0 >= 0.0 && d1 >= 0.0 && d2 >= 0.0 && d3 >= 0.0) ||
                        (d0 <= 0.0 && d1 <= 0.0 && d2 <= 0.0 && d3 <= 0.0);

    return inside ? quad.radiance : Color(0.0f);
}

Color trace(const Ray ray, const Vec3 eye, const Scene& scene, int depth, Integrator integrator) {
    Color color{0, 0, 0, 1};
    if (depth <= 0) {
        return color;
    }

    auto hit = scene.bvh.intersect(ray);

    if (!hit) {
        if (integrator == Integrator::AnalyticDirect) {
            for (const auto& quad: scene.areaLights) {
                color += intersectQuad(ray, quad);
            }
        }
        return color;
    }

    const Hittable* object = hit->object;

    switch (integrator) {
    case Integrator::Whitted:
        color = whitted(eye, *object, *hit, scene);
        break;
    case Integrator::AnalyticDirect:
        return analytic(*object, *hit, scene);
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

