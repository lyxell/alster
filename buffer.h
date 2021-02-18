#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdlib.h>

struct buffer_position {
    size_t x;
    size_t y;
    bool operator<(const buffer_position& other) const {
        return std::pair(x, y) < std::pair(other.x, other.y);
    }
    bool operator==(const buffer_position& other) const {
        return std::pair(x, y) == std::pair(other.x, other.y);
    }
    bool operator!=(const buffer_position& other) const {
        return std::pair(x, y) != std::pair(other.x, other.y);
    }
};

using buffer_char = char32_t;
using buffer_line = std::u32string;
using buffer_lines = std::vector<std::shared_ptr<buffer_line>>;
using buffer = std::pair<buffer_lines, buffer_position>;

buffer buffer_move_left(buffer_lines, buffer_position, size_t);
buffer buffer_move_right(buffer_lines, buffer_position, size_t);
buffer buffer_move_down(buffer_lines, buffer_position, size_t);
buffer buffer_move_up(buffer_lines, buffer_position, size_t);
buffer buffer_insert(buffer_lines, buffer_position, buffer_char, size_t);
buffer buffer_erase(buffer_lines, buffer_position);
buffer buffer_break_line(buffer_lines, buffer_position);
buffer buffer_move_start(buffer_lines, buffer_position);
buffer buffer_move_end(buffer_lines, buffer_position);
buffer buffer_move_start_of_line(buffer_lines, buffer_position);
buffer buffer_move_end_of_line(buffer_lines, buffer_position);
buffer buffer_erase_current_line(buffer_lines, buffer_position);
