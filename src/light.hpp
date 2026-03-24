#pragma once

#include "scene.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>
#include <glm/exponential.hpp>

namespace raytracer {

using vec3 = glm::vec3;
using vec4 = glm::vec4;

vec4 light(vec3 eye, vec3 pos, vec3 normal, vec4 diff, float shin, vec4 spec, vec4 lposh, vec4 lcolor) {

    //vec3 pos = posh.xyz / posh.w;
    //vec3 eye = vec3(0, 0, 0);
    vec3 eyedir = glm::normalize(eye - pos);
    //vec3 normal = normalize(norm);

    vec4 color = vec4(0, 0, 0, 1);
    vec3 ldir;
    if (lposh.w == 0) {
        ldir = glm::normalize(vec3{lposh});
    } else {
        vec3 lpos = vec3{lposh / lposh.w};
        ldir = glm::normalize(lpos - pos);
    }
    vec3 halfvec = glm::normalize(ldir + eyedir);

    float nDotL = glm::dot(normal, ldir);
    vec4 lambert = diff * lcolor * std::max(nDotL, 0.0f);

    float nDotH = glm::dot(normal, halfvec);
    vec4 phong = spec * lcolor * glm::pow(std::max(nDotH, 0.0f), shin);
    color += lambert + phong;

    return color;
}

vec4 colorOf(const vec3 eye, const Node& object, const Hit& hit, const Scene& scene) {

    vec4 color = vec4{object.ambient + object.material.emission, 1};

    for(const auto& source: scene.lights) {
        auto srcDir = vec3(source.position) - hit.point;
        float offset = 0.001f; // to prevent self-intersection
        auto shadowRay = Ray{hit.point + offset*srcDir, srcDir};
        bool shadowed = false;
        for (const auto& node: scene.nodes) {
            auto h = node->intersect(shadowRay);
            if (h && h->t > Hittable::step && h->t < 1) {
                shadowed = true;
                break;
            }
        }
        if (shadowed) { continue; };
        // TODO: set rgba colors while parsing
        auto diff = vec4{object.material.diffuse, 1};
        auto spec = vec4{object.material.specular, 1};
        auto lcol = vec4{source.rgb, 1};
        float attenuation = 1.0f;
        if (source.position.w > 0) { // point light
            float distance = glm::distance(vec3{source.position}, hit.point);
            attenuation = scene.attenuation.factor(distance);
        }

        color += attenuation*light(eye, hit.point, hit.normal, diff, object.material.shininess, spec, source.position, lcol);
    }
    return color;
}

}
