#pragma once

#include <glm/vec2.hpp>

#include <iterator>
#include <compare>

namespace raytracer {

class Image {
public:
    using Point = glm::ivec2;

    struct Size {
        int width, height;
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

    explicit Image(Size size)
        : size_(size) {}

    Image(int width, int height)
        : size_(width, height) {}

    Iterator begin() const {
        return Iterator(0, 0, size_);
    }

    Iterator end() const {
        return Iterator(0, size_.height, size_);
    }

    Size size() const { return size_; }
private:
    Size size_;
};

} // namespace raytracer
