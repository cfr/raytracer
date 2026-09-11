#pragma once

#include "bvh.hpp"
#include "integrator.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "shapes.hpp"
#include "tolerance.hpp"
#include "transforms.hpp"
#include "values.hpp"

#include <glm/common.hpp>

#include <cassert>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace aktis {

struct Attenuation {
    Float constant = 1;
    Float linear = 0;
    Float quadratic = 0;

    [[nodiscard]] Float factor(Float distance) const {
        Float const d = glm::max(distance, tol::tmin);
        return 1 / (constant + d * linear + d * d * quadratic);
    }
};

struct Light {
    Vec4 position = {0, 0, 0, 0};
    Color color = colors::black;

    [[nodiscard]] bool point() const {
        return position.w > 0;  // not directional light
    }
};

struct Settings {
    Size size = {.width = 640, .height = 480};
    size_t threads = 0;
    Float gamma = 1.0;
    Integrator integrator;
    std::string output = "out.ppm";
    std::optional<Seed> seed;  // {} = use random device, otherwise per-row seed + y
};

static_assert(std::is_same_v<PrimId, ShapeId>);

struct Scene {
    std::vector<Material> materials;
    std::vector<Transforms> transforms;
    std::vector<Shape> shapes;  // in BVH leaf order after build()
    Attenuation attenuation;
    std::vector<Light> lights;
    std::vector<ShapeId> quadLights;
    BoundingVolumeHierarchy bvh;

    void build() {
        std::vector<Box> boxes;
        boxes.reserve(shapes.size());
        for (Shape const& shape : shapes) {
            boxes.push_back(shape.aabb(shape.transform(transforms)));
        }
        bvh = BoundingVolumeHierarchy{boxes};
        assert(bvh.size() == shapes.size());

        // validate before reorderShapes(), which indexes slotOf[] by these ids
        for (ShapeId const id : quadLights) {
            if (id >= shapes.size() || shapes[id].type() != Shape::Type::Quad) {
                throw std::logic_error("quadLights entries must be Quad shapes");
            }
        }

        reorderShapes();
    }

    [[nodiscard]] std::optional<Hit> intersect(Ray const& ray) const {
        assert(bvh.size() == shapes.size());
        // tie-break on the original id (lower wins), so hits don't depend on tree shape
        struct Candidate {
            Float t = inf;
            ShapeId original = 0;
            ShapeId slot = noShape;
        };
        Candidate best;
        std::vector<ShapeId> const& originalOf = bvh.order();

        bvh.closest(ray, best.t, [&](ShapeId slot) {
            Shape const& shape = shapes[slot];
            Float const t = shape.tworld(ray, shape.transform(transforms));
            if (t > 0 && std::pair{t, originalOf[slot]} < std::pair{best.t, best.original}) {
                best = {t, originalOf[slot], slot};
            }
            return false;
        });

        if (best.slot == noShape) {
            return {};
        }
        Shape const& shape = shapes[best.slot];
        return shape.makeHit(best.slot, ray, best.t, shape.transform(transforms));
    }

    [[nodiscard]] bool occluded(Ray const& ray, Float tmax = inf, ShapeId ignore = noShape) const {
        assert(bvh.size() == shapes.size());
        bool hit = false;
        bvh.any(ray, tmax, [&](ShapeId slot) {
            if (slot == ignore) {
                return false;
            }
            Shape const& shape = shapes[slot];
            Float const t = shape.tworld(ray, shape.transform(transforms));
            hit = t > 0 && t <= tmax;
            return hit;
        });
        return hit;
    }

  private:
    // moves shapes into the BVH's leaf order; quadLights follow
    void reorderShapes() {
        std::vector<ShapeId> const& originalOf = bvh.order();
        std::vector<Shape> sorted(originalOf.size());
        std::vector<ShapeId> slotOf(originalOf.size());
        for (size_t slot = 0; slot < originalOf.size(); ++slot) {
            ShapeId const original = originalOf[slot];
            sorted[slot] = shapes[original];
            slotOf[original] = static_cast<ShapeId>(slot);
        }
        shapes = std::move(sorted);
        for (ShapeId& id : quadLights) {
            id = slotOf[id];
        }
    }
};

[[nodiscard]] inline Material const& materialOf(Scene const& scene, Shape const& shape) {
    assert(shape.materialId() < scene.materials.size());
    return scene.materials[shape.materialId()];
}

[[nodiscard]] inline Material const& materialOf(Scene const& scene, Hit const& hit) {
    assert(hit.materialId < scene.materials.size());
    return scene.materials[hit.materialId];
}

}  // namespace aktis
