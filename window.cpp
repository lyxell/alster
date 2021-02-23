#include <sys/ioctl.h>
#include <cstring>
#include "utf8.h"
#include <cassert>
#include "window.h"
#include "tokenize.h"
#include "syntax/syntax.h"
#include "colors.h"

window window_update_scroll(const buffer& buf, window w) {
    if (buf.pos.y < w.scroll) {
        w.scroll = buf.pos.y;
    } else if (buf.pos.y >= w.scroll + w.height) {
        w.scroll = buf.pos.y - w.height + 1;
    }
    return w;
}

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

void window_render(const buffer& buf, window w, std::optional<buffer_position> visual_marker) {
    static char command[10000];
    command[0] = '\0';
    for (size_t y = 0; y < w.height; y++) {
        sprintf(command + strlen(command), "\x1b[%ld;%dH\x1b[K", (y+1), 1);
        if (y + w.scroll >= buf.lines.size()) continue;
        auto tokens = tokenize_c(buf.lines[y + w.scroll]->c_str());
        size_t x = 0;
        for (auto [s, e, t] : tokens) {
            if (std::pair(y, x) > std::pair(buf.pos.y, buf.pos.x)) {
                sprintf(command + strlen(command), "\x1b[%dm", COLOR_YELLOW);
            } else {
                sprintf(command + strlen(command), "\x1b[%dm",
                        token_to_color(t));
            }
            auto str = std::u32string(s, e);
            for (auto ch : str) {
                if (x < w.width) {
                    std::u32string output;
                    output += ch;
                    sprintf(command + strlen(command), "%s", utf8_encode(output).c_str());
                }
                x++;
            }
            sprintf(command + strlen(command), "\x1b[%dm", COLOR_RESET);
        }
    }
    printf("%s", command);
}

void window_render_cursor(const buffer& buf, window w, bool insert) {
    if (insert) {
        printf("\x1b[6 q");
    } else {
        printf("\x1b[2 q");
    }
    printf("\x1b[%ld;%ldH", buf.pos.y - w.scroll + 1,
            std::min(buf.pos.x, buf.lines[buf.pos.y]->size()) + 1);
}

window window_update_size(window w) {
    struct winsize ws;
    assert(ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0);
    w.width = ws.ws_col;
    w.height = ws.ws_row;
    return w;
}

