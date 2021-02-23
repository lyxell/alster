#include "editor.h"
#include "window.h"
#include "utf8.h"
#include <cstring>
#include <cassert>

// pure function
static editor editor_handle_command_normal(editor e) {
    const char32_t *YYCURSOR = e.cmd.c_str();
    const char32_t *YYMARKER;
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:flags:unicode = 1;
    re2c:define:YYCTYPE = char32_t;
    ([1-9][0-9]*)? ("h" | "j" | "k" | "l") {
        e.cmd = {};
        switch (yych) {
        case 'h': e.buf = buffer_move_left(std::move(e.buf),  1); break;
        case 'j': e.buf = buffer_move_down(std::move(e.buf),  1); break;
        case 'k': e.buf = buffer_move_up(std::move(e.buf),    1); break;
        case 'l': e.buf = buffer_move_right(std::move(e.buf), 1); break;
        }
        return e;
    }
    ([1-9][0-9]*)? ("u" | "r") {
        e.cmd = {};
        auto& pop_from = yych == 'u' ? e.history : e.future;
        auto& push_to = yych == 'u' ? e.future : e.history;
        if (pop_from.empty()) {
            sprintf(e.status, "%s empty!", yych == 'u' ? "History" : "Future");
        } else {
            push_to.push_back(std::move(e.buf));
            e.buf = std::move(pop_from.back());
            pop_from.pop_back();
        }
        return e;
    }
    "o" | "O" {
        e.cmd = {};
        e.history.push_back(e.buf);
        e.future.clear();
        if (yych == 'O') {
            e.buf = buffer_move_up(std::move(e.buf), 1);
        }
        e.buf = buffer_move_end_of_line(std::move(e.buf));
        e.buf = buffer_break_line(std::move(e.buf));
        e.mode = MODE_INSERT;
        return e;
    }
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
    "0" | "^" {
        e.cmd = {};
        e.buf = buffer_move_start_of_line(std::move(e.buf));
        return e;
    }
    "q" {
        e.cmd = {};
        e.exiting = true;
        return e;
    }
    "v" {
        e.cmd = {};
        e.visual_marker = e.buf.pos;
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
        // if yych == '\0' we have read until end, i.e.
        // the command must be a prefix of some command
        if (yych == '\0') {
            sprintf(e.status, "%s", utf8_encode(e.cmd.c_str()).c_str());
        } else {
            sprintf(e.status, "Unknown command %s",
                    utf8_encode(e.cmd.c_str()).c_str());
            e.cmd = {};
        }
        return e;
    }
    */
    assert(false);
    return e;
}

// pure function
static editor editor_handle_command_insert(editor e) {
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
    assert(false);
    return e;
}

// pure function
editor editor_handle_command(editor e) {
    if (e.mode == MODE_INSERT) {
        return editor_handle_command_insert(std::move(e));
    } else if (e.mode == MODE_NORMAL) {
        return editor_handle_command_normal(std::move(e));
    }
    assert(false);
    return e;
}

window editor_draw(const editor& e, window win) {
    win = window_update_size(win);
    win = window_update_scroll(e.buf, win);
    window_render(e.buf, win, e.visual_marker);
    if (strlen(e.status)) {
        printf("\x1b[%ld;%ldH%s\033[K", win.height, 0ul, e.status);
    }
    window_render_cursor(e.buf, win, e.mode == MODE_INSERT);
    return win;
}
