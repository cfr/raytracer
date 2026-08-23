#pragma once

#include "values.hpp"

#include <compare>
#include <cstddef>
#include <iterator>
#include <vector>

namespace aktis {

class Frame {
    Size size_;

  public:
    explicit Frame(Size size) : size_(size) {}

    class Iterator {
        size_t index_, width_;

      public:
        using iterator_category = std::input_iterator_tag;
        using value_type = Point;
        using difference_type = std::ptrdiff_t;
        using reference = Point;

        Iterator(size_t idx, size_t w) : index_(idx), width_(w) {}

        Point operator*() const {
            return {index_ % width_, index_ / width_};
        }

        Iterator& operator++() {
            ++index_;
            return *this;
        }

        auto operator<=>(Iterator const& other) const = default;
    };

    [[nodiscard]] Iterator begin() const {
        return {0, size_.width};
    }
    [[nodiscard]] Iterator end() const {
        return {size_.width * size_.height, size_.width};
    }
};

class Row {
    size_t y_;
    Size size_;
    std::vector<Color> data_;

  public:
    [[nodiscard]] size_t y() const {
        return y_;
    }
    Row(size_t y, Size size) : y_(y), size_(size), data_{size.width, Color{0}} {}

    [[nodiscard]] Frame::Iterator begin() const {
        return {y_ * size_.width, size_.width};
    }
    [[nodiscard]] Frame::Iterator end() const {
        return {(y_ * size_.width) + size_.width, size_.width};
    }

    void set(Point pt, Color color) {
        data_[pt.x] = color;
    }

    [[nodiscard]] Color get(Point pt) const {
        return data_[pt.x];
    }
};

}  // namespace aktis
