#include <ncurses.h>
#include <stdbool.h>
#include <string>
#include <fstream>
#include <streambuf>

#include "buffer.h"

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
    mvprintw(0, 0, buffer_to_string(buf).c_str());
    for (int i = 0; i < LINES; i++) {
        mvprintw(i, 0, buffer_get_line(buf, i + s.scroll).c_str());
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
