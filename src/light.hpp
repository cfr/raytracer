#pragma once

#include "values.hpp"
#include "scene.hpp"

#include <glm/geometric.hpp>
#include <glm/exponential.hpp>

#include <algorithm>
#include <limits>

namespace raytracer {

ColorA light(const Vec3 eye, const Hit& hit, const Material& material, const Light& light) {
    auto eyedir = glm::normalize(eye - hit.point);

    auto color = ColorA(0, 0, 0, 1);
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

ColorA colorOf(const Vec3 eye, const Hittable& object, const Hit& hit, const Scene& scene) {
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
            isPoint ? glm::distance(Vec3(source.position), shadowRay.eye)
                    : std::numeric_limits<Float>::max();
        bool shadowed = false;
        for (const auto& node : scene.nodes) {
            auto h = node->intersect(shadowRay);
            if (h && h->t < distance) {
                shadowed = true;
                break;
            }
        }
        if (shadowed) { continue; }
        Float attenuation = 1.0;

        if (isPoint) {
            Float distance = glm::distance(Vec3{source.position}, hit.point);
            attenuation = scene.attenuation.factor(distance);
        }

        color += attenuation*light(eye, hit, object.material, source);
    }
    return color;
}

}  // namespace raytracer
