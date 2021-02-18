#include <sys/ioctl.h>
#include "utf8.h"
#include <assert.h>
#include "window.h"
#include "tokenize.h"
#include "syntax/syntax.h"
#include "colors.h"

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
    /* create windowframe */
    std::vector<buffer_line> windowframe;
    for (size_t i = 0; i < w.height; i++) {
        if (lines.size() > i + w.scroll) {
            windowframe.push_back(*lines[i + w.scroll]);
        }
    }
    /* do render */
    size_t y = 0;
    for (auto line : windowframe) {
        /* clear line */
        printf("\033[%ld;%dH%04lx %02ld\033[K ", (y+1), 1,
                size_t(lines[y+w.scroll].get()) & 0xffff,
                lines[y+w.scroll].use_count());
        /* draw chars in line */
        size_t x = 8;
        for (auto [s, e, t] : tokenize_c(line.c_str())) {
            switch (t) {
                case C_LITERAL_DECIMAL:
                case C_LITERAL_OCTAL:
                    set_color(COLOR_RED);
                    break;
                case C_STRING_CHAR:
                    set_color(COLOR_CYAN);
                    break;
                case C_STRING_ESCAPE_SEQUENCE:
                    set_color(COLOR_GREEN);
                    break;
                case C_STRING_ENCODING_PREFIX:
                case C_STRING_OPENING_QUOTE:
                case C_STRING_CLOSING_QUOTE:
                case C_TYPE:
                    set_color(COLOR_BLUE);
                    break;
                case C_KEYWORD:
                    set_color(COLOR_MAGENTA);
                    break;
                case C_IDENTIFIER:
                    set_color(COLOR_YELLOW);
                    break;
                default:
                    set_color(COLOR_RESET);
                    break;
            }
            auto str = std::u32string(s, e);
            for (auto ch : str) {
                if (x < w.width) {
                    std::u32string output;
                    output += ch;
                    printf(utf8_encode(output).c_str());
                }
                x++;
            }
        }
        set_color(COLOR_RESET);
        y++;
    }
}

void window_render_cursor(const buffer& buf, const window& w) {
    const auto& [lines, pos] = buf;
    printf("\033[%ld;%ldH", pos.y - w.scroll + 1,
            std::min(pos.x, lines[pos.y]->size()) + 9);
}

window window_update_size(window w) {
    struct winsize ws;
    assert(ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0);
    w.width = ws.ws_col;
    w.height = ws.ws_row;
    return w;
}

