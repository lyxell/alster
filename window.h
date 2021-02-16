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

void window_render(const buffer& buf, const window& w);
void window_render_cursor(const buffer& buf, const window& w);
window window_update_size(window w);
window window_update_scroll(const buffer& b, window w);
