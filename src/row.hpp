#pragma once

#include "values.hpp"
#include "image.hpp"

#include <new>

namespace raytracer {

class alignas(std::hardware_destructive_interference_size) Row {

    size_t y_;
    Size size_;
    std::vector<Color> data_;

public:

    size_t y() { return y_;}
    Row(size_t y, Size size) : y_(y), size_(size), data_{size.width, {0, 0, 0}} {}

    Frame::Iterator begin() const { return {y_ * size_.width, size_.width}; }
    Frame::Iterator end() const { return {y_ * size_.width + size_.width, size_.width}; }

    void set(Point pt, Color color) {
        //data_[size_.width * (pt.y - y_) + pt.x] = color;
        data_[pt.x] = color;
    }

    Color get(Point pt) {
        //return data_[size_.width * (pt.y - y_) + pt.x];
        return data_[pt.x];
    }
};

}
