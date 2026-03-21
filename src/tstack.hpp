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
    std::stack<mat4> stack;

    void rmultiply(const mat4& m) {
        mat4& t = stack.top();
        t = t * m;
    }

public:

    TStack() {
        stack.push(mat4(1));
    }

    mat4 top() {
        return stack.top();
    }

    void push() {
        stack.push(stack.top());
    }

    void pop() {
        if (stack.size() <= 1) {
            throw std::length_error("Can't pop transformation stack last item");
            return;
        }
        stack.pop();
    }

    vec3 transform(vec3 v) {
        auto v4 = vec4(v, 1);
        auto t = stack.top() * v4;
        auto v3 = v4 / v4.w;
        return {v4 / v4.w};
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
