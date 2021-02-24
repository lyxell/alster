#pragma once
#include "buffer.h"
#include <optional>
#include <vector>

struct window_cell {
    char32_t ch;
    int fg;
    int bg;
    bool operator==(window_cell rhs) const {
        return ch == rhs.ch && fg == rhs.fg && bg == rhs.bg;
    }
};

struct window {
    size_t width;
    size_t height;
    struct {
        size_t x;
        size_t y;
    } cursor;
    std::vector<std::vector<window_cell>> matrix;
    bool operator==(window rhs) {
        return std::tuple(width, height, matrix)
            == std::tuple(rhs.width, rhs.height, rhs.matrix);
    }
};

window window_update_size(window);
window window_render_buffer(window, const buffer&, size_t scroll);
window window_render_visual_selection(window w, buffer_position start,
                                    buffer_position end, size_t scroll);
window window_update_cursor(window, const buffer& buf, size_t scroll);
std::string window_to_string(window);
