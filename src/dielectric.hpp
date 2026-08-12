#pragma once

#include "values.hpp"
#include "hittable.hpp"
#include "brdf.hpp"

#include <glm/geometric.hpp>

namespace raytracer {

namespace dielectric {

// world space
struct Fresnel {
    Vec3 wr;
    Vec3 wt;
    Float reflectance;
    bool tir;
};

inline Fresnel fresnel(const Hit& hit) {
    Float ior = hit.object->material->refraction;
    Vec3 n = hit.normal;
    Float eta = hit.front ? Float(1) / ior : ior;
    Float cosI = glm::clamp(glm::dot(hit.wo, n), Float(0), Float(1));
    Float sinT2 = eta * eta * (Float(1) - cosI * cosI);
    Vec3 wr = glm::reflect(-hit.wo, n);

    if (sinT2 >= Float(1)) {
        return {wr, wr, Float(1), true};
    }

    Float cosT = glm::sqrt(Float(1) - sinT2);
    Float c = hit.front ? cosI : cosT;
    Float r0 = (Float(1) - ior) / (Float(1) + ior);
    r0 *= r0;
    Float m = Float(1) - c, m2 = m * m;
    Float fr = r0 + (Float(1) - r0) * m2 * m2 * m;

    return {wr, -eta * hit.wo + (eta * cosI - cosT) * n, fr, false};
}

// NOTE: omits the 1/η² radiance scaling
inline Sample sample(const Hit& hit, Float u) {
    auto f = fresnel(hit);
    return Sample{(f.tir || u < f.reflectance) ? f.wr : f.wt, colors::white, Float(1), true};
}

}  // namespace dielectric

}  // namespace raytracer
