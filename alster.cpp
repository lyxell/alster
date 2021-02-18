#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>

#include "tty.h"
#include "window.h"
#include "buffer.h"
#include "tokenize.h"
#include "file.h"

const char* output = "test2";
const char* MESSAGE_COMMAND_NOT_FOUND = "No such command.";

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

std::tuple<buffer,state,window>
handle_input(buffer b, state s, window w, std::istream& in) {
    auto& [lines, pos] = b;
    std::streampos marker;
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:flags:input = custom;
    re2c:api:style = free-form;
    re2c:define:YYCTYPE   = int;
    re2c:define:YYPEEK    = "in.peek()";
    re2c:define:YYSKIP    = "in.ignore();";
    re2c:define:YYBACKUP  = "marker = in.tellg();";
    re2c:define:YYRESTORE = "in.seekg(marker);";
    nul = "\x00";
    esc = "\x1b";
    ret = "\x0d";
    del = "\x7f";
    tab = "\x09";
    */
    if (s.mode == MODE_NORMAL) {
        /*!re2c

        "$" {
            return {buffer_move_end_of_line(std::move(lines), pos), s, w};
        }

        "0" {
            return {buffer_move_start_of_line(std::move(lines), pos), s, w};
        }

        "A" {
            s.history.push_back(b);
            s.mode = MODE_INSERT;
            s.future.clear();
            return {buffer_move_end_of_line(std::move(lines), pos), s, w};
        }

        "G" {
            return {buffer_move_end(std::move(lines), pos), s, w};
        }

        "dd" {
            s.history.push_back(b);
            s.future.clear();
            return {buffer_erase_current_line(std::move(lines), pos), s, w};
        }

        "gg" {
            return {buffer_move_start(std::move(lines), pos), s, w};
        }

        "h" {
            return {buffer_move_left(std::move(lines), pos, 1), s, w};
        }

        "i" {
            s.history.push_back(b);
            s.mode = MODE_INSERT;
            s.future.clear();
            return {{std::move(lines), pos}, s, w};
        }

        "j" {
            return {buffer_move_down(std::move(lines), pos, 1), s, w};
        }

        "k" {
            return {buffer_move_up(std::move(lines), pos, 1), s, w};
        }

        "l" {
            return {buffer_move_right(std::move(lines), pos, 1), s, w};
        }

        "u" {
            if (s.history.empty()) {
                s.status = "History empty!";
            } else {
                s.future.push_back(b);
                b = std::move(s.history.back());
                s.history.pop_back();
            }
            return {{std::move(lines), pos}, s, w};
        }

        "r" {
            if (s.future.empty()) {
                s.status = "Future empty!";
            } else {
                s.history.push_back(b);
                b = std::move(s.future.back());
                s.future.pop_back();
            }
            return {{std::move(lines), pos}, s, w};
        }

        "q" {
            exit(0);
        }

        "s" {
            file_save(output, b); return {{std::move(lines), pos}, s, w};
        }

        * {
            s.status = MESSAGE_COMMAND_NOT_FOUND;
            return {{std::move(lines), pos}, s, w};
        }

        */
    } else if (s.mode == MODE_INSERT) {
        /*!re2c

        del {
            return {buffer_erase(std::move(lines), pos), s, w};
        }

        esc {
            s.mode = MODE_NORMAL;
            return {buffer_move_left(std::move(lines), pos, 1), s, w};
        }

        ret {
            return {buffer_break_line(std::move(lines), pos), s, w};
        }

        tab {
            return {buffer_insert(std::move(lines), pos, ' ', 4), s, w};
        }

        nul {
            return {{std::move(lines), pos}, s, w};
        }

        * {
            return {buffer_insert(std::move(lines), pos, yych, 1), s, w};
        }

        */
    }
    return {b, s, w};
}

std::tuple<buffer,state,window> next_frame(buffer buf, state s, window win) {
    win = window_update_size(win);
    win = window_update_scroll(buf, win);
    window_render(buf, win);
    if (s.status) {
        printf("\033[%ld;%ldH%s\033[K", win.height, 0ul, s.status);
        s.status = NULL;
    }
    window_render_cursor(buf, win);
    return handle_input(std::move(buf),
                        std::move(s),
                        std::move(win),
                        std::cin);

}

int main(int argc, char* argv[]) {

    std::tuple<buffer,state,window> editor {
        {{std::make_shared<buffer_line>()},{0, 0}},
        {},
        {}
    };

    if (argc > 1) {
        std::get<0>(editor) = file_load(argv[1]);
    }

    assert(tty_enable_raw_mode() == 0);

    while (true) {
       editor = next_frame(std::move(std::get<0>(editor)),
                           std::move(std::get<1>(editor)),
                           std::move(std::get<2>(editor)));
    }

    return 0;
}

