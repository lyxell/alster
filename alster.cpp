#include <cassert>
#include <chrono>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>

#include "utf8.h"
#include "tty.h"
#include "window.h"
#include "buffer.h"
#include "tokenize.h"
#include "file.h"

struct timer {
    std::chrono::time_point<std::chrono::high_resolution_clock> s;
    void start() {
        s = std::chrono::high_resolution_clock::now();
    }
    void report(const char* str) {
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::microseconds;
        auto e = high_resolution_clock::now();
        auto diff = duration_cast<microseconds>(e - s).count();
        fprintf(stderr, "%s %ld μs\n", str, diff);
    }
};

enum {
    MODE_NORMAL,
    MODE_INSERT
};

struct editor {
    std::u32string cmd;
    int mode;
    char status[120];
    buffer buf;
    std::vector<buffer> history;
    std::vector<buffer> future;
    bool exiting;
    bool saving;
};

// pure function
editor editor_handle_command_normal(editor e) {
    const char32_t *YYCURSOR = e.cmd.c_str();
    const char32_t *YYMARKER;
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:flags:unicode = 1;
    re2c:define:YYCTYPE = char32_t;
    "$" {
        e.cmd = {};
        e.buf = buffer_move_end_of_line(std::move(e.buf));
        return e;
    }
    "A" {
        e.cmd = {};
        e.mode = MODE_INSERT;
        if (!e.history.size() || e.history.back().lines != e.buf.lines) {
            e.history.push_back(e.buf);
        }
        e.buf = buffer_move_end_of_line(std::move(e.buf));
        return e;
    }
    "G" {
        e.cmd = {};
        e.buf = buffer_move_end(std::move(e.buf));
        return e;
    }
    "dd" {
        e.cmd = {};
        e.history.push_back(e.buf);
        e.future.clear();
        e.buf = buffer_erase_current_line(std::move(e.buf));
        return e;
    }
    "gg" {
        e.cmd = {};
        e.buf = buffer_move_start(std::move(e.buf));
        return e;
    }
    "i" {
        e.cmd = {};
        e.mode = MODE_INSERT;
        if (!e.history.size() || e.history.back().lines != e.buf.lines) {
            e.history.push_back(e.buf);
        }
        return e;
    }
    "0" {
        e.cmd = {};
        e.buf = buffer_move_start_of_line(std::move(e.buf));
        return e;
    }
    ([1-9][0-9]*)? "h" {
        e.cmd = {};
        e.buf = buffer_move_left(std::move(e.buf), 1);
        return e;
    }
    ([1-9][0-9]*)? "j" {
        e.cmd = {};
        e.buf = buffer_move_down(std::move(e.buf), 1);
        return e;
    }
    ([1-9][0-9]*)? "k" {
        e.cmd = {};
        e.buf = buffer_move_up(std::move(e.buf), 1);
        return e;
    }
    ([1-9][0-9]*)? "l" {
        e.cmd = {};
        e.buf = buffer_move_right(std::move(e.buf), 1);
        return e;
    }
    "u" {
        e.cmd = {};
        if (e.history.empty()) {
            sprintf(e.status, "History empty!");
        } else {
            e.future.push_back(std::move(e.buf));
            e.buf = std::move(e.history.back());
            e.history.pop_back();
        }
        return e;
    }
    "r" {
        e.cmd = {};
        if (e.future.empty()) {
            sprintf(e.status, "Future empty!");
        } else {
            e.history.push_back(std::move(e.buf));
            e.buf = std::move(e.future.back());
            e.future.pop_back();
        }
        return e;
    }
    "q" {
        e.cmd = {};
        e.exiting = true;
        return e;
    }
    "∂" {
        e.cmd = {};
        sprintf(e.status, "deriving...");
        return e;
    }
    "s" {
        e.cmd = {};
        e.saving = true;
        return e;
    }
    * {
        // if yych == '\0' we have read until end, i.e. the command
        // is a prefix of some command
        if (yych != '\0') {
            sprintf(e.status, "Unknown command %s",
                    utf8_encode(e.cmd.c_str()).c_str());
            e.cmd = {};
        } else {
            sprintf(e.status, "%s", utf8_encode(e.cmd.c_str()).c_str());
        }
        return e;
    }
    */
    return e;
}

// pure function
editor editor_handle_command_insert(editor e) {
    const char32_t *YYCURSOR = e.cmd.c_str();
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:define:YYCTYPE = char32_t;
    "\x7f" | "\b" {
        e.cmd = {};
        e.future.clear();
        e.buf = buffer_erase(std::move(e.buf));
        return e;
    }
    "\x1b" {
        e.cmd = {};
        e.mode = MODE_NORMAL;
        if (e.buf.lines == e.history.back().lines) {
            e.history.pop_back();
        }
        e.buf = buffer_move_left(std::move(e.buf), 1);
        return e;
    }
    "\r" {
        e.cmd = {};
        e.future.clear();
        e.buf = buffer_break_line(std::move(e.buf));
        return e;
    }
    "\t" {
        e.cmd = {};
        e.future.clear();
        e.buf = buffer_indent(std::move(e.buf));
        return e;
    }
    * {
        e.cmd = {};
        e.buf = buffer_insert(std::move(e.buf), yych);
        return e;
    }
    */
    return e;
}

// pure function
editor editor_handle_command(editor e) {
    if (e.mode == MODE_INSERT) {
        return editor_handle_command_insert(std::move(e));
    } else if (e.mode == MODE_NORMAL) {
        return editor_handle_command_normal(std::move(e));
    }
    return e;
}

window editor_draw(const editor& e, window win) {
    win = window_update_size(win);
    win = window_update_scroll(e.buf, win);
    window_render(e.buf, win);
    if (strlen(e.status)) {
        printf("\033[%ld;%ldH%s\033[K", win.height, 0ul, e.status);
    }
    window_render_cursor(e.buf, win, e.mode == MODE_INSERT);
    return win;
}

int main(int argc, char* argv[]) {
    editor e {};
    timer t {};
    window win {};
    if (argc > 1) {
        e.buf = file_load(argv[1]);
    } else {
        e.buf = {{std::make_shared<buffer_line>()}, {0, 0}};
    }
    assert(tty_enable_raw_mode() == 0);
    while (true) {
        win = editor_draw(e, win);
        e.status[0] = '\0';
        t.report("render:    ");
        e.cmd.push_back(getchar_utf8());
        t.start();
        e = editor_handle_command(std::move(e));
        t.report("handle cmd:");
        t.start();
        if (e.exiting) break;
    }
    return 0;
}

