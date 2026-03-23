#pragma once

#include "image.hpp"
#include "camera.hpp"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace raytracer {

using vec3 = glm::vec3;

struct Ray {
    vec3 eye = {0, 0, 0};
    vec3 dir = {0, 0, -1};
};

struct Hit {
    float t;
    vec3 point;
    vec3 normal;
};

class RayCaster {

    float thfovx_; // tan(fovx/2)
    float thfovy_; // tan(fovy/2)
    float hwidth_;  // width/2
    float hheight_; // height/2
    vec3 eye_;
    Basis basis_;

public:

    RayCaster(Camera cam, Image::Size size) : eye_{cam.eye}, basis_{cam} {

        auto aspect = hwidth_/hheight_;
        auto fovy = glm::radians(cam.fovy);
        thfovx_ = glm::tan(fovy * aspect / 2);
        thfovy_ = glm::tan(fovy / 2);

        hwidth_ = static_cast<float>(size.width) / 2;
        hheight_ = static_cast<float>(size.height) / 2;
    }

    Ray cast(Image::Point pixel) {

        float x = static_cast<float>(pixel.x) + 0.5;
        float y = static_cast<float>(pixel.y) + 0.5;
        auto alpha = thfovx_ * (x - hwidth_)/hwidth_;
        auto beta = thfovy_ * (y - hheight_)/hheight_;

        auto dir = alpha*basis_.u + beta*basis_.v - basis_.w;

        return {eye_, glm::normalize(dir)};
    }
};

}
