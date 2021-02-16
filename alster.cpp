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
    bool undo;
    bool persist;
    bool redo;
    int mode;
    int scroll;
    size_t window_width;
    size_t window_height;
};

state
state_enter_insert_mode(state s) {
    s.mode = MODE_INSERT;
    return s;
}

state
state_set_undo_flag(state s) {
    s.undo = true;
    return s;
}

state
state_set_redo_flag(state s) {
    s.redo = true;
    return s;
}

state
state_set_persist_flag(state s) {
    s.persist = true;
    return s;
}

state
state_enter_normal_mode(state s) {
    s.mode = MODE_NORMAL;
    return s;
}

state
state_update_window_size(const state& s) {
    state next = s;
    next.window_width = COLS;
    next.window_height = LINES;
    return next;
}

state
state_update_scroll(const state& s, const buffer& b) {
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
            printw("%8x", lines[i+s.scroll].get());
            addch(' ');
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
    move(pos.y - s.scroll, std::min(pos.x, buffer_get_line(buf, pos.y).size()) + 9);
}

template <typename S, typename T>
std::pair<buffer, state>
handle_input(const buffer& b, const state& s, S YYPEEK, T YYSKIP) {
    /*!re2c
    re2c:flags:input = custom;
    re2c:define:YYCTYPE = char;
    re2c:yyfill:enable = 0;
    nul = "\x00";
    esc = "\x1b";
    ret = "\x0d";
    del = "\x7f";
    tab = "\x09";
    */
    if (s.mode == MODE_NORMAL) {
        /*!re2c
        "$"  {return {buffer_move_end_of_line(b), s};}
        "0"  {return {buffer_move_start_of_line(b), s};}
        "A"  {return {buffer_move_end_of_line(b), state_enter_insert_mode(s)};}
        "G"  {return {buffer_move_end(b), s};}
        "dd" {return {buffer_erase_current_line(b), state_set_persist_flag(s)};}
        "gg" {return {buffer_move_start(b), s};}
        "h"  {return {buffer_move_left(b, 1), s};}
        "i"  {return {b, state_enter_insert_mode(s)};}
        "j"  {return {buffer_move_down(b, 1), s};}
        "k"  {return {buffer_move_up(b, 1), s};}
        "l"  {return {buffer_move_right(b, 1), s};}
        "u"  {return {b, state_set_undo_flag(s)};}
        "r"  {return {b, state_set_redo_flag(s)};}
        *    {return {b, s};}
        */
    } else if (s.mode == MODE_INSERT) {
        /*!re2c
        del  {return {buffer_erase(b), s};}
        esc  {return {buffer_move_left(b, 1),
                      state_set_persist_flag(state_enter_normal_mode(s))};}
        ret  {return {buffer_break_line(b), s};}
        tab  {return {buffer_insert(b, ' ', 4), s};}
        nul  {return {b, s};}
        *    {return {buffer_insert(b, yych, 1), s};}
        */
    }
    return {b, s};
}

std::pair<buffer, state>
handle_input_stdin(buffer& b, state& s) {
    char c;
    bool skip = true;
    return handle_input(
        b,
        s,
        [&](){
            if (skip) {
                c = getchar(); 
                skip = false;
            }
            return c;
        },
        [&](){
            skip = true;
        }
    );
}

#ifndef FUZZ
int main(int argc, char* argv[]) {
    std::vector<buffer> history;
    std::vector<buffer> future;
    state s = {};
    buffer b = {
        {std::make_shared<buffer_line>()},
        {0, 0}
    };
    history.push_back(b);
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
        s = state_update_window_size(s);
        s = state_update_scroll(s, b);
        render(b, s);
        auto timer_end = std::chrono::high_resolution_clock::now();
        int timer_elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count();
        mvprintw(1, COLS - 9, "%6d ms", timer_elapsed_ms);
        render_cursor(b, s);
        refresh();
        auto [next_b, next_s] = handle_input_stdin(b, s);
        /* undo/redo */
        if (next_s.redo) {
            if (future.size()) {
                history.push_back(b);
                b = future.back();
                future.pop_back();
            }
        } else if (next_s.undo) {
            if (history.size()) {
                future.push_back(b);
                b = history.back();
                history.pop_back();
            }
        } else if (next_s.persist && (!history.size() || history.back().first != next_b.first)) {
            future.clear();
            history.push_back(b);
            b = next_b;
            s = next_s;
            s.persist = false;
        } else {
            b = next_b;
            s = next_s;
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
