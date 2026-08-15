#pragma once

#include "values.hpp"

#include <cstddef>
#include <compare>
#include <iterator>
#include <vector>

namespace raytracer {

class Frame {
    Size size_;

 public:
    explicit Frame(Size size) : size_(size) {}

    class Iterator {
        size_t index_, width_;

     public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = Point;
        using difference_type   = std::ptrdiff_t;
        using reference         = Point;

        Iterator(size_t idx, size_t w) : index_(idx), width_(w) {}

        Point operator*() const {
            return {index_ % width_, index_ / width_};
        }

        Iterator& operator++() {
            ++index_;
            return *this;
        }

        auto operator<=>(const Iterator& other) const = default;
    };

    Iterator begin() const { return {0, size_.width}; }
    Iterator end() const { return {size_.width * size_.height, size_.width}; }
};


class Row {
    size_t y_;
    Size size_;
    std::vector<Color> data_;

 public:
    size_t y() const { return y_;}
    Row(size_t y, Size size) : y_(y), size_(size), data_{size.width, Color{0}} {}

    Frame::Iterator begin() const { return {y_ * size_.width, size_.width}; }
    Frame::Iterator end() const { return {y_ * size_.width + size_.width, size_.width}; }

    void set(Point pt, Color color) {
        data_[pt.x] = color;
    }

    Color get(Point pt) const {
        return data_[pt.x];
    }
};

}  // namespace raytracer
