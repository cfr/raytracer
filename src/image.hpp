#pragma once

#include "values.hpp"

#include <iterator>
#include <compare>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <vector>

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
        : size_{s}, data_{s.width*s.height, Color{0}} {}

    Image(size_t width, size_t height)
        : Image(Size{width, height}) {}

    Size size() const { return size_; }

    Color get(Point pt) const {
        return data_[size_.width * pt.y + pt.x];
    }

    void set(Point pt, Color color) {
        data_[size_.width * pt.y + pt.x] = color;
    }

    void writePPM(std::ostream& out, bool ascii = false) const {
        out << (ascii ? "P3\n" : "P6\n") << size_.width << ' ' << size_.height << "\n255\n";
        static constexpr Float scale = 255.0;
        for (auto pix : *this) {
            auto r = std::lround(scale * glm::clamp(pix.r, Float(0), Float(1)));
            auto g = std::lround(scale * glm::clamp(pix.g, Float(0), Float(1)));
            auto b = std::lround(scale * glm::clamp(pix.b, Float(0), Float(1)));
            if (ascii) {
                out << r << ' ' << g << ' ' << b << '\n';
            } else {
                unsigned char rgb[3] = {
                    static_cast<unsigned char>(r),
                    static_cast<unsigned char>(g),
                    static_cast<unsigned char>(b),
                };
                out.write(reinterpret_cast<const char*>(rgb), 3);
            }
        }
    }
};

}  // namespace raytracer
