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

Color traceWhitted(const Ray& ray, const Scene& scene, int depth) {
    if (depth <= 0) {
        return colors::black;
    }

    auto hit = scene.bvh.intersect(ray);
    if (!hit) { return colors::black; }

    const Hittable* object = hit->object;
    auto color = whitted(-ray.dir, *object, *hit, scene);

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
            color += (1.0 - fr) * traceWhitted(refractRay, scene, depth - 1);
        }
    }

    bool reflective = glm::any(glm::greaterThan(object->material.specular, Color{0.0}));
    if (reflective || object->material.refraction > 0) {
        Vec3 incoming = glm::normalize(ray.dir);
        Vec3 reflect = glm::reflect(incoming, normal);

        Float offset = Hittable::step;
        Ray reflectionRay{hit->point + offset*reflect, reflect};

        Color reflected = traceWhitted(reflectionRay, scene, depth - 1);
        if (object->material.refraction > 0) {
            color += fr * reflected;
        } else {
            color += object->material.specular * reflected;
        }
    }

    return color;
}

Color traceAnalytic(const Ray& ray, const Scene& scene) {
    auto hit = scene.bvh.intersect(ray);
    if (!hit) { return colors::black; }
    return analytic(*hit->object, *hit, scene);
}

Color traceDirect(const Ray& ray, const Scene& scene, Sampler& sampler) {
    auto hit = scene.bvh.intersect(ray);
    if (!hit) { return colors::black; }
    return hit->object->material.emission + direct(-ray.dir, *hit->object, *hit, scene, sampler);
}

Color tracePath(const Ray& ray, const Scene& scene, const Integrator& integrator, Sampler& sampler, int depth, bool primary, Color throughput) {
    auto hit = scene.bvh.intersect(ray);
    if (!hit) { return colors::black; }

    Color le = (integrator.nextEvent && !primary) ? colors::black : hit->object->material.emission;
    if (depth == 0) { return le; } // depth < 0 -- infinite bounces

    Color ldirect = integrator.nextEvent ? direct(-ray.dir, *hit->object, *hit, scene, sampler) : colors::black;

    auto w = sampler.hemisphere(hit->normal, 0);
    auto f = phongBRDF(w, -ray.dir, hit->normal, hit->object->material);
    auto bounce = Ray{hit->point + Hittable::step * w, w};
    //return le + ldirect + pi * f * tracePath(bounce, scene, integrator, sampler, depth - 1, false, throughput);
    auto lweight = 2.0*pi * f * glm::dot(hit->normal, w);
    if (!integrator.russianRoulette) {
        return le + ldirect + lweight * tracePath(bounce, scene, integrator, sampler, depth - 1, false, throughput);
    }
    throughput *= lweight;
    Float q = 1.0 - glm::min(glm::max(glm::max(throughput.x, throughput.y), throughput.z), 1.0);
    if (q >= 1.0 || sampler.unit() < q) {
        return le + ldirect;
    } else {
        Float boost = 1.0 / (1.0 - q);
        lweight *= boost;
        throughput *= boost;
        return le + ldirect + lweight * tracePath(bounce, scene, integrator, sampler, depth - 1, false, throughput);
    }
}

Color trace(const Ray& ray, const Scene& scene, const Integrator& integrator, Sampler& sampler, int depth) {
    switch (integrator.type) {
    case Integrator::Type::Whitted:
        return traceWhitted(ray, scene, depth);
    case Integrator::Type::AnalyticDirect:
        return traceAnalytic(ray, scene);
    case Integrator::Type::Direct:
        return traceDirect(ray, scene, sampler);
    case Integrator::Type::PathTracer:
        {
        auto color = colors::black;
        // TODO: move to traceRow, add antialiasing
        for (size_t s = 0; s < integrator.samplesPerPixel; s++) {
            color += tracePath(ray, scene, integrator, sampler, depth, true, colors::white);
        }
        return color/static_cast<Float>(integrator.samplesPerPixel);
        }
    default:
        return colors::black;
    }

}

}  // namespace raytracer

