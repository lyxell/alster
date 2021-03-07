#include "colors.h"
#include "syntax/syntax.h"
#include "window.h"
#include <assert.h>
#include <string.h>
#include <sys/ioctl.h>

static int token_to_color(int t) {
    switch (t) {
        case C_PUNCTUATOR:
        case C_SINGLE_LINE_COMMENT:
            return COLOR_WHITE;
        case C_LITERAL_DECIMAL:
        case C_LITERAL_OCTAL:
        case C_LITERAL_BOOL:
            return COLOR_RED;
        case C_STRING:
            return COLOR_CYAN;
        case C_TYPE:
            return COLOR_BLUE;
        case C_KEYWORD:
            return COLOR_MAGENTA;
        default:
            break;
    }
    return COLOR_RESET;
}

void window_render_buffer(window* w,
                            const std::vector<std::string>& lines,
                            size_t scroll) {
    memset(w->matrix, 0, sizeof(w->matrix));
    for (size_t y = 0; y < w->height; y++) {
        if (y + scroll >= lines.size()) continue;
        const char* ptr = lines[y + scroll].c_str();
        const char* end = ptr + lines[y + scroll].size();
        size_t x = 0;
        while (ptr < end) {
            struct token t = tokenize_c(ptr);
            if (t.start != t.end) {
                while (t.start < t.end) {
                    w->matrix[y][x].ch = *(t.start);
                    w->matrix[y][x].fg = token_to_color(t.type);
                    t.start++;
                    x++;
                }
            }
            ptr = t.end;
        }
    }
}

void window_to_string(const window *w, char* str) {
    for (size_t y = 0; y < w->height; y++) {
        str += sprintf(str, "\x1b[%ld;1H\x1b[K", y + 1);
        for (size_t x = 0; x < w->width; x++) {
            if (w->matrix[y][x].ch) {
                if (w->matrix[y][x].bg) {
                    str += sprintf(str, "\x1b[%d;%dm", w->matrix[y][x].fg, w->matrix[y][x].bg);
                } else {
                    str += sprintf(str, "\x1b[%dm", w->matrix[y][x].fg);
                }
                char f = (char) w->matrix[y][x].ch;
                str += sprintf(str, "%c", f);
                str += sprintf(str, "%s", "\x1b[0m");
            }
        }
    }
    sprintf(str, "\x1b[%ld;%ldH", w->cursor.y, w->cursor.x);
}

void window_update_cursor(window *w, const std::vector<std::string>& lines,
                            buffer_position pos, size_t scroll) {
    w->cursor.y = pos.y - scroll;
    w->cursor.x = std::min(
                   std::min(pos.x, lines[pos.y-1].size() + 1),
                   w->width - 1);
}

window window_update_size(window w) {
    struct winsize ws;
    assert(ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0);
    w.width = ws.ws_col;
    w.height = ws.ws_row;
    return w;
}

