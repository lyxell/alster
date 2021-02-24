#include "colors.h"
#include "syntax/syntax.h"
#include "tokenize.h"
#include "utf8.h"
#include "window.h"
#include <cassert>
#include <cstring>
#include <sys/ioctl.h>

/*
window window_update_scroll(const buffer& buf, window w) {
    if (buf.pos.y < w.scroll) {
        w.scroll = buf.pos.y;
    } else if (buf.pos.y >= w.scroll + w.height) {
        w.scroll = buf.pos.y - w.height + 1;
    }
    return w;
}*/

int token_to_color(int t) {
    switch (t) {
        case C_PUNCTUATOR:
        case C_SINGLE_LINE_COMMENT:
            return COLOR_BRIGHT_BLACK;
        case C_LITERAL_DECIMAL:
        case C_LITERAL_OCTAL:
        case C_LITERAL_BOOL:
            return COLOR_RED;
        case C_STRING_CHAR:
            return COLOR_CYAN;
        case C_STRING_ESCAPE_SEQUENCE:
            return COLOR_GREEN;
        case C_STRING_ENCODING_PREFIX:
        case C_STRING_OPENING_QUOTE:
        case C_STRING_CLOSING_QUOTE:
        case C_TYPE:
            return COLOR_BLUE;
        case C_KEYWORD:
            return COLOR_MAGENTA;
        default:
            break;
    }
    return COLOR_RESET;
}

window window_render_buffer(window w, const buffer& buf, size_t scroll) {
    for (size_t y = 0; y < w.height; y++) {
        if (y + scroll >= buf.lines.size()) continue;
        size_t x = 0;
        for (auto [s, e, t] : tokenize_c(buf.lines[y + scroll]->c_str())) {
            for (auto ch : std::u32string(s, e)) {
                if (x < w.width) {
                    w.matrix[y][x].ch = ch;
                    w.matrix[y][x].color = token_to_color(t);
                }
                x++;
            }
        }
    }
    return w;
}

std::string window_to_string(window w) {
    std::string str;
    for (size_t y = 0; y < w.height; y++) {
        str += "\x1b[" + std::to_string(y + 1) + ";1H\x1b[K";
        for (size_t x = 0; x < w.width; x++) {
            if (w.matrix[y][x].ch) {
                str += "\x1b[" + std::to_string(w.matrix[y][x].color) + "m";
                str += utf8_encode(std::u32string(1, w.matrix[y][x].ch));
            }
        }
    }
    /*
    if (insert_mode) {
        printf("\x1b[6 q");
    } else {
        printf("\x1b[2 q");
    }*/
    str += "\x1b[" + std::to_string(w.cursor.y + 1) + ";" +
                     std::to_string(w.cursor.x + 1) + "H";
    return str;
}

window window_update_cursor(window w, const buffer& buf, size_t scroll) {
    w.cursor.y = buf.pos.y - scroll;
    w.cursor.x = std::min(
                   std::min(buf.pos.x, buf.lines[buf.pos.y]->size()),
                   w.width - 1);
    return w;
}

window window_update_size(window w) {
    struct winsize ws;
    assert(ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0);
    w.width = ws.ws_col;
    w.height = ws.ws_row;
    w.matrix.resize(w.height);
    for (size_t i = 0; i < w.height; i++) {
        w.matrix[i].resize(w.width);
    }
    return w;
}

