#pragma once

#include "integrator.hpp"
#include "values.hpp"
#include "scene.hpp"
#include "shape/quad.hpp"
#include "brdf.hpp"

#include <glm/geometric.hpp>
#include <glm/exponential.hpp>

#include <algorithm>
#include <limits>

namespace raytracer {

inline Ray offset(const Hit& h, Vec3 wi) {
    Vec3 n = glm::dot(h.normal, wi) < 0 ? -h.normal : h.normal;
    return Ray{h.point + n * Hittable::eps(h.point), wi};
}

inline Color emitted(const Hit& h) {
    return h.front ? h.object->material->emission : colors::black;
}

inline Color blinnPhong(Vec3 eyedir, Vec3 ldir, const Hit& hit, const Material& material, const Light& light) {
    Float nDotL = glm::dot(hit.normal, ldir);
    auto lambert = material.diffuse * light.color * glm::max(nDotL, Float(0));

    Float nDotH = glm::dot(hit.normal, halfvec(ldir, eyedir));
    auto specular = material.specular * light.color * glm::pow(glm::max(nDotH, Float(0)), material.shininess);
    return lambert + specular;
}

inline Color whitted(const Hit& hit, const Scene& scene) {
    auto color = hit.object->material->ambient + emitted(hit);

    for (const auto& source : scene.lights) {
        auto ldir = Vec3(source.position);
        auto attenuation = Float(1);
        auto distance = inf;
        if (source.point()) {
            ldir = Vec3(source.position) - hit.point;
            distance = glm::distance(Vec3(source.position), hit.point);
            attenuation = scene.attenuation.factor(distance);
        }
        ldir = glm::normalize(ldir);
        auto shadowRay = offset(hit, ldir);
        if (scene.bvh.occluded(shadowRay, distance, hit.object)) { continue; }

        color += attenuation * blinnPhong(hit.wo, ldir, hit, *hit.object->material, source);
    }
    return color;
}

inline Color direct(const Hit& hit, const Scene& scene, const Integrator& integrator, Sampler& sampler, bool mis) {
    Color color = colors::black;
    auto samples = mis ? 1 : sampler.samples();
    Basis b{hit.normal};

    Float e = Hittable::eps(hit.point);
    Vec3 origin = hit.point + e * hit.normal;
    Float minDist2 = e * e;

    for (const auto& quad : scene.areaLights) {
        if (quad.get() == hit.object) { continue; }

        // single-sided emitter, no abs
        const Float cosL = glm::dot(quad->planeNormal, hit.point - quad->v0);
        if (cosL < e) { continue; }  // behind or co-planar

        Color qcol = colors::black;
        for (size_t i = 0; i < samples; i++) {
            Vec3 xl = quad->sample(samples == 1 ? sampler.unit2() : sampler.unit2(i));
            Vec3 d = xl - hit.point;
            Float d2 = glm::dot(d, d);
            Float cosI = glm::dot(hit.normal, d);
            if (cosI <= 0 || d2 < minDist2) { continue; }
            Float r = std::sqrt(d2);
            Vec3 wi = d / r;
            Vec3 sd = xl - origin;
            Float rl = glm::length(sd);
            Ray shadow{origin, sd / rl};
            if (scene.bvh.occluded(shadow, rl - e, hit.object)) { continue; }
            auto f = brdf::eval(*hit.object->material, b.toLocal(hit.wo), b.toLocal(wi));
            Float w = 1;
            if (mis) {
                Float cosLn = cosL / r;
                Float pl = d2 / (quad->area * cosLn);
                Float pb = integrator.pdf(hit, wi);
                w = importance::misWeight(pl, pb);
            }
            qcol += w * f * (cosI * cosL / (d2 * d2));
        }
        color += qcol * quad->material->emission * (quad->area / samples);
    }
    return color;
}

inline Color analytic(const Hit& hit, const Scene& scene) {
    auto color = emitted(hit);

    for (const auto& source : scene.areaLights) {
        color += hit.object->material->diffuse / pi * source->material->emission * source->irradiance(hit.point, hit.normal);
    }
    return color;
}

}  // namespace raytracer
