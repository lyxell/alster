#include <cassert>
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>

#include "tty.h"
#include "window.h"
#include "buffer.h"
#include "tokenize.h"
#include "file.h"

std::chrono::time_point<std::chrono::high_resolution_clock> start;
std::chrono::time_point<std::chrono::high_resolution_clock> end;

#define KEY_ESCAPE '\x1b'

enum {
    MODE_NORMAL,
    MODE_INSERT
};

struct state {
    int mode;
    const char* status;
    std::vector<buffer> history;
    std::vector<buffer> future;
};

using editor = std::tuple<buffer,state,window>;

editor editor_handle_input(buffer b, state s, window w, std::istream& in) {
    int c;
    auto get = [&](){
        c = getchar();
        start = std::chrono::high_resolution_clock::now();
        return c;
    };
    switch (s.mode) {
    case MODE_NORMAL:
        switch (get()) {
        case '1' ... '9':
            return {std::move(b),std::move(s),w};
        case '$':
            return {buffer_move_end_of_line(std::move(b)),
                    std::move(s), w};
        case '0':
            return {buffer_move_start_of_line(std::move(b)),
                    std::move(s), w};
        case 'A':
            s.mode = MODE_INSERT;
            if (!s.history.size() || s.history.back().lines != b.lines) {
                s.history.push_back(b);
            }
            return {buffer_move_end_of_line(std::move(b)), s, w};
        case 'G':
            return {buffer_move_end(std::move(b)), s, w};
        case 'd': {
            switch (get()) {
            case 'd':
                s.history.push_back(b);
                s.future.clear();
                return {buffer_erase_current_line(std::move(b)), s, w};
            default:
                return {b,s,w};
            }
        }
        case 'g': {
            switch (get()) {
            case 'g':
                return {buffer_move_start(std::move(b)), s, w};
            default:
                return {b,s,w};
            }
        }
        case 'h':
            return {buffer_move_left(std::move(b), 1), std::move(s), w};
        case 'i':
            s.mode = MODE_INSERT;
            if (!s.history.size() || s.history.back().lines != b.lines) {
                s.history.push_back(b);
            }
            return {std::move(b), std::move(s), std::move(w)};
        case 'j':
            return {buffer_move_down(std::move(b), 1),
                        std::move(s), std::move(w)};
        case 'k':
            return {buffer_move_up(std::move(b), 1), std::move(s), w};
        case 'l':
            return {buffer_move_right(std::move(b), 1), std::move(s), w};
        case 'u':
            if (s.history.empty()) {
                s.status = "History empty!";
            } else {
                s.future.push_back(std::move(b));
                b = std::move(s.history.back());
                s.history.pop_back();
            }
            return {std::move(b), std::move(s), w};
        case 'r':
            if (s.future.empty()) {
                s.status = "Future empty!";
            } else {
                s.history.push_back(std::move(b));
                b = std::move(s.future.back());
                s.future.pop_back();
            }
            return {std::move(b), std::move(s), w};
        case 'q':
            exit(0);
        case 's':
            file_save("test2", b); return {std::move(b), std::move(s), w};
        default:
            return {b,s,w};
        }
    case MODE_INSERT:
        switch (get()) {
        case '\x7f':
        case '\b':
            s.future.clear();
            return {buffer_erase(std::move(b)), s, w};
        case '\x1b':
            s.mode = MODE_NORMAL;
            if (b.lines == s.history.back().lines) {
                s.history.pop_back();
            }
            return {buffer_move_left(std::move(b), 1), std::move(s), w};
        case '\r':
            s.future.clear();
            return {buffer_break_line(std::move(b)), s, w};
        case '\t':
            s.future.clear();
            return {buffer_indent(std::move(b)), s, w};
        default:
            s.future.clear();
            return {buffer_insert(std::move(b), c), s, w};
        }
    default:
        break;
    }
    assert(false);
    return {b, s, w};
}

editor editor_draw(buffer buf, state s, window win) {
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    win = window_update_size(win);
    win = window_update_scroll(buf, win);
    window_render(buf, win);
    if (s.status) {
        printf("\033[%ld;%ldH%s\033[K", win.height, 0ul, s.status);
        s.status = NULL;
    }
    end = high_resolution_clock::now();
    auto diff = duration_cast<milliseconds>(end-start).count();
    printf("\033[%ld;%ldH%4ld", win.height, win.width - 4, diff % 1000);
    window_render_cursor(buf, win, s.mode == MODE_INSERT);
    return {
        std::move(buf),
        std::move(s),
        std::move(win)
    };
}

int main(int argc, char* argv[]) {
    editor ed {};
    if (argc > 1) {
        std::get<0>(ed) = file_load(argv[1]);
    } else {
        std::get<0>(ed) = {{std::make_shared<buffer_line>()}, {0, 0}};
    }
    assert(tty_enable_raw_mode() == 0);
    while (true) {
        ed = editor_draw(std::move(std::get<0>(ed)),
                         std::move(std::get<1>(ed)),
                         std::move(std::get<2>(ed)));
        ed = editor_handle_input(std::move(std::get<0>(ed)),
                                 std::move(std::get<1>(ed)),
                                 std::move(std::get<2>(ed)),
                                 std::cin);
    }
    return 0;
}

