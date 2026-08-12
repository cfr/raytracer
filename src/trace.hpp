#pragma once

#include "integrator.hpp"
#include "values.hpp"
#include "scene.hpp"
#include "ray.hpp"
#include "shade.hpp"

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
    auto color = whitted(*hit, scene);

    Float fr = 0.0;

    if (object->material->refractive()) {
        Float ri = hit->front ? Float(1)/object->material->refraction
                              : object->material->refraction;
        Float cosI = std::fmin(glm::dot(hit->wo, hit->normal), Float(1));
        Float sinT = ri * std::sqrt(Float(1) - cosI*cosI);
        bool tir = sinT > Float(1);

        Float r0 = (Float(1) - ri) / (Float(1) + ri);
        r0 *= r0;
        fr = tir ? Float(1) : r0 + (Float(1) - r0) * std::pow(Float(1) - cosI, 5);

        if (!tir) {
            Vec3 tdir = glm::refract(ray.dir, hit->normal, ri);
            Ray refractRay = offset(*hit, tdir);
            color += (Float(1) - fr) * traceWhitted(refractRay, scene, depth - 1);
        }
    }

    if (object->material->reflective() || object->material->refractive()) {
        Vec3 incoming = glm::normalize(ray.dir);
        Vec3 reflect = glm::reflect(incoming, hit->normal);

        Ray reflectionRay = offset(*hit, reflect);

        Color reflected = traceWhitted(reflectionRay, scene, depth - 1);
        if (object->material->refractive()) {
            color += fr * reflected;
        } else {
            color += object->material->specular * reflected;
        }
    }

    return color;
}

Color traceAnalytic(const Ray& ray, const Scene& scene) {
    auto hit = scene.bvh.intersect(ray);
    if (!hit) { return colors::black; }
    return analytic(*hit, scene);
}

Color traceDirect(const Ray& ray, const Scene& scene, const Integrator& integrator, Sampler& sampler) {
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
        int depth = 0;
        bool primary = false;
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
            if (s.primary || !nee) {
                le = hit.object->material->emission;
            } else if (mis) {
                // direct() samples all lights, no need in 1/n
                Float pl = importance::pdfLight(hit);
                Float w = importance::misWeight(s.pdfPrev, pl);
                le = w * hit.object->material->emission;
            }
        }
        if (s.depth == 0) return le;

        Color ldirect = nee ? direct(hit, scene_, integrator_, sampler_, mis) : colors::black;

        auto sample = integrator_.sample(hit, sampler_.unit(), sampler_.unit2());
        if (!sample) return le + ldirect;

        Color lweight = sample->f * cosTheta(hit, sample->wi) / sample->pdf;
        Ray bounce = offset(hit, sample->wi);

        if (!integrator_.russianRoulette) {
            return le + ldirect + lweight * trace(bounce, {s.throughput, sample->pdf, s.depth - 1, false});
        }

        auto throughput = s.throughput * lweight;
        Float q = Float(1) - glm::min(glm::max(glm::max(throughput.x, throughput.y), throughput.z), Float(1));
        if (q >= Float(1) || sampler_.unit() < q) {
            return le + ldirect;
        }
        Float boost = Float(1) / (Float(1) - q);
        lweight *= boost;
        return le + ldirect + lweight * trace(bounce, {throughput * boost, sample->pdf, s.depth - 1, false});
    }
};

Color trace(const Ray& ray, const Scene& scene, const Integrator& integrator, Sampler& sampler, int depth) {
    switch (integrator.type) {
    case Integrator::Type::Whitted:
        return traceWhitted(ray, scene, depth);
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
            color += pt.shade(*hit, {.depth = depth, .primary = true});
        }
        return color/static_cast<Float>(integrator.samplesPerPixel);
        }
    }
    return colors::black;
}

Color tracePixel(const RayCaster& caster, Point point, const Scene& scene, const Integrator& integrator, Sampler& sampler, const Stratify2D& pixels, int depth) {
    if (integrator.type != Integrator::Type::PathTracer || !integrator.jitter) {
        return trace(caster.cast(point), scene, integrator, sampler, depth);
    }

    PathTracer pt{scene, integrator, sampler};
    auto color = colors::black;
    for (size_t s = 0; s < integrator.samplesPerPixel; ++s) {
        auto ray = caster.cast(point, pixels.unit2(sampler.unit2(), s));
        color += pt.trace(ray, {.depth = depth, .primary = true});
    }
    return color / static_cast<Float>(integrator.samplesPerPixel);
}

}  // namespace raytracer

