#pragma once

#include "integrator.hpp"
#include "values.hpp"
#include "scene.hpp"
#include "quad.hpp"
#include "brdf.hpp"

#include <glm/geometric.hpp>
#include <glm/exponential.hpp>

#include <algorithm>
#include <limits>

namespace raytracer {

Color blinnPhong(Vec3 eyedir, Vec3 ldir, const Hit& hit, const Material& material, const Light& light) {
    Float nDotL = glm::dot(hit.normal, ldir);
    auto lambert = material.diffuse * light.color * std::max<Float>(nDotL, 0);

    Float nDotH = glm::dot(hit.normal, halfvec(ldir, eyedir));
    auto specular = material.specular * light.color * glm::pow(std::max<Float>(nDotH, 0), material.shininess);
    return lambert + specular;
}

Color whitted(Vec3 eyedir, const Hittable& object, const Hit& hit, const Scene& scene) {
    auto color = object.ambient + object.material.emission;

    for (const auto& source : scene.lights) {
        bool isPoint = source.position.w > 0;  // not directional light
        auto ldir = Vec3(source.position);
        if (isPoint) {
            ldir = Vec3(source.position) - hit.point;
        }
        ldir = glm::normalize(ldir);
        Float offset = Hittable::step;  // to prevent self-intersection
        auto shadowRay = Ray{hit.point + offset*ldir, ldir};
        Float distance =
            isPoint ? glm::distance(Vec3(source.position), hit.point) : inf;
        if (scene.bvh.occluded(shadowRay, distance, hit.object)) { continue; }

        Float attenuation = 1.0;

        if (isPoint) {
            attenuation = scene.attenuation.factor(distance);
        }

        color += attenuation * blinnPhong(eyedir, ldir, hit, object.material, source);
    }
    return color;
}

constexpr Float step2 = Hittable::step * Hittable::step;

Color direct(Vec3 wo, const Hittable& object, const Hit& hit, const Scene& scene, Sampler& sampler) {
    Color color = colors::black;
    auto samples = sampler.samples();
    Basis b{hit.normal};

    for (const auto& quad : scene.areaLights) {
        if (quad.get() == &object) { continue; }
        // skip co-planar light
        if (glm::dot(quad->planeNormal, hit.point - quad->v0) < Hittable::step) { continue; }
        Color qcol = colors::black;
        Vec3 origin = hit.point + Hittable::step*hit.normal;
        for (size_t i = 0; i < samples; i++) {
            Vec3 xl = quad->sample(sampler.unit2(i));
            Vec3 d = xl - hit.point;
            Float d2 = glm::dot(d, d);
            Float cosI = glm::dot(hit.normal, d);
            // single-sided light, no abs
            Float cosL = glm::dot(quad->planeNormal, -d);
            if (cosI <= 0 || cosL <= 0 || d2 < step2) continue;
            Float r = std::sqrt(d2);
            Vec3 wi = d / r;
            Vec3 sd = xl - origin;
            Float rl = glm::length(sd);
            Ray shadow{origin, sd / rl};
            if (scene.bvh.occluded(shadow, rl - Hittable::step, hit.object)) continue;
            Vec3 woL = b.toLocal(wo);
            qcol += brdf::eval(object.material, woL, b.toLocal(wi)) * (cosI * cosL / (d2 * d2));
        }
        color += qcol * quad->radiance * (quad->area / samples);
    }
    return color;
}

Color analytic(const Hittable& object, const Hit& hit, const Scene& scene) {
    auto color = object.material.emission;

    for (const auto& source : scene.areaLights) {
        color += object.material.diffuse / pi * source->radiance * source->irradiance(hit.point, hit.normal);
    }
    return color;
}

}  // namespace raytracer
