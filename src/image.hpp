#pragma once

#include "values.hpp"

#include <iterator>
#include <compare>
#include <cstdint>
#include <ostream>

namespace raytracer {

class Image {

    Size size_;
    std::vector<Color> data_;

public:
    using iterator = typename std::vector<Color>::iterator;
    using const_iterator = typename std::vector<Color>::const_iterator;

    iterator begin() { return data_.begin(); }
    const_iterator begin() const { return data_.begin(); }
    const_iterator cbegin() const { return data_.cbegin(); }

    iterator end() { return data_.end(); }
    const_iterator end() const { return data_.end(); }
    const_iterator cend() const { return data_.cend(); }

    Point point(const_iterator it) const {
        auto d = std::distance(begin(), it);
        auto x = d % size_.width;
        auto y = d / size_.width;
        return {x, y};
    }
    Point point(iterator it) const {
        return point(static_cast<const_iterator>(it));
    }

    explicit Image(Size s)
        : size_{s}, data_{s.width*s.height, {0, 0, 0}} {}

    Image(size_t width, size_t height)
        : Image(Size{width, height}) {}

    Size size() const { return size_; }

    Color get(Point pt) const {
        return data_[size_.width * pt.y + pt.x];
    }

    void set(Point pt, Color color) {
        data_[size_.width * pt.y + pt.x] = color;
    }

    void writePPM(std::ostream& out) const {
        out << "P3\n" << size_.width << ' ' << size_.height << "\n255\n";
        for(auto pix: *this) {
            auto r = static_cast<int>(255.999 * pix.r);
            auto g = static_cast<int>(255.999 * pix.g);
            auto b = static_cast<int>(255.999 * pix.b);
            out << r << ' ' << g << ' ' << b << '\n';
        }
        out << std::endl;
    }
};

} // namespace raytracer
