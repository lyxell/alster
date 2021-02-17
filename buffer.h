#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdlib.h>

struct coord {
    size_t x;
    size_t y;
    bool operator<(const coord& other) const {
        return std::pair(x, y) < std::pair(other.x, other.y);
    }
    bool operator==(const coord& other) const {
        return std::pair(x, y) == std::pair(other.x, other.y);
    }
    bool operator!=(const coord& other) const {
        return std::pair(x, y) != std::pair(other.x, other.y);
    }
};

using buffer_char = char32_t;
using buffer_line = std::u32string;
using buffer = std::pair<std::vector<std::shared_ptr<buffer_line>>,coord>;

buffer buffer_move_left(buffer, size_t);
buffer buffer_move_right(buffer, size_t);
buffer buffer_move_down(buffer, size_t);
buffer buffer_move_up(buffer, size_t);
buffer buffer_insert(buffer, buffer_char, size_t);
buffer buffer_erase(buffer);
buffer buffer_break_line(buffer);
buffer buffer_move_start(buffer);
buffer buffer_move_end(buffer);
buffer buffer_move_start_of_line(buffer);
buffer buffer_move_end_of_line(buffer);
buffer buffer_erase_current_line(buffer);
buffer_line buffer_get_line(const buffer&, size_t);
buffer_line buffer_get_current_line(const buffer&);

