#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdlib.h>

using buffer_char_t = char;
using buffer_line_t = std::vector<buffer_char_t>;

struct buffer_t {
    int x;
    int y;
    std::vector<std::shared_ptr<buffer_line_t>> lines;
    buffer_t() : x(), y(), lines({std::make_shared<buffer_line_t>()}) {}
    buffer_t(const std::string& s) : x(), y(), lines() {
        buffer_line_t curr_line;
        for (const auto c : s) {
            if (c == '\n') {
                lines.push_back(std::make_shared<buffer_line_t>(curr_line));
                curr_line = buffer_line_t();
            } else {
                curr_line.push_back(c);
            }
        }
        lines.push_back(std::make_shared<buffer_line_t>(curr_line));
    }
};

buffer_t
buffer_move_left(const buffer_t&, int);

buffer_t
buffer_move_right(const buffer_t&, int);

buffer_t
buffer_move_down(const buffer_t&, int);

buffer_t
buffer_move_up(const buffer_t&, int);

buffer_t
buffer_insert(const buffer_t& b, buffer_char_t c);

buffer_t
buffer_erase(const buffer_t& b);

std::string
buffer_to_string(const buffer_t& b);

buffer_t
buffer_break_line(const buffer_t& b);

std::string
buffer_get_line(const buffer_t& b, size_t i);
