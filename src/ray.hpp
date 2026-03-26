#pragma once

#include "image.hpp"
#include "camera.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace raytracer {

using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;

struct Ray {
    vec3 eye = {0, 0, 0};
    vec3 dir = {0, 0, -1};

    vec3 at(float t) {
        return eye + t*dir;
    }

    Ray transformed(mat4 transform) {
        auto tEye = transform * vec4(eye, 1);
        auto tDir = transform * vec4(dir, 0);
        return Ray{vec3{tEye/tEye.w}, vec3{tDir}};
    }
};

class RayCaster {

    float hwidth_;  // width/2
    float hheight_; // height/2

    float thfovy_; // tan(fovy/2)
    float thfovx_; // tan(fovx/2)

    vec3 eye_;
    Basis basis_;

public:

    RayCaster(Camera cam, Image::Size size) : eye_{cam.eye}, basis_{cam} {

        hwidth_ = static_cast<float>(size.width) / 2;
        hheight_ = static_cast<float>(size.height) / 2;

        auto aspect = hwidth_/hheight_;
        auto fovy = glm::radians(cam.fovy);
        thfovy_ = glm::tan(fovy / 2);
        thfovx_ = aspect * thfovy_;
    }

    Ray cast(Image::Point pixel) {

        float x = static_cast<float>(pixel.x) + 0.5;
        float y = static_cast<float>(pixel.y) + 0.5;
        auto alpha = thfovx_ * (x - hwidth_) / hwidth_;
        auto beta = thfovy_ * (hheight_ - y) / hheight_;

        auto dir = alpha * basis_.u + beta * basis_.v - basis_.w;

        return {eye_, glm::normalize(dir)};
    }
};

}
