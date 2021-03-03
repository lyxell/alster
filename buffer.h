#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdlib.h>

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
using buffer_lines = std::vector<std::shared_ptr<buffer_line>>;

struct buffer {
    buffer_lines lines;
    buffer_position pos;
};

struct buf {
    size_t num_lines;
    const buffer_char* lines[];
};
