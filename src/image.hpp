#pragma once

#include "values.hpp"

#include <iterator>
#include <compare>
#include <cstdint>
#include <ostream>

namespace raytracer {

class Image {
public:

    struct Size {
        int width = 0;
        int height = 0;
    };

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Point;
        using difference_type = std::ptrdiff_t;
        using pointer = Point*;
        using reference = Point&;

        Iterator(int x, int y, Size size)
            : x_(x), y_(y), size_(size) {}

        Point operator*() const {
            return {x_, y_};
        }

        Iterator& operator++() {
            ++x_;
            if (x_ >= size_.width) {
                x_ = 0;
                ++y_;
            }
            return *this;
        }

        Iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator<=>(const Iterator& other) const {
            if (auto cmp = y_ <=> other.y_; cmp != 0) {
                return cmp;
            }
            return x_ <=> other.x_;
        }

        bool operator==(const Iterator& other) const {
            return operator<=>(other) == std::strong_ordering::equal;
        }

    private:
        int x_, y_;
        Size size_;
    };

    explicit Image(Size s)
        : size_{s}, data_{static_cast<size_t>(s.width*s.height), {0, 0, 0}} {}

    Image(int width, int height)
        : Image(Size{width, height}) {}

    Iterator begin() const {
        return Iterator(0, 0, size_);
    }

    Iterator end() const {
        return Iterator(0, size_.height, size_);
    }

    Size size() const { return size_; }

    Color get(Point pt) const {
        return data_[size_.width * pt.y + pt.x];
    }

    void set(Point pt, Color color) {
        data_[size_.width * pt.y + pt.x] = color;
    }

    void writePPM(std::ostream& out) const {
        out << "P3\n" << size_.width << ' ' << size_.height << "\n255\n";
        for(auto px: *this) {
            auto rgb = get(px);
            auto r = static_cast<int>(255.999 * rgb.r);
            auto g = static_cast<int>(255.999 * rgb.g);
            auto b = static_cast<int>(255.999 * rgb.b);
            out << r << ' ' << g << ' ' << b << '\n';
        }
        out << std::endl;
    }

private:
    Size size_;
    std::vector<Color> data_;
};

} // namespace raytracer
