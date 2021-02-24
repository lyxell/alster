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

buffer buffer_move_left(buffer, size_t);
buffer buffer_move_right(buffer, size_t);
buffer buffer_move_down(buffer, size_t);
buffer buffer_move_up(buffer, size_t);
buffer buffer_insert(buffer, buffer_char);
buffer buffer_erase(buffer);
buffer buffer_break_line(buffer);
buffer buffer_move_start(buffer);
buffer buffer_move_end(buffer);
buffer buffer_move_start_of_line(buffer);
buffer buffer_move_end_of_line(buffer);
buffer buffer_erase_current_line(buffer);
buffer buffer_indent(buffer);
