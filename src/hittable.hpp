#pragma once

#include "box.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "tolerance.hpp"
#include "transforms.hpp"
#include "values.hpp"

#include <glm/geometric.hpp>

#include <memory>
#include <optional>
#include <utility>

namespace raytracer {

using ObjectPtr = std::shared_ptr<Hittable const>;

struct Hittable {
    MaterialPtr material;
    TransformsPtr transforms;  // nullptr = identity

    explicit Hittable(MaterialPtr m, TransformsPtr xf = nullptr)
        : material{std::move(m)}, transforms{std::move(xf)} {}
    virtual ~Hittable() = default;

    [[nodiscard]] virtual Box aabb() const = 0;

    // area-measure pdf of uniform surface sampling, 0 if not samplable
    [[nodiscard]] virtual Float pdfArea() const {
        return 0;
    }

    // local normal at point
    [[nodiscard]] virtual Vec4 normal(Vec3 point) const = 0;

    // local-space ray parameter to object, 0 if no intersection
    [[nodiscard]] virtual Float tlocal(Ray ray) const = 0;

    // world-space distance to object for unit ray, affine transform, 0 if none
    [[nodiscard]] Float tworld(Ray const& ray) const {
        auto t = transforms ? tlocal(ray.transformed(transforms->inv)) : tlocal(ray);
        return t < tol::tmin ? 0 : t;
    }

    // full hit record for a ray known to hit at world distance tw
    [[nodiscard]] Hit makeHit(Ray const& ray, Float tw) const {
        auto wpoint = ray.at(tw);
        auto lpoint = transforms ? transformPoint(transforms->inv, wpoint) : wpoint;
        auto wnormal = transforms ? Vec3{transforms->invT * normal(lpoint)} : Vec3{normal(lpoint)};
        wnormal = glm::normalize(wnormal);
        bool const front = glm::dot(ray.dir, wnormal) < 0;
        wnormal = front ? wnormal : -wnormal;
        return Hit{.wo = -ray.dir,
                   .point = wpoint,
                   .normal = wnormal,
                   .t = tw,
                   .object = this,
                   .front = front};
    }

    [[nodiscard]] std::optional<Hit> intersect(Ray const& ray) const {
        auto tw = tworld(ray);
        if (tw <= 0) {
            return {};
        }
        return makeHit(ray, tw);
    }

  protected:
    Hittable(Hittable const&) = default;
    Hittable& operator=(Hittable const&) = default;
    Hittable(Hittable&&) = default;
    Hittable& operator=(Hittable&&) = default;
};

}  // namespace raytracer
