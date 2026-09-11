#pragma once

#include "box.hpp"
#include "ray.hpp"
#include "shape/quad.hpp"
#include "shape/quadric.hpp"
#include "shape/sphere.hpp"
#include "shape/triangle.hpp"
#include "tolerance.hpp"
#include "transforms.hpp"
#include "values.hpp"

#include <glm/geometric.hpp>

#include <cassert>
#include <cstdint>
#include <span>
#include <utility>

namespace aktis {

class Shape final {
  public:
    enum class Type : std::uint8_t { Sphere, Triangle, Quad, Quadric };

    [[nodiscard]] static Shape sphere(MaterialId m, Vec3 center, Float radius,
                                      TransformId xf = noTransform) {
        Shape s;
        s.type_ = Type::Sphere;
        s.materialId_ = m;
        s.transformId_ = xf;
        s.data_.sphere = Sphere{center, radius};
        return s;
    }

    [[nodiscard]] static Shape triangle(MaterialId m, Vec3 a, Vec3 b, Vec3 c) {
        Shape s;
        s.type_ = Type::Triangle;
        s.materialId_ = m;
        s.data_.triangle = Triangle::of(a, b, c);
        return s;
    }

    [[nodiscard]] static Shape quad(MaterialId m, Vec3 v0, Vec3 edge1, Vec3 edge2) {
        Shape s;
        s.type_ = Type::Quad;
        s.materialId_ = m;
        s.data_.quad = Quad::of(v0, edge1, edge2);
        return s;
    }

    [[nodiscard]] static Shape quadric(MaterialId m, TransformId xf, Quadric quadric) {
        Shape s;
        s.type_ = Type::Quadric;
        s.materialId_ = m;
        s.transformId_ = xf;
        s.data_.quadric = quadric;
        return s;
    }

  private:
    union Data {
        Sphere sphere{{0, 0, 0}, 1};
        Triangle triangle;
        Quad quad;
        Quadric quadric;
    };

    Type type_ = Type::Sphere;
    MaterialId materialId_ = 0;
    TransformId transformId_ = noTransform;
    Data data_;

    template <class F> [[nodiscard]] static auto dispatch(Shape const& s, F const& f) {
        switch (s.type_) {
        case Type::Sphere:
            return f(s.data_.sphere);
        case Type::Triangle:
            return f(s.data_.triangle);
        case Type::Quad:
            return f(s.data_.quad);
        case Type::Quadric:
            return f(s.data_.quadric);
        }
        std::unreachable();
    }

  public:
    [[nodiscard]] Type type() const {
        return type_;
    }

    [[nodiscard]] MaterialId materialId() const {
        return materialId_;
    }

    // transform, nullptr if untransformed
    [[nodiscard]] Transforms const* transform(std::span<Transforms const> xfs) const {
        return transformId_ == noTransform ? nullptr : &xfs[transformId_];
    }

    [[nodiscard]] Box aabb(Transforms const* xf = nullptr) const {
        Box const local = dispatch(*this, [](auto const& g) { return g.aabb(); });
        return xf != nullptr ? local.transformed(*xf) : local;
    }

    [[nodiscard]] Vec4 normal(Vec3 point) const {
        return dispatch(*this, [point](auto const& g) { return g.normal(point); });
    }

    [[nodiscard]] Float tlocal(Ray ray) const {
        return dispatch(*this, [&ray](auto const& g) { return g.tlocal(ray); });
    }

    // area-measure pdf of uniform surface sampling, 0 if not samplable
    [[nodiscard]] Float pdfArea() const {
        return dispatch(*this, [](auto const& g) { return g.pdfArea(); });
    }

    // world-space distance to object for unit ray, affine transform, 0 if none
    [[nodiscard]] Float tworld(Ray const& ray, Transforms const* xf) const {
        auto t = xf != nullptr ? tlocal(ray.transformed(xf->inv)) : tlocal(ray);
        return t < tol::tmin || t == inf ? 0 : t;
    }

    // full hit record for a ray known to hit at world distance tw
    [[nodiscard]] Hit makeHit(ShapeId id, Ray const& ray, Float tw, Transforms const* xf) const {
        auto wpoint = ray.at(tw);
        auto lpoint = xf != nullptr ? transformPoint(xf->inv, wpoint) : wpoint;
        auto wnormal = xf != nullptr ? Vec3{xf->invT * normal(lpoint)} : Vec3{normal(lpoint)};
        wnormal = glm::normalize(wnormal);
        bool const front = glm::dot(ray.dir, wnormal) < 0;
        wnormal = front ? wnormal : -wnormal;
        return Hit{.wo = -ray.dir,
                   .point = wpoint,
                   .normal = wnormal,
                   .t = tw,
                   .shapeId = id,
                   .materialId = materialId_,
                   .front = front};
    }

    [[nodiscard]] Quad const& quad() const {
        assert(type_ == Type::Quad);
        return data_.quad;
    }
};

}  // namespace aktis
