#include "buffer.h"

struct window {
    size_t scroll;
    size_t width;
    size_t height;
    bool operator==(window rhs) {
        return std::tuple(scroll, width, height)
            == std::tuple(rhs.scroll, rhs.width, rhs.height);
    }
};

void window_render(const buffer&, window);
void window_render_cursor(const buffer&, window);
window window_update_size(window);
window window_update_scroll(const buffer&, window);
