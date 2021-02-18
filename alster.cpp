#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>

#include "tty.h"
#include "window.h"
#include "buffer.h"
#include "tokenize.h"
#include "file.h"

const char* output = "test";
const char* MESSAGE_COMMAND_NOT_FOUND = "No such command.";

enum {
    MODE_NORMAL,
    MODE_INSERT
};

struct state {
    int mode;
    bool exit;
    bool undo;
    bool redo;
    const char* status;
    window win;
};

using editor = std::pair<buffer, state>;

editor handle_input(buffer b, state s, std::vector<editor>& history,
        std::vector<editor>& future, std::istream& in) {
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
        "$"  {return {buffer_move_end_of_line(std::move(lines), pos), s};}
        "0"  {return {buffer_move_start_of_line(std::move(lines), pos), s};}
        "A"  {history.push_back({{lines, pos}, s});
              s.mode = MODE_INSERT;
              future.clear();
              return {buffer_move_end_of_line(std::move(lines), pos), s};}
        "G"  {return {buffer_move_end(std::move(lines), pos), s};}
        "dd" {history.push_back({{lines, pos}, s});
              future.clear();
              return {buffer_erase_current_line(std::move(lines), pos), s};}
        "gg" {return {buffer_move_start(std::move(lines), pos), s};}
        "h"  {return {buffer_move_left(std::move(lines), pos, 1), s};}
        "i"  {history.push_back({{lines, pos}, s});
              s.mode = MODE_INSERT;
              future.clear();
              return {{std::move(lines), pos}, s};}
        "j"  {return {buffer_move_down(std::move(lines), pos, 1), s};}
        "k"  {return {buffer_move_up(std::move(lines), pos, 1), s};}
        "l"  {return {buffer_move_right(std::move(lines), pos, 1), s};}
        "u"  {s.undo = true;
              return {{std::move(lines), pos}, s};}
        "r"  {s.redo = true;
              return {{std::move(lines), pos}, s};}
        "q"  {s.exit = true;
              return {{std::move(lines), pos}, s};}
        "s"  {file_save(output, b); return {{std::move(lines), pos}, s};}
        *    {s.status = MESSAGE_COMMAND_NOT_FOUND;
              return {{std::move(lines), pos}, s};}
        */
    } else if (s.mode == MODE_INSERT) {
        /*!re2c
        del  {return {buffer_erase(std::move(lines), pos), s};}
        esc  {s.mode = MODE_NORMAL;
              return {buffer_move_left(std::move(lines), pos, 1), s};}
        ret  {return {buffer_break_line(std::move(lines), pos), s};}
        tab  {return {buffer_insert(std::move(lines), pos, ' ', 4), s};}
        nul  {return {{std::move(lines), pos}, s};}
        *    {return {buffer_insert(std::move(lines), pos, yych, 1), s};}
        */
    }
    return {b, s};
}

int main(int argc, char* argv[]) {

    std::vector<editor> history;
    std::vector<editor> future;
    editor ed = {
        {{std::make_shared<buffer_line>()},{0, 0}},
        {}
    };
    auto& [b, s] = ed;
    if (argc > 1) {
        b = file_load(argv[1]);
    }

    assert(tty_enable_raw_mode() == 0);

    while (true) {

        /* render main window */
        s.win = window_update_size(s.win);
        s.win = window_update_scroll(ed.first, s.win);
        window_render(b, s.win);

        /* print statusline, if any */
        if (s.status) {
            printf("\033[%ld;%ldH%s\033[K", s.win.height, 0ul, s.status);
            s.status = NULL;
        }

        window_render_cursor(b, s.win);

        /* handle user input */
        ed = handle_input(std::move(b), s, history, future, std::cin);

        if (s.exit) {
            break;
        }

        /* handle undo */
        if (s.undo) {
            s.undo = false;
            if (history.empty()) {
                s.status = "History empty!";
            } else {
                future.push_back(std::move(ed));
                ed = std::move(history.back());
                history.pop_back();
            }
        }

        /* handle redo */
        if (s.redo) {
            s.redo = false;
            if (future.empty()) {
                s.status = "Future empty!";
            } else {
                history.push_back(std::move(ed));
                ed = std::move(future.back());
                future.pop_back();
            }
        }

    }
    return 0;
}

