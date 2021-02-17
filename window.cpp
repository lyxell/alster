#include <sys/ioctl.h>
#include "utf8.h"
#include <assert.h>
#include "window.h"
#include "tokenize.h"
#include "colors.h"

std::vector<int> ttkn(const char32_t* YYCURSOR) {
    std::vector<int> tokens;
    const char32_t* YYMARKER;
    for (;;) {
        auto token_start = YYCURSOR;
        /*!re2c
        re2c:define:YYCTYPE = char32_t;
        re2c:yyfill:enable = 0;
        re2c:flags:8 = 0;
        re2c:flags:u = 1;

        c_keyword = "break"
                  | "continue"
                  | "else"
                  | "for"
                  | "return"
                  | "if"
                  | "while";

        c_literal = "\"" [^"\n\x00]* "\""
                  | "true"
                  | "false"
                  | "0"
                  | "NULL"
                  | [1-9][0-9]*;

        c_comment = "//" [^\n\x00]* "\n";

        c_type = "FILE" "*"*
               | "bool" "*"*
               | "char" "*"*
               | "int" "*"*
               | "double" "*"*
               | "const"
               | "float" "*"*
               | "size_t" "*"*
               | "void" "*"*;

        c_identifier = [a-zA-Z_][a-zA-Z_0-9]*;

        end = "\x00";

        end          { return tokens; }
        *            { assert(YYCURSOR - token_start == 1);
                       tokens.push_back(TOKEN_NONE);
                       continue; }
        c_literal    { while (token_start < YYCURSOR) {
                            tokens.push_back(TOKEN_LITERAL);
                            token_start++;
                       };
                       continue; }
        c_type       { while (token_start < YYCURSOR) {
                            tokens.push_back(TOKEN_TYPE);
                            token_start++;
                       };
                       continue; }
        c_keyword    { while (token_start < YYCURSOR) {
                            tokens.push_back(TOKEN_KEYWORD);
                            token_start++;
                       };
                       continue; }
        c_identifier { while (token_start < YYCURSOR) {
                            tokens.push_back(TOKEN_IDENTIFIER);
                            token_start++;
                       };
                       continue; }
        */
    }
    return tokens;
}

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
    /* create windowframe and tokens */
    std::vector<buffer_line> windowframe;
    std::u32string tokens;
    for (size_t i = 0; i < w.height; i++) {
        if (lines.size() > i + w.scroll) {
            windowframe.push_back(buffer_get_line(buf, i + w.scroll));
            tokens += windowframe.back();
            tokens += '\n';
        }
    }
    auto tokenstr = ttkn(tokens.c_str());
    auto ptr = tokenstr.data();
    /* do render */
    size_t y = 0;
    for (auto line : windowframe) {
        /* clear line */
        printf("\033[%ld;%dH%04lx %02ld\033[K ", (y+1), 1,
                size_t(lines[y+w.scroll].get()) % 0xffff,
                lines[y+w.scroll].use_count());
        /* draw chars in line */
        size_t x = 8;
        for (auto ch : line) {
            if (x < w.width) {
                std::u32string s;
                s += ch;
                switch (*ptr) {
                    case TOKEN_LITERAL:
                        set_color(COLOR_YELLOW);
                        break;
                    case TOKEN_KEYWORD:
                        set_color(COLOR_GREEN);
                        break;
                    case TOKEN_TYPE:
                        set_color(COLOR_CYAN);
                        break;
                    default:
                        break;
                }
                printf(utf8_encode(s).c_str());
                set_color(COLOR_RESET);
            }
            x++;
            ptr++;
        }
        y++;
        ptr++;
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

