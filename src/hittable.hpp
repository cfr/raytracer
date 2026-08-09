#pragma once

#include "values.hpp"
#include "material.hpp"
#include "transforms.hpp"
#include "box.hpp"
#include "ray.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <optional>
#include <algorithm>
#include <memory>
#include <utility>

namespace raytracer {

using ManagedObject = std::shared_ptr<const Hittable>;

struct Hittable {
    static constexpr Float step = 0.0001;  // min distance for intersection

    std::shared_ptr<const Material> material;
    std::shared_ptr<const Transforms> transforms;  // nullptr = identity

    // TODO: Wächter & Binder's integer-arithmetic offset
    static Float eps(Vec3 p) {
        return step * std::max({Float(1), glm::abs(p.x), glm::abs(p.y), glm::abs(p.z)});
    }

    explicit Hittable(std::shared_ptr<const Material> m, std::shared_ptr<const Transforms> xf = nullptr)
        : material{std::move(m)}, transforms{std::move(xf)} {}
    virtual ~Hittable() = default;

    virtual Box aabb() const = 0;

    // area-measure pdf of uniform surface sampling, 0 if not samplable
    virtual Float pdfArea() const { return 0; }

    // local normal at point
    virtual Vec4 normal(Vec3 point) const = 0;

    // local-space ray parameter to object, 0 if no intersection
    virtual Float tlocal(Ray ray) const = 0;

    // world-space distance to object for unit ray, affine transform, 0 if none
    Float tworld(const Ray& ray) const {
        auto t = transforms ? tlocal(ray.transformed(transforms->inv)) : tlocal(ray);
        return t < step ? 0 : t;
    }

    // full hit record for a ray known to hit at world distance tw
    Hit makeHit(const Ray& ray, Float tw) const {
        auto wpoint = ray.at(tw);
        auto lpoint = transforms ? transformVec3(transforms->inv, wpoint) : wpoint;
        auto wnormal = transforms ? Vec3{transforms->invT * normal(lpoint)}
                                  : Vec3{normal(lpoint)};
        wnormal = glm::normalize(wnormal);
        bool front = glm::dot(ray.dir, wnormal) < 0;
        wnormal = front ? wnormal : -wnormal;
        return Hit{-ray.dir, wpoint, wnormal, tw, this, front};
    }

    std::optional<Hit> intersect(const Ray& ray) const {
        auto tw = tworld(ray);
        if (tw <= 0) { return {}; }
        return makeHit(ray, tw);
    }
};

}  // namespace raytracer
