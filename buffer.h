#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdlib>

struct buffer_position {
    size_t x;
    size_t y;
    bool operator<(const buffer_position& other) const {
        return std::pair(y, x) < std::pair(other.y, other.x);
    }
    bool operator>(const buffer_position& other) const {
        return std::pair(y, x) > std::pair(other.y, other.x);
    }
    bool operator>=(const buffer_position& other) const {
        return std::pair(y, x) >= std::pair(other.y, other.x);
    }
    bool operator<=(const buffer_position& other) const {
        return std::pair(y, x) <= std::pair(other.y, other.x);
    }
    bool operator==(const buffer_position& other) const {
        return std::pair(y, x) == std::pair(other.y, other.x);
    }
    bool operator!=(const buffer_position& other) const {
        return std::pair(y, x) != std::pair(other.y, other.x);
    }
};

using buffer_char = char32_t;
using buffer_line = std::u32string;

