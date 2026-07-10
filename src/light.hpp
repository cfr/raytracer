#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "quad.hpp"

#include <glm/geometric.hpp>
#include <glm/exponential.hpp>

#include <algorithm>
#include <limits>

namespace raytracer {

Color phong(Vec3 eye, const Hit& hit, const Material& material, const Light& light) {
    auto eyedir = glm::normalize(eye - hit.point);

    Vec3 ldir;
    if (light.position.w == 0) {
        ldir = glm::normalize(Vec3{light.position});
    } else {
        auto lpos = Vec3{light.position / light.position.w};
        ldir = glm::normalize(lpos - hit.point);
    }
    auto halfvec = glm::normalize(ldir + eyedir);

    Float nDotL = glm::dot(hit.normal, ldir);
    auto lambert = material.diffuse * light.color * std::max<Float>(nDotL, 0);

    Float nDotH = glm::dot(hit.normal, halfvec);
    auto specular = material.specular * light.color * glm::pow(std::max<Float>(nDotH, 0), material.shininess);
    return lambert + specular;
}

Color whitted(Vec3 eye, const Hittable& object, const Hit& hit, const Scene& scene) {
    auto color = object.ambient + object.material.emission;

    for (const auto& source : scene.lights) {
        bool isPoint = source.position.w > 0;  // not directional light
        auto srcDir = Vec3(source.position);
        if (isPoint) {
            srcDir = Vec3(source.position) - hit.point;
        }
        srcDir = glm::normalize(srcDir);
        Float offset = Hittable::step;  // to prevent self-intersection
        auto shadowRay = Ray{hit.point + offset*srcDir, srcDir};
        Float distance =
            isPoint ? glm::distance(Vec3(source.position), hit.point) : inf;
        if (scene.bvh.occluded(shadowRay, distance, hit.object)) { continue; }

        Float attenuation = 1.0;

        if (isPoint) {
            attenuation = scene.attenuation.factor(distance);
        }

        color += attenuation * phong(eye, hit, object.material, source);
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

Color direct(Vec3 eye, const Hittable& object, const Hit& hit, const Scene& scene, Gen& gen, size_t samples, bool stratify) {
    Color color = colors::black;
    Vec3 wo = glm::normalize(eye - hit.point);

    size_t nx = stratify ? size_t(std::sqrt(Float(samples))) : 1;
    while (nx > 1 && samples % nx != 0) { --nx; }
    size_t ny = stratify ? samples / nx : 1;

    for (const auto& quad : scene.areaLights) {
        // skip co-planar light
        if (glm::dot(quad.normal, hit.point - quad.v0) < Hittable::step) { continue; }
        Color qcol = colors::black;
        for (size_t i = 0; i < samples; i++) {
            Vec2 u;
            if (stratify) {
                size_t sx = i % nx;
                size_t sy = i / nx;
                u = {(sx + gen()) / nx, (sy + gen()) / ny};
            } else {
                u = {gen(), gen()};
            }
            Vec3 xl = quad.sample(u);
            Vec3 d = xl - hit.point;
            Float d2 = glm::dot(d, d);
            Float cosI = glm::dot(hit.normal, d);
            if (cosI <= 0 || d2 < step2) continue;
            // single-sided light, no abs
            Float cosL = glm::dot(quad.normal, -d);
            Float r = std::sqrt(d2);
            Vec3 wi = d / r;
            Ray shadow{hit.point + Hittable::step*hit.normal, wi};
            if (scene.bvh.occluded(shadow, r - Hittable::step)) continue;
            qcol += phongBRDF(wi, wo, hit.normal, object.material) * (cosI * cosL / (d2 * d2));
        }
        color += qcol * quad.radiance * (quad.area / samples);
    }
    return color;
}

Color analytic(const Hittable& object, const Hit& hit, const Scene& scene) {
    auto color = colors::black;

    for (const auto& source : scene.areaLights) {
        color += object.material.diffuse / pi * source.radiance * source.irradiance(hit.point, hit.normal);
    }
    return color;
}

}  // namespace raytracer
