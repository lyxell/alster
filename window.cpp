#include <sys/ioctl.h>
#include "utf8.h"
#include <assert.h>
#include "window.h"
#include "tokenize.h"

window window_update_scroll(const buffer& b, window w) {
    const auto& [lines, pos] = b;
    if (pos.y < w.scroll) {
        w.scroll = pos.y;
    } else if (pos.y >= w.scroll + w.height) {
        w.scroll = pos.y - w.height + 1;
    }
    return w;
}

void window_render(const buffer& buf, const window& w) {
    const auto& [lines, pos] = buf;
//    std::u32string string;
//    for (auto line : lines) {
//        string += *line;
//        string += '\n';
//    }
//    }
//    tokenize(string.c_str());
//    if (prev_lines == lines && prev_w == w) return;
//    prev_lines = lines;
//    prev_w = w;
    for (size_t i = 0; i < w.height; i++) {
        if (lines.size() > i + w.scroll) {
            auto line = buffer_get_line(buf, i + w.scroll);
            printf("\033[%ld;%dH%04lx %02ld %s\033[K", (i+1), 1,
                    size_t(lines[i+w.scroll].get()) % 0xffff,
                    lines[i+w.scroll].use_count(),
                    utf8_encode(line).c_str());
        }
    }
}

void window_render_cursor(const buffer& buf, const window& w) {
    const auto& [lines, pos] = buf;
    printf("\033[%ld;%ldH", pos.y - w.scroll + 1,
            std::min(pos.x, buffer_get_line(buf, pos.y).size()) + 9);
}

window window_update_size(window w) {
    struct winsize ws;
    assert(ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0);
    w.width = ws.ws_col;
    w.height = ws.ws_row;
    return w;
}

