#pragma once

#include <stdexcept>
#include <charconv>
#include <format>
#include <string>
#include <string_view>

namespace raytracer::parser {

class ParseException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

template <typename T>
T parseNum(std::string_view sv) {
    T value;
    if (!sv.empty() && sv[0] == '+') { sv = sv.substr(1); } // skip leading +
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

    if (ec != std::errc{}) {
        throw ParseException(std::format("Invalid number format: '{}'", sv));
    }
    if (ptr != sv.data() + sv.size()) {
        throw ParseException(std::format("Trailing characters in number: '{}'", sv));
    }
    return value;
}

}
