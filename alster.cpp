#include <chrono>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <string>
#include <termios.h>

#include "buffer.h"
#include "tokenize.h"
#include "file.h"

static struct termios orig_termios; /* In order to restore at exit.*/

void disable_raw_mode(int fd) {
    /* Don't even check the return value as it's too late. */
    tcsetattr(fd,TCSAFLUSH,&orig_termios);
}

/* Called at exit to avoid remaining in raw mode. */
void editor_at_exit(void) {
    disable_raw_mode(STDIN_FILENO);
}

/* Raw mode: 1960 magic shit. */
int enable_raw_mode() {
    struct termios raw;

    if (!isatty(STDIN_FILENO)) goto fatal;
    atexit(editor_at_exit);
    if (tcgetattr(STDIN_FILENO,&orig_termios) == -1) goto fatal;

    raw = orig_termios;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw) < 0) goto fatal;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}

enum {
    COL_BLACK,
    COL_RED,
    COL_GREEN,
    COL_YELLOW,
    COL_BLUE,
    COL_PURPLE,
    COL_CYAN,
    COL_WHITE,
    COL_BLACK_LIGHT,
    COL_RED_LIGHT,
    COL_GREEN_LIGHT,
    COL_YELLOW_LIGHT,
    COL_BLUE_LIGHT,
    COL_PURPLE_LIGHT,
    COL_CYAN_LIGHT,
    COL_WHITE_LIGHT
};

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
    size_t scroll;
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
    struct winsize ws;
    assert(ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0);
    state next = s;
    next.window_width = ws.ws_col;
    next.window_height = ws.ws_row;
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
    static std::vector<std::shared_ptr<buffer_line>> prev_lines;
    static size_t prev_scroll;
    const auto& [lines, pos] = buf;
    if (prev_lines == lines && prev_scroll == s.scroll) return;
    prev_lines = lines;
    prev_scroll = s.scroll;
    for (size_t i = 0; i < s.window_height; i++) {
        if (lines.size() > i + s.scroll) {
            auto line = buffer_get_line(buf, i + s.scroll);
            printf("\033[%ld;%dH%lx \033[K", (i+1), 1,
                    (size_t) lines[i+s.scroll].get());
            for (int j = 0; j < int(std::min(s.window_width - 13, line.size())); j++) {
                putchar(line[j]);
            }
        }
    }
}

void render_cursor(const buffer& buf, const state& s) {
    auto [lines, pos] = buf;
    printf("\033[%ld;%ldH", pos.y - s.scroll + 1,
            std::min(pos.x, buffer_get_line(buf, pos.y).size()) + 14);
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
        "w"  {printf("\033D"); return {b, s};}
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
    assert(enable_raw_mode() == 0);
    if (argc > 1) {
        file_load(argv[1]);
    }
    history.push_back(b);
    while (true) {
        auto timer_start = std::chrono::high_resolution_clock::now();
        s = state_update_window_size(s);
        s = state_update_scroll(s, b);
        render(b, s);
        //refresh();
        auto timer_end = std::chrono::high_resolution_clock::now();
        int timer_elapsed_ms = std::chrono::duration_cast<
                std::chrono::microseconds>(timer_end - timer_start).count();
        printf("\033[%ld;%ldH%6d",
                s.window_height - 1,
                s.window_width - 9,
                timer_elapsed_ms);
        render_cursor(b, s);
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
        } else if (next_s.persist && (!history.size() ||
                              history.back().first != next_b.first)) {
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
