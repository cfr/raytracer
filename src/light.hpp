#pragma once

#include "scene.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>
#include <glm/exponential.hpp>

namespace raytracer {

using vec3 = glm::vec3;
using vec4 = glm::vec4;

vec4 light(vec3 eye, const Hit& hit, const Material& material, const Light& light) {

    vec3 eyedir = glm::normalize(eye - hit.point);

    vec4 color = vec4(0, 0, 0, 1);
    vec3 ldir;
    if (light.position.w == 0) {
        ldir = glm::normalize(vec3{light.position});
    } else {
        vec3 lpos = vec3{light.position / light.position.w};
        ldir = glm::normalize(lpos - hit.point);
    }
    vec3 halfvec = glm::normalize(ldir + eyedir);

    float nDotL = glm::dot(hit.normal, ldir);
    vec4 lambert = material.diffuse * light.color * std::max(nDotL, 0.0f);

    float nDotH = glm::dot(hit.normal, halfvec);
    vec4 phong = material.specular * light.color * glm::pow(std::max(nDotH, 0.0f), material.shininess);
    color += lambert + phong;

    return color;
}

vec4 colorOf(const vec3 eye, const Hittable& object, const Hit& hit, const Scene& scene) {

    vec4 color = object.ambient + object.material.emission;

    for(const auto& source: scene.lights) {
        bool isPoint = source.position.w > 0; // point light, not directional
        auto srcDir = vec3(source.position);
        if (isPoint) {
            srcDir = vec3(source.position) - hit.point;
        }
        srcDir = glm::normalize(srcDir);
        float offset = Hittable::step; // to prevent self-intersection
        auto shadowRay = Ray{hit.point + offset*srcDir, srcDir};
        float distance = glm::distance(vec3(source.position), shadowRay.eye);
        bool shadowed = false;
        for (const auto& node: scene.nodes) {
            auto h = node->intersect(shadowRay);
            if (h && (!isPoint || h->t < distance)) {
                shadowed = true;
                break;
            }
        }
        if (shadowed) { continue; };
        float attenuation = 1.0f;

        if (isPoint) {
            float distance = glm::distance(vec3{source.position}, hit.point);
            attenuation = scene.attenuation.factor(distance);
        }

        color += attenuation*light(eye, hit, object.material, source);
    }
    return color;
}

}
