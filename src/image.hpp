#pragma once

#include "values.hpp"

#include <glm/common.hpp>

#include <array>
#include <cmath>
#include <compare>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace aktis {

class Image {
    Size size_;
    std::vector<Color> data_;

  public:
    using iterator = typename std::vector<Color>::iterator;
    using const_iterator = typename std::vector<Color>::const_iterator;

    iterator begin() {
        return data_.begin();
    }
    [[nodiscard]] const_iterator begin() const {
        return data_.begin();
    }
    [[nodiscard]] const_iterator cbegin() const {
        return data_.cbegin();
    }

    iterator end() {
        return data_.end();
    }
    [[nodiscard]] const_iterator end() const {
        return data_.end();
    }
    [[nodiscard]] const_iterator cend() const {
        return data_.cend();
    }

    [[nodiscard]] Point point(const_iterator it) const {
        auto d = std::distance(begin(), it);
        auto x = d % size_.width;
        auto y = d / size_.width;
        return {x, y};
    }
    [[nodiscard]] Point point(iterator it) const {
        return point(static_cast<const_iterator>(it));
    }

    explicit Image(Size s) : size_{s}, data_{s.width * s.height, Color{0}} {
        if (s.width == 0 || s.height == 0) {
            throw std::runtime_error("Image dimensions must be > 0");
        }
    }

    Image(size_t width, size_t height) : Image(Size{.width = width, .height = height}) {}

    [[nodiscard]] Size size() const {
        return size_;
    }

    [[nodiscard]] Color get(Point pt) const {
        return data_[(size_.width * pt.y) + pt.x];
    }

    void set(Point pt, Color color) {
        data_[(size_.width * pt.y) + pt.x] = color;
    }

    bool writePPM(std::ostream& out, bool ascii = false) const {
        out << (ascii ? "P3\n" : "P6\n") << size_.width << ' ' << size_.height << "\n255\n";
        static constexpr Float scale = 255.0;
        for (auto pix : *this) {
            auto r = std::lround(scale * glm::clamp(pix.r, Float(0), Float(1)));
            auto g = std::lround(scale * glm::clamp(pix.g, Float(0), Float(1)));
            auto b = std::lround(scale * glm::clamp(pix.b, Float(0), Float(1)));
            if (ascii) {
                out << r << ' ' << g << ' ' << b << '\n';
            } else {
                std::array<unsigned char, 3> const rgb = {
                    static_cast<unsigned char>(r),
                    static_cast<unsigned char>(g),
                    static_cast<unsigned char>(b),
                };
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) -- binary PPM output
                out.write(reinterpret_cast<char const*>(rgb.data()), rgb.size());
            }
        }
        return out.good();
    }
};

}  // namespace aktis
