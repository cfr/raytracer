#pragma once

#include "brdf.hpp"
#include "material.hpp"
#include "values.hpp"

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

namespace aktis::dielectric {

// world space
struct Fresnel {
    Vec3 wr;
    Vec3 wt;
    Float reflectance;
    bool tir;
};

inline Fresnel fresnel(Material const& material, Hit const& hit) {
    Float const ior = material.refraction;
    Vec3 const n = hit.normal;
    Float const eta = hit.front ? Float(1) / ior : ior;
    Float const cosI = glm::clamp(glm::dot(hit.wo, n), Float(0), Float(1));
    Float const sinT2 = eta * eta * (Float(1) - cosI * cosI);
    Vec3 const wr = glm::reflect(-hit.wo, n);

    if (sinT2 >= Float(1)) {
        return {.wr = wr, .wt = wr, .reflectance = Float(1), .tir = true};
    }

    Float const cosT = glm::sqrt(Float(1) - sinT2);
    Float const c = hit.front ? cosI : cosT;
    Float r0 = (Float(1) - ior) / (Float(1) + ior);
    r0 *= r0;
    Float const m = Float(1) - c, m2 = m * m;
    Float const fr = r0 + ((Float(1) - r0) * m2 * m2 * m);

    return {
        .wr = wr, .wt = -eta * hit.wo + ((eta * cosI) - cosT) * n, .reflectance = fr, .tir = false};
}

// NOTE: omits the 1/η² radiance scaling
inline Sample sample(Material const& material, Hit const& hit, Float u) {
    auto f = fresnel(material, hit);
    return Sample{.wi = (f.tir || u < f.reflectance) ? f.wr : f.wt,
                  .f = colors::white,
                  .pdf = Float(1),
                  .delta = true};
}

}  // namespace aktis::dielectric
