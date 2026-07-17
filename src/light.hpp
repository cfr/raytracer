#pragma once

#include "integrator.hpp"
#include "values.hpp"
#include "scene.hpp"
#include "quad.hpp"

#include <glm/geometric.hpp>
#include <glm/exponential.hpp>

#include <algorithm>
#include <limits>

namespace raytracer {

Color phong(Vec3 eyedir, Vec3 ldir, const Hit& hit, const Material& material, const Light& light) {
    auto halfvec = glm::normalize(ldir + eyedir);

    Float nDotL = glm::dot(hit.normal, ldir);
    auto lambert = material.diffuse * light.color * std::max<Float>(nDotL, 0);

    Float nDotH = glm::dot(hit.normal, halfvec);
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

        color += attenuation * phong(eyedir, ldir, hit, object.material, source);
    }
    return color;
}

Color phongBRDF(Vec3 wi, Vec3 wo, Vec3 n, const Material& material) {
    Color diff = material.diffuse / pi;

    Vec3 r = glm::reflect(-wi, n);
    Float rDotV = std::max<Float>(glm::dot(r, wo), 0);
    Float s = material.shininess;
    Color spec = material.specular * ((s + 2) / (2 * pi)) * std::pow(rDotV, s);

    return diff + spec;
}

constexpr Float step2 = Hittable::step * Hittable::step;

Color direct(Vec3 wo, const Hittable& object, const Hit& hit, const Scene& scene, Sampler& sampler) {
    Color color = object.material.emission;
    auto samples = sampler.samples();

    for (const auto& quad : scene.areaLights) {
        if (quad.get() == &object) { continue; }
        // skip co-planar light
        if (glm::dot(quad->planeNormal, hit.point - quad->v0) < Hittable::step) { continue; }
        Color qcol = colors::black;
        Vec3 origin = hit.point + Hittable::step*hit.normal;
        for (size_t i = 0; i < samples; i++) {
            Vec3 xl = quad->sample(sampler.gen(i));
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
            qcol += phongBRDF(wi, wo, hit.normal, object.material) * (cosI * cosL / (d2 * d2));
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
