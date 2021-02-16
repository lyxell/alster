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

using buffer_char = char;
using buffer_line = std::vector<buffer_char>;
using buffer = std::pair<std::vector<std::shared_ptr<buffer_line>>,coord>;


buffer
buffer_move_left(const buffer&, size_t);

buffer
buffer_move_right(const buffer&, size_t);

buffer
buffer_move_down(const buffer&, size_t);

buffer
buffer_move_up(const buffer&, size_t);

buffer
buffer_insert(const buffer&, buffer_char, size_t);

buffer
buffer_erase(const buffer&);

std::string
buffer_to_string(const buffer&);

buffer
buffer_break_line(const buffer&);

buffer_line
buffer_get_line(const buffer&, size_t);

buffer
buffer_move_start(const buffer&);

buffer
buffer_move_end(const buffer&);

buffer
buffer_move_start_of_line(const buffer&);

buffer
buffer_move_end_of_line(const buffer&);

buffer_line
buffer_get_current_line(const buffer&);

buffer
buffer_erase_current_line(const buffer&);

