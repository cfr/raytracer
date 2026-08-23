#pragma once

#include "values.hpp"
#include <glm/trigonometric.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <stack>
#include <stdexcept>

namespace aktis {

using Transform = glm::tmat4x4<Float, glm::defaultp>;

constexpr Transform identity = Transform{1};

inline Vec3 transformPoint(Transform m, Vec3 p) {
    auto p4 = Vec4(p, 1);
    auto tp = m * p4;
    return {tp / tp.w};
}

struct Transforms {
    Transform m = identity;
    Transform inv = identity;
    Transform invT = identity;
};

using TransformsPtr = std::shared_ptr<Transforms const>;

class TStack {
    std::stack<Transform> stack_;

    void rmultiply(Transform const& m) {
        auto& t = stack_.top();
        t = t * m;
    }

  public:
    TStack() {
        stack_.push(identity);
    }

    [[nodiscard]] Transform top() const {
        return stack_.top();
    }

    void push() {
        stack_.push(stack_.top());
    }

    void pop() {
        if (stack_.size() <= 1) {
            throw std::length_error("Can't pop transformation stack last item");
        }
        stack_.pop();
    }

    void rotate(Vec3 axis, Float angle) {
        auto r = glm::rotate(Transform(1), glm::radians(angle), axis);
        rmultiply(r);
    }

    void translate(Vec3 t) {
        auto tr = glm::translate(Transform(1), t);
        rmultiply(tr);
    }

    void scale(Vec3 s) {
        auto sc = glm::scale(Transform(1), s);
        rmultiply(sc);
    }
};

}  // namespace aktis
