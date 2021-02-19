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

void window_render(const buffer& buf, window w) {
    static char command[10000];
    command[0] = '\0';
    for (size_t y = 0; y < w.height; y++) {
        if (buf.lines.size() > y + w.scroll) {
            auto line = *buf.lines[y + w.scroll];
            /* clear line */
            sprintf(command+strlen(command),
                    "\033[%ld;%dH%04lx %02ld\033[K ", (y+1), 1,
                    size_t(buf.lines[y+w.scroll].get()) & 0xffff,
                    buf.lines[y+w.scroll].use_count());
            /* draw chars in line */
            size_t x = 8;
            for (auto [s, e, t] : tokenize_c(line.c_str())) {
                switch (t) {
                    case C_PUNCTUATOR:
                        sprintf(command+strlen(command), "\033[%dm", COLOR_BRIGHT_BLACK);
                        break;
                    case C_LITERAL_DECIMAL:
                    case C_LITERAL_OCTAL:
                    case C_LITERAL_BOOL:
                        sprintf(command+strlen(command), "\033[%dm", COLOR_RED);
                        break;
                    case C_STRING_CHAR:
                        sprintf(command+strlen(command), "\033[%dm", COLOR_CYAN);
                        break;
                    case C_STRING_ESCAPE_SEQUENCE:
                        sprintf(command+strlen(command), "\033[%dm", COLOR_GREEN);
                        break;
                    case C_STRING_ENCODING_PREFIX:
                    case C_STRING_OPENING_QUOTE:
                    case C_STRING_CLOSING_QUOTE:
                    case C_TYPE:
                        sprintf(command+strlen(command), "\033[%dm", COLOR_BLUE);
                        break;
                    case C_KEYWORD:
                        sprintf(command+strlen(command), "\033[%dm", COLOR_MAGENTA);
                        break;
                    default:
                        sprintf(command+strlen(command), "\033[%dm", COLOR_RESET);
                        break;
                }
                auto str = std::u32string(s, e);
                for (auto ch : str) {
                    if (x < w.width) {
                        std::u32string output;
                        output += ch;
                        sprintf(command+strlen(command), utf8_encode(output).c_str());
                    }
                    x++;
                }
                sprintf(command+strlen(command), "\033[%dm", COLOR_RESET);
            }
        } else {
            sprintf(command+strlen(command), "\033[%ld;%dH\033[K ", (y+1), 1);
        }
    }
    printf(command);
}

void window_render_cursor(const buffer& buf, window w, bool insert) {
    if (insert) {
        printf("\033[6 q");
    } else {
        printf("\033[2 q");
    }
    printf("\033[%ld;%ldH", buf.pos.y - w.scroll + 1,
            std::min(buf.pos.x, buf.lines[buf.pos.y]->size()) + 9);
}

window window_update_size(window w) {
    struct winsize ws;
    assert(ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0);
    w.width = ws.ws_col;
    w.height = ws.ws_row;
    return w;
}

