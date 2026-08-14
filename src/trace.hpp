#pragma once

#include "integrator.hpp"
#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "shade.hpp"
#include "dielectric.hpp"

#include <glm/geometric.hpp>

#include <limits>
#include <memory>

namespace raytracer {

inline Color traceWhitted(const Ray& ray, const Scene& scene, int depth) {
    if (depth <= 0) {
        return colors::black;
    }

    auto hit = scene.bvh.intersect(ray);
    if (!hit) { return colors::black; }

    const Hittable* object = hit->object;
    auto color = whitted(*hit, scene);

    if (object->material->refractive()) {
        auto f = dielectric::fresnel(*hit);
        color += f.reflectance * traceWhitted(offset(*hit, f.wr), scene, depth - 1);
        if (!f.tir) {
            color += (Float(1) - f.reflectance) * traceWhitted(offset(*hit, f.wt), scene, depth - 1);
        }
    } else if (object->material->reflective()) {
        color += object->material->specular
               * traceWhitted(offset(*hit, glm::reflect(ray.dir, hit->normal)), scene, depth - 1);
    }

    return color;
}

inline Color traceAnalytic(const Ray& ray, const Scene& scene) {
    auto hit = scene.bvh.intersect(ray);
    if (!hit) { return colors::black; }
    return analytic(*hit, scene);
}

inline Color traceDirect(const Ray& ray, const Scene& scene, const Integrator& integrator, Sampler& sampler) {
    auto hit = scene.bvh.intersect(ray);
    if (!hit) { return colors::black; }
    return emitted(*hit) + direct(*hit, scene, integrator, sampler, false);
}

class PathTracer {
    const Scene& scene_;
    const Integrator& integrator_;
    Sampler& sampler_;

  public:
    struct State {
        Color throughput = colors::white;
        Float pdfPrev = 0;
        int bounces = 0;
        bool primary = false;
        bool delta = false;
    };

    PathTracer(const Scene& scene, const Integrator& integrator, Sampler& sampler)
        : scene_{scene}, integrator_{integrator}, sampler_{sampler} {}

    Color trace(const Ray& ray, State s) {
        auto hit = scene_.bvh.intersect(ray);
        return hit ? shade(*hit, s) : colors::black;
    }

    Color shade(const Hit& hit, State s) {
        bool nee = integrator_.nextEvent != Integrator::NEE::Off;
        bool mis = integrator_.nextEvent == Integrator::NEE::MIS;

        Color le = colors::black;
        // NOTE: single-sided emitter
        if (hit.object->material->emissive() && hit.front) {
            if (s.primary || !nee || s.delta) {
                le = hit.object->material->emission;
            } else if (mis) {
                // direct() samples all lights, no need in 1/n
                Float pl = importance::pdfLight(hit);
                Float w = importance::misWeight(s.pdfPrev, pl);
                le = w * hit.object->material->emission;
            }
        }
        if (s.bounces >= integrator_.depth) return le;

        bool refractive = hit.object->material->refractive();
        Color ldirect = (nee && !refractive) ? direct(hit, scene_, integrator_, sampler_, mis) : colors::black;

        std::optional<Sample> sample;
        if (refractive) {
            sample = dielectric::sample(hit, sampler_.unit());
        } else {
            sample = integrator_.sample(hit, sampler_.unit(), sampler_.unit2());
        }
        if (!sample) return le + ldirect;

        Color lweight = sample->delta ? sample->f
                                      : sample->f * cosTheta(hit, sample->wi) / sample->pdf;
        Ray bounce = offset(hit, sample->wi);

        if (!integrator_.russianRoulette) {
            return le + ldirect + lweight * trace(bounce, {s.throughput, sample->pdf, s.bounces + 1, false, sample->delta});
        }

        auto throughput = s.throughput * lweight;
        Float q = Float(1) - glm::min(glm::max(glm::max(throughput.x, throughput.y), throughput.z), Float(1));
        if (s.bounces >= 3) {
            // floor the termination to compensate delta bounces
            q = glm::max(q, Float(0.05));
        }
        if (q >= Float(1) || sampler_.unit() < q) {
            return le + ldirect;
        }
        Float boost = Float(1) / (Float(1) - q);
        lweight *= boost;
        return le + ldirect + lweight * trace(bounce, {throughput * boost, sample->pdf, s.bounces + 1, false, sample->delta});
    }
};

inline Color trace(const Ray& ray, const Scene& scene, const Integrator& integrator, Sampler& sampler) {
    switch (integrator.type) {
    case Integrator::Type::Whitted:
        return traceWhitted(ray, scene, integrator.depth);
    case Integrator::Type::AnalyticDirect:
        return traceAnalytic(ray, scene);
    case Integrator::Type::Direct:
        return traceDirect(ray, scene, integrator, sampler);
    case Integrator::Type::PathTracer:
        {
        auto hit = scene.bvh.intersect(ray);
        if (!hit) { return colors::black; }
        PathTracer pt{scene, integrator, sampler};
        // shade shared primary rays
        auto color = colors::black;
        for (size_t s = 0; s < integrator.samplesPerPixel; s++) {
            color += pt.shade(*hit, {.primary = true});
        }
        return color/static_cast<Float>(integrator.samplesPerPixel);
        }
    }
    return colors::black;
}

inline Color tracePixel(const RayCaster& caster, Point point, const Scene& scene, const Integrator& integrator, Sampler& sampler, const Stratify2D& pixels) {
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

}  // namespace raytracer

