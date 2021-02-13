#include <ncurses.h>
#include <stdbool.h>
#include <string>
#include <fstream>
#include <streambuf>

#include "buffer.h"
#include "tokenize/tokenize.h"

#define KEY_ESCAPE 27

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
    for (int i = 0; i < LINES; i++) {
        auto str = buffer_get_line(buf, i + s.scroll);
        move(i, 0);
        for (int j = 0; j < std::min(size_t(COLS), str.size()); j++) {
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
        clrtoeol();
    }
    move(buf.y - s.scroll, buf.x);
    refresh();
}

int main(int argc, char* argv[]) {
    state s = {};
    const char* filename = "alster.cpp";
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
    if (filename) {
        std::ifstream t(filename);
        std::string str((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());
        buf = buffer_t(str);
    }
    while (true)
    {
        if (buf.y < s.scroll) {
            s.scroll = buf.y;
        } else if (buf.y >= s.scroll + LINES) {
            s.scroll = buf.y - LINES + 1;
        }
        render(buf, s);
        int input = getch();
        if (s.mode == MODE_NORMAL) {
            switch (input) {
            case 'l':
                buf = buffer_move_right(buf, 1);
                break;
            case 'h':
                buf = buffer_move_left(buf, 1);
                break;
            case 'k':
                buf = buffer_move_up(buf, 1);
                break;
            case 'j':
                buf = buffer_move_down(buf, 1);
                break;
            case 'i':
                s.mode = MODE_INSERT;
                break;
            case 'q':
                s.exiting = true;
                break;
            }
        } else if (s.mode == MODE_INSERT) {
            switch (input) {
            case '\n':
                buf = buffer_break_line(buf);
                break;
            case KEY_BACKSPACE:
                buf = buffer_erase(buf);
                break;
            case KEY_ESCAPE:
                s.mode = MODE_NORMAL;
                break;
            default:
                buf = buffer_insert(buf, input);
                break;
            }
        }
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
