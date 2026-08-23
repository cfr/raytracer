#pragma once

#include "dielectric.hpp"
#include "integrator.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "shade.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/geometric.hpp>

#include <optional>

namespace aktis {

inline Color traceWhitted(Ray const& ray, Scene const& scene, int depth) {
    if (depth <= 0) {
        return colors::black;
    }

    auto hit = scene.bvh.intersect(ray);
    if (!hit) {
        return colors::black;
    }

    Hittable const* object = hit->object;
    auto color = whitted(*hit, scene);

    if (object->material->refractive()) {
        auto f = dielectric::fresnel(*hit);
        color += f.reflectance * traceWhitted(offset(*hit, f.wr), scene, depth - 1);
        if (!f.tir) {
            color +=
                (Float(1) - f.reflectance) * traceWhitted(offset(*hit, f.wt), scene, depth - 1);
        }
    } else if (object->material->reflective()) {
        color += object->material->specular
                 * traceWhitted(offset(*hit, glm::reflect(ray.dir, hit->normal)), scene, depth - 1);
    }

    return color;
}

inline Color traceAnalytic(Ray const& ray, Scene const& scene) {
    auto hit = scene.bvh.intersect(ray);
    if (!hit) {
        return colors::black;
    }
    return analytic(*hit, scene);
}

inline Color traceDirect(Ray const& ray, Scene const& scene, Integrator const& integrator,
                         Sampler& sampler) {
    auto hit = scene.bvh.intersect(ray);
    if (!hit) {
        return colors::black;
    }
    return emitted(*hit) + direct(*hit, scene, integrator, sampler, false);
}

class PathTracer {
    Scene const& scene_;
    Integrator const& integrator_;
    Sampler& sampler_;

  public:
    struct State {
        Color throughput = colors::white;
        Float pdfPrev = 0;
        int bounces = 0;
        bool primary = false;
        bool delta = false;
    };

    PathTracer(Scene const& scene, Integrator const& integrator, Sampler& sampler)
        : scene_{scene}, integrator_{integrator}, sampler_{sampler} {}

    Color trace(Ray const& ray, State s) {
        auto hit = scene_.bvh.intersect(ray);
        return hit ? shade(*hit, s) : colors::black;
    }

    Color shade(Hit const& hit, State s) {
        bool const nee = integrator_.nextEvent != Integrator::NEE::Off;
        bool const mis = integrator_.nextEvent == Integrator::NEE::MIS;

        Color le = colors::black;
        // NOTE: single-sided emitter
        if (hit.object->material->emissive() && hit.front) {
            if (s.primary || !nee || s.delta) {
                le = hit.object->material->emission;
            } else if (mis) {
                // direct() samples all lights, no need in 1/n
                Float const pl = importance::pdfLight(hit);
                Float const w = importance::misWeight(s.pdfPrev, pl);
                le = w * hit.object->material->emission;
            }
        }
        if (s.bounces >= integrator_.depth)
            return le;

        bool const refractive = hit.object->material->refractive();
        Color const ldirect =
            (nee && !refractive) ? direct(hit, scene_, integrator_, sampler_, mis) : colors::black;

        std::optional<Sample> sample;
        if (refractive) {
            sample = dielectric::sample(hit, sampler_.unit());
        } else {
            sample = integrator_.sample(hit, sampler_.unit(), sampler_.unit2());
        }
        if (!sample)
            return le + ldirect;

        Color lweight =
            sample->delta ? sample->f : sample->f * cosTheta(hit, sample->wi) / sample->pdf;
        Ray const bounce = offset(hit, sample->wi);

        if (!integrator_.russianRoulette) {
            return le + ldirect
                   + lweight
                         * trace(bounce, {.throughput = s.throughput,
                                          .pdfPrev = sample->pdf,
                                          .bounces = s.bounces + 1,
                                          .primary = false,
                                          .delta = sample->delta});
        }

        auto throughput = s.throughput * lweight;
        Float q =
            Float(1)
            - glm::min(glm::max(glm::max(throughput.x, throughput.y), throughput.z), Float(1));
        if (s.bounces >= 3) {
            // floor the termination to compensate delta bounces
            q = glm::max(q, Float(0.05));
        }
        if (q >= Float(1) || sampler_.unit() < q) {
            return le + ldirect;
        }
        Float const boost = Float(1) / (Float(1) - q);
        lweight *= boost;
        return le + ldirect
               + lweight
                     * trace(bounce, {.throughput = throughput * boost,
                                      .pdfPrev = sample->pdf,
                                      .bounces = s.bounces + 1,
                                      .primary = false,
                                      .delta = sample->delta});
    }
};

inline Color trace(Ray const& ray, Scene const& scene, Integrator const& integrator,
                   Sampler& sampler) {
    switch (integrator.type) {
    case Integrator::Type::Whitted:
        return traceWhitted(ray, scene, integrator.depth);
    case Integrator::Type::AnalyticDirect:
        return traceAnalytic(ray, scene);
    case Integrator::Type::Direct:
        return traceDirect(ray, scene, integrator, sampler);
    case Integrator::Type::PathTracer: {
        auto hit = scene.bvh.intersect(ray);
        if (!hit) {
            return colors::black;
        }
        PathTracer pt{scene, integrator, sampler};
        // shade shared primary rays
        auto color = colors::black;
        for (size_t s = 0; s < integrator.samplesPerPixel; s++) {
            color += pt.shade(*hit, {.primary = true});
        }
        return color / static_cast<Float>(integrator.samplesPerPixel);
    }
    }
    return colors::black;
}

inline Color tracePixel(RayCaster const& caster, Point point, Scene const& scene,
                        Integrator const& integrator, Sampler& sampler, Stratify2D const& pixels) {
    if (integrator.type != Integrator::Type::PathTracer || !integrator.jitter) {
        return trace(caster.cast(point), scene, integrator, sampler);
    }

    PathTracer pt{scene, integrator, sampler};
    auto color = colors::black;
    for (size_t s = 0; s < integrator.samplesPerPixel; ++s) {
        auto ray = caster.cast(point, pixels.unit2(sampler.unit2(), s));
        color += pt.trace(ray, {.primary = true});
    }
    return color / static_cast<Float>(integrator.samplesPerPixel);
}

}  // namespace aktis
