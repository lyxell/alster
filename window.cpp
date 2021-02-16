#include <sys/ioctl.h>
#include <assert.h>
#include "window.h"

window window_update_scroll(const buffer& b, window w) {
    const auto& [lines, pos] = b;
    if (pos.y < w.scroll) {
        w.scroll = pos.y;
    } else if (pos.y >= w.scroll + w.height) {
        w.scroll = pos.y - w.height + 1;
    }
    return w;
}

void
window_render(const buffer& buf, const window& w) {
    static std::vector<std::shared_ptr<buffer_line>> prev_lines;
    static window prev_w;
    const auto& [lines, pos] = buf;
    if (prev_lines == lines && prev_w == w) return;
    prev_lines = lines;
    prev_w = w;
    for (size_t i = 0; i < w.height; i++) {
        if (lines.size() > i + w.scroll) {
            auto line = buffer_get_line(buf, i + w.scroll);
            printf("\033[%ld;%dH%lx \033[K", (i+1), 1,
                    (size_t) lines[i+w.scroll].get());
            for (int j = 0; j < int(std::min(w.width - 13, line.size())); j++) {
                putchar(line[j]);
            }
        }
    }
}

void window_render_cursor(const buffer& buf, const window& w) {
    auto [lines, pos] = buf;
    printf("\033[%ld;%ldH", pos.y - w.scroll + 1,
            std::min(pos.x, buffer_get_line(buf, pos.y).size()) + 14);
}
window window_update_size(window w) {
    struct winsize ws;
    assert(ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0);
    w.width = ws.ws_col;
    w.height = ws.ws_row;
    return w;
}
