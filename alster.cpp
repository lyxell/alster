#include <ncurses.h>
#include <chrono>
#include <stdbool.h>
#include <assert.h>
#include <string>
#include <fstream>
#include <streambuf>

#include "buffer.h"
#include "tokenize.h"

enum {
    MODE_NORMAL,
    MODE_INSERT
};

struct state {
    bool exiting;
    int mode;
    int scroll;
    size_t window_width;
    size_t window_height;
};

state
update_window_size(const state& s) {
    state next = s;
    next.window_width = COLS;
    next.window_height = LINES;
    return next;
}

state
update_scroll(const state& s, const buffer& b) {
    const auto& [lines, pos] = b;
    state next = s;
    if (pos.y < s.scroll) {
        next.scroll = pos.y;
    } else if (pos.y >= s.scroll + s.window_height) {
        next.scroll = pos.y - s.window_height + 1;
    }
    return next;
}

void render(const buffer& buf, const state& s) {
    const auto& [lines, pos] = buf;
    std::string str = buffer_to_string(buf);
    std::vector<buffer_char> color(str.size());
    tokenize(str.c_str(), color.data());
    std::vector<std::vector<buffer_char>> colors = {std::vector<buffer_char>()};
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '\n') {
            colors.emplace_back();
        } else {
            colors.back().push_back(color[i]);
        }
    } 
    for (size_t i = 0; i < s.window_height; i++) {
        move(i, 0);
        if (lines.size() > i + s.scroll) {
            auto line = buffer_get_line(buf, i + s.scroll);
            for (size_t j = 0; j < std::min(s.window_width, line.size()); j++) {
                attron(COLOR_PAIR(colors[i+s.scroll][j]));
                addch(line[j]);
                attroff(COLOR_PAIR(colors[i+s.scroll][j]));
            }
        }
        clrtoeol();
    }
    if (buffer_get_line(buf, pos.y).size() > pos.x) {
        mvprintw(0, COLS - 4, "%04d", buffer_get_line(buf, pos.y)[pos.x]);
    }
}

void render_cursor(const buffer& buf, const state& s) {
    auto [lines, pos] = buf;
    move(pos.y - s.scroll,
         std::min(pos.x, buffer_get_line(buf, pos.y).size()));
}

template <typename S, typename T>
void handle_input(buffer& buf, state& s, S YYPEEK, T YYSKIP) {
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
        "$"  { buf = buffer_move_end_of_line(buf); return; }
        "0"  { buf = buffer_move_start_of_line(buf); return; }
        "A"  { buf = buffer_move_end_of_line(buf);
               s.mode = MODE_INSERT;
               return; }
        "G"  { buf = buffer_move_end(buf); return; }
        "gg" { buf = buffer_move_start(buf); return; }
        "h"  { buf = buffer_move_left(buf, 1); return; }
        "i"  { s.mode = MODE_INSERT; return; }
        "j"  { buf = buffer_move_down(buf, 1); return; }
        "k"  { buf = buffer_move_up(buf, 1); return; }
        "l"  { buf = buffer_move_right(buf, 1); return; }
        *    { return; }
        null { return; }
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

void handle_input_stdin(buffer& buf, state& s) {
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
    handle_input(buf, s, YYPEEK, YYSKIP);
}

#ifndef FUZZ
int main(int argc, char* argv[]) {
    state s = {};
    buffer buf = {
        {std::make_shared<buffer_line>()},
        {0,0}
    };
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
    init_pair(5, 8, 0);
    init_pair(6, 1, 0);
    init_pair(7, 2, 0);
    init_pair(8, 3, 0);
    init_pair(9, 8, 0);
    init_pair(10, 1, 0);
    init_pair(11, 2, 0);
    init_pair(12, 3, 0);
    /*
    if (argc > 1) {
        std::ifstream t(argv[1]);
        std::string str((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());
        buf = buffer(str);
    }*/
    while (true) {
        auto timer_start = std::chrono::high_resolution_clock::now();
        s = update_window_size(s);
        s = update_scroll(s, buf);
        render(buf, s);
        auto timer_end = std::chrono::high_resolution_clock::now();
        int timer_elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count();
        mvprintw(1, COLS - 9, "%6d ms", timer_elapsed_ms);
        render_cursor(buf, s);
        refresh();
        handle_input_stdin(buf, s);
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
    buffer buf = {
        {std::make_shared<buffer_line>()},
        {0,0}
    };
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
