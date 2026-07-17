#pragma once

#include "integrator.hpp"
#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "light.hpp"

#include <glm/geometric.hpp>

#include <limits>
#include <memory>

namespace raytracer {

Color trace(const Ray ray, const Vec3 eye, const Scene& scene, int depth, const Integrator& integrator, Sampler& sampler) {
    auto color = colors::black;
    if (depth <= 0) {
        return color;
    }

    auto hit = scene.bvh.intersect(ray);
    if (!hit) { return colors::black; }

    const Hittable* object = hit->object;

    switch (integrator.type) {
    case Integrator::Type::Whitted:
        color = whitted(eye, *object, *hit, scene);
        break;
    case Integrator::Type::AnalyticDirect:
        return analytic(*object, *hit, scene);
    case Integrator::Type::Direct:
        return direct(eye, *object, *hit, scene, sampler);
    }

    Float fr = 0.0;
    Vec3 normal = hit->normal;

    if (object->material.refraction > 0) {
        bool front = glm::dot(ray.dir, hit->normal) < 0;
        normal = front ? hit->normal : -hit->normal;

        Float ri = front ? 1.0/object->material.refraction
                         : object->material.refraction;
        Float cosI = std::fmin(glm::dot(-ray.dir, normal), 1.0);
        Float sinT = ri * std::sqrt(1.0 - cosI*cosI);
        bool tir = sinT > 1.0;

        Float r0 = (1 - ri) / (1 + ri);
        r0 *= r0;
        fr = tir ? 1.0 : r0 + (1 - r0) * std::pow(1 - cosI, 5);

        if (!tir) {
            Float offset = Hittable::step;
            Vec3 tdir = glm::refract(ray.dir, normal, ri);
            Ray refractRay{hit->point + offset*tdir, tdir};
            color += (1.0 - fr) * trace(refractRay, hit->point, scene, depth - 1, integrator, sampler);
        }
    }

    Float reflectivity = glm::length(object->material.specular);
    if (reflectivity > 0 || object->material.refraction > 0) {
        Vec3 incoming = glm::normalize(ray.dir);
        Vec3 reflect = glm::reflect(incoming, normal);

        Float offset = Hittable::step;
        Ray reflectionRay{hit->point + offset*reflect, reflect};

        Color reflected = trace(reflectionRay, hit->point, scene, depth - 1, integrator, sampler);
        if (object->material.refraction > 0) {
            color += fr * reflected;
        } else {
            color += object->material.specular * reflected;
        }
    }

    return color;
}

}  // namespace raytracer

