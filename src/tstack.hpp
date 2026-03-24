#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stack>
#include <stdexcept>

namespace raytracer {

using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;

class TStack {
    std::stack<mat4> stack_;

    void rmultiply(const mat4& m) {
        mat4& t = stack_.top();
        t = t * m;
    }

public:

    TStack() {
        stack_.push(mat4{1});
    }

    mat4 top() {
        return stack_.top();
    }

    void push() {
        stack_.push(stack_.top());
    }

    void pop() {
        if (stack_.size() <= 1) {
            throw std::length_error("Can't pop transformation stack last item");
            return;
        }
        stack_.pop();
    }

    void rotate(vec3 axis, float angle) {
        auto r = glm::rotate(mat4(1), angle, axis);
        rmultiply(r);
    }

    void translate(vec3 t) {
        auto tr = glm::translate(mat4(1), t);
        rmultiply(tr);
    }

    void scale(vec3 s) {
        auto sc = glm::scale(mat4(1), s);
        rmultiply(sc);
    }
};

}

