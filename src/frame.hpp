#pragma once

#include "values.hpp"

namespace raytracer {

class Frame {

    Size size_;
public:
    Frame(Size size) : size_(size) {}

    class Iterator {
        size_t index_, width_;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = Point;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const Point*;
        using reference         = Point;

        Iterator(size_t idx, size_t w) : index_(idx), width_(w) {}

        Point operator*() const {
            return {index_ % width_, index_ / width_};
        }

        Iterator& operator++() {
            ++index_;
            return *this;
        }

        bool operator<=>(const Iterator& other) const = default;
    };

    Iterator begin() const { return {0, size_.width}; }
    Iterator end() const { return {size_.width * size_.height, size_.width}; }
};

}
