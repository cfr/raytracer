#pragma once

#include "values.hpp"
#include "scene.hpp"

#include <glm/geometric.hpp>
#include <glm/exponential.hpp>

#include <algorithm>
#include <limits>

namespace raytracer {

Color light(const Vec3 eye, const Hit& hit, const Material& material, const Light& light) {
    auto eyedir = glm::normalize(eye - hit.point);

    auto color = Color(0, 0, 0, 1);
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
    auto phong = material.specular * light.color * glm::pow(std::max<Float>(nDotH, 0), material.shininess);
    color += lambert + phong;

    return color;
}

Color whitted(const Vec3 eye, const Hittable& object, const Hit& hit, const Scene& scene) {
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

        color += attenuation*light(eye, hit, object.material, source);
    }
    return color;
}

Float quadIrradiance(const AreaLight& quad, const Vec3 r, const Vec3 normal) {
    Vec3 v0 = quad.position;
    Vec3 v1 = quad.position + quad.edgeL;
    Vec3 v2 = quad.position + quad.edgeL + quad.edgeR;
    Vec3 v3 = quad.position + quad.edgeR;
    Vec3 u0 = glm::normalize(v0 - r);
    Vec3 u1 = glm::normalize(v1 - r);
    Vec3 u2 = glm::normalize(v2 - r);
    Vec3 u3 = glm::normalize(v3 - r);
    Vec3 phi = Vec3{0.0};
    phi += glm::acos(glm::dot(u0, u1)) * glm::normalize(glm::cross(u0, u1));
    phi += glm::acos(glm::dot(u1, u2)) * glm::normalize(glm::cross(u1, u2));
    phi += glm::acos(glm::dot(u2, u3)) * glm::normalize(glm::cross(u2, u3));
    phi += glm::acos(glm::dot(u3, u0)) * glm::normalize(glm::cross(u3, u0));
    return 0.5 * glm::dot(phi, normal);
}

Color analytic(const Vec3 eye, const Hittable& object, const Hit& hit, const Scene& scene) {
    auto color = Color{Vec3(0.0), 1.0};

    for (const auto& source : scene.areaLights) {
        color += object.material.diffuse / pi * source.radiance * quadIrradiance(source, hit.point, hit.normal);
    }
    return color;
}

}  // namespace raytracer
