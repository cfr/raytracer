#pragma once

#include "brdf.hpp"
#include "integrator.hpp"
#include "scene.hpp"
#include "shape/quad.hpp"
#include "tolerance.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

namespace aktis {

inline Ray offset(Hit const& h, Vec3 wi) {
    Vec3 const n = glm::dot(h.normal, wi) < 0 ? -h.normal : h.normal;
    return Ray{h.point + n * tol::offset(h.point), wi};
}

inline Color emitted(Hit const& h) {
    return h.front ? h.object->material->emission : colors::black;
}

inline Color blinnPhong(Vec3 eyedir, Vec3 ldir, Hit const& hit, Material const& material,
                        Light const& light) {
    Float const nDotL = glm::dot(hit.normal, ldir);
    auto lambert = material.diffuse * light.color * glm::max(nDotL, Float(0));

    Float const nDotH = glm::dot(hit.normal, halfvec(ldir, eyedir));
    auto specular =
        material.specular * light.color * glm::pow(glm::max(nDotH, Float(0)), material.shininess);
    return lambert + specular;
}

inline Color whitted(Hit const& hit, Scene const& scene) {
    auto color = hit.object->material->ambient + emitted(hit);

    for (auto const& source : scene.lights) {
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
        if (scene.bvh.occluded(shadowRay, distance, hit.object)) {
            continue;
        }

        color += attenuation * blinnPhong(hit.wo, ldir, hit, *hit.object->material, source);
    }
    return color;
}

inline Color direct(Hit const& hit, Scene const& scene, Integrator const& integrator,
                    Sampler& sampler, bool mis) {
    Color color = colors::black;
    auto samples = mis ? 1 : sampler.samples();
    Basis const b{hit.normal};

    Float const e = tol::offset(hit.point);
    Vec3 const origin = hit.point + e * hit.normal;
    Float const minDist2 = e * e;

    for (auto const& quad : scene.areaLights) {
        if (quad.get() == hit.object) {
            continue;
        }

        // single-sided emitter, no abs
        Float const cosL = glm::dot(quad->planeNormal, hit.point - quad->v0);
        if (cosL < e) {
            continue;
        }  // behind or co-planar

        Color qcol = colors::black;
        for (size_t i = 0; i < samples; i++) {
            Vec3 const xl = quad->sample(samples == 1 ? sampler.unit2() : sampler.unit2(i));
            Vec3 const d = xl - hit.point;
            Float const d2 = glm::dot(d, d);
            Float const cosI = glm::dot(hit.normal, d);
            if (cosI <= 0 || d2 < minDist2) {
                continue;
            }
            Float const r = glm::sqrt(d2);
            Vec3 const wi = d / r;
            Vec3 const sd = xl - origin;
            Float const rl = glm::length(sd);
            Ray const shadow{origin, sd / rl};
            if (scene.bvh.occluded(shadow, rl - e, hit.object)) {
                continue;
            }
            auto f = brdf::eval(*hit.object->material, b.toLocal(hit.wo), b.toLocal(wi));
            Float w = 1;
            if (mis) {
                Float const cosLn = cosL / r;
                Float const pl = d2 / (quad->area * cosLn);
                Float const pb = integrator.pdf(hit, wi);
                w = importance::misWeight(pl, pb);
            }
            qcol += w * f * (cosI * cosL / (d2 * d2));
        }
        color += qcol * quad->material->emission * (quad->area / static_cast<Float>(samples));
    }
    return color;
}

inline Color analytic(Hit const& hit, Scene const& scene) {
    auto color = emitted(hit);

    for (auto const& source : scene.areaLights) {
        color += hit.object->material->diffuse / pi * source->material->emission
                 * source->irradiance(hit.point, hit.normal);
    }
    return color;
}

}  // namespace aktis
