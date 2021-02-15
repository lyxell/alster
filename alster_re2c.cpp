#include <ncurses.h>
#include <chrono>
#include <stdbool.h>
#include <string>
#include <fstream>
#include <streambuf>

#include "buffer.h"
#include "tokenize/tokenize.h"

enum {
    MODE_NORMAL,
    MODE_INSERT
};

struct state {
    bool exiting;
    int mode;
    int scroll;
};

void render(const buffer_t& buf, const state& s) {
    static buffer_t oldBuf;
    if (oldBuf == buf) return;
    oldBuf = buf;
    std::string str = buffer_to_string(buf);
    std::vector<buffer_char_t> color(str.size());
    tokenize_c(str.c_str(), color.data());
    std::vector<std::vector<buffer_char_t>> colors = {std::vector<buffer_char_t>()};
    for (size_t i = 0; i < color.size(); i++) {
        if (str[i] == '\n') {
            colors.emplace_back();
        } else {
            colors.back().push_back(color[i]);
        }
    }
    for (size_t i = 0; i < size_t(LINES); i++) {
        move(i, 0);
        if (buf.lines.size() > i + s.scroll) {
            auto str = buffer_get_line(buf, i + s.scroll);
            for (size_t j = 0; j < std::min(size_t(COLS), str.size()); j++) {
                switch (colors[i+s.scroll][j]) {
                case TOKEN_PREPROC:
                case TOKEN_COMMENT:
                    attron(COLOR_PAIR(1));
                    mvaddch(i, j, str[j]);
                    attroff(COLOR_PAIR(1));
                    break;
                case TOKEN_BOOL_LIT:
                case TOKEN_INT_LIT:
                case TOKEN_STRING_LIT:
                    attron(COLOR_PAIR(2));
                    mvaddch(i, j, str[j]);
                    attroff(COLOR_PAIR(2));
                    break;
                case TOKEN_TYPE_QUALIFIER:
                case TOKEN_TYPE:
                    attron(COLOR_PAIR(3));
                    mvaddch(i, j, str[j]);
                    attroff(COLOR_PAIR(3));
                    break;
                case TOKEN_KEYWORD:
                    attron(COLOR_PAIR(4));
                    mvaddch(i, j, str[j]);
                    attroff(COLOR_PAIR(4));
                    break;
                default:
                    mvaddch(i, j, str[j]);
                    break;
                }
            }
        }
        clrtoeol();
    }
    mvprintw(0, COLS - 4, "%04d", buffer_get_line(buf, buf.y)[buf.x]);
}

void render_cursor(const buffer_t& buf, const state& s) {
    move(buf.y - s.scroll, buf.x);
}

template <typename S, typename T>
void handle_input(buffer_t& buf, state& s, S YYPEEK, T YYSKIP) {
/*!re2c
    re2c:flags:input = custom;
    re2c:define:YYCTYPE = char;
    re2c:yyfill:enable = 0;

    null    = "\x00";
    escape  = "\x1b";
    return  = "\x0d";
    delete  = "\x7f";
    tab     = "\x09";
*/
    if (s.mode == MODE_NORMAL) {
/*!re2c
        "h"  { buf = buffer_move_left(buf, 1); return; }
        "l"  { buf = buffer_move_right(buf, 1); return; }
        "j"  { buf = buffer_move_down(buf, 1); return; }
        "k"  { buf = buffer_move_up(buf, 1); return; }
        "i"  { s.mode = MODE_INSERT; return; }
        null { return; }
        *    { return; }
*/
    } else if (s.mode == MODE_INSERT) {
/*!re2c
        delete { buf = buffer_erase(buf); return; }
        escape { s.mode = MODE_NORMAL; return; }
        return { buf = buffer_break_line(buf); return; }
        tab    { for (int i = 0; i < 4; i++) buf = buffer_insert(buf, ' ');
                 return; }
        null   { return; }
        *      { buf = buffer_insert(buf, yych); return; }
*/
    }
}

#ifndef FUZZ
int main(int argc, char* argv[]) {
    state s = {};
    buffer_t buf = {};
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, true);
    set_escdelay(100);
    use_default_colors();
    start_color();
    init_pair(1, 8, 0);
    init_pair(2, 1, 0);
    init_pair(3, 2, 0);
    init_pair(4, 3, 0);
    if (argc > 1) {
        std::ifstream t(argv[1]);
        std::string str((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());
        buf = buffer_t(str);
    }
    while (true) {
        auto start = std::chrono::high_resolution_clock::now();
        if (buf.y < s.scroll) {
            s.scroll = buf.y;
        } else if (buf.y >= s.scroll + LINES) {
            s.scroll = buf.y - LINES + 1;
        }
        render(buf, s);
        char c;
        bool skip = true;
        auto YYPEEK = [&](){
            if (skip) {
                c = getchar(); 
                skip = false;
            }
            return c;
        };
        auto YYSKIP = [&](){
            skip = true;
        };
        auto end = std::chrono::high_resolution_clock::now();
        int ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        mvprintw(1, COLS - 9, "%6d us", ms);
        render_cursor(buf, s);
        refresh();
        handle_input(buf, s, YYPEEK, YYSKIP);
        if (s.exiting) {
            break;
        }
    }
    // refresh display
    refresh();
    // end ncurses mode
    endwin();
    return 0;
}
#else
extern "C"
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    state s = {};
    buffer_t buf = {};
    size_t curr = 0;
    auto YYPEEK = [&](){
        return data[curr];
    };
    auto YYSKIP = [&](){
        if (curr + 1 < size) {
            curr++;
        }
    };
    while (true) {
        handle_input(buf, s, YYPEEK, YYSKIP);
        if (curr + 1 >= size) break;
    }
    return 0;
}
#endif
