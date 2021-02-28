#include <cstring>
#include <cassert>

#include "editor.h"
#include "window.h"
#include "utf8.h"
#include "unicode.h"

// pure function
static editor editor_handle_command_normal(editor e) {
    if (e.bindings.find(e.cmd) != e.bindings.end()) {
        e.lua_function = e.cmd;
        e.cmd = {};
        return e;
    }
    // check if e.cmd is a prefix of some command
    if (e.bindings.upper_bound(e.cmd) != e.bindings.end()) {
        auto next = *e.bindings.upper_bound(e.cmd);
        if (next.find(e.cmd) == 0) {
            return e;
        }
    }
    e.cmd = {};
    return e;
}

// pure function
static editor editor_handle_command_insert(editor e) {
    if (e.cmd[0] == '\x7f' || e.cmd[0] == '\b') {
        e.cmd = {};
        e.future.clear();
        e.buf = buffer_erase(std::move(e.buf));
        return e;
    }
    if (e.cmd[0] == '\x1b') {
        e.cmd = {};
        e.mode = MODE_NORMAL;
        return e;
    }
    if (e.cmd[0] == '\r') {
        e.cmd = {};
        e.future.clear();
        e.buf = buffer_break_line(std::move(e.buf));
        return e;
    }
    if (e.cmd[0] == '\t') {
        e.cmd = {};
        e.future.clear();
        e.buf = buffer_indent(std::move(e.buf));
        return e;
    }
    e.buf = buffer_insert(std::move(e.buf), e.cmd[0]);
    e.cmd = {};
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

// TODO: editor should not be modified
window editor_draw(editor& e, window old) {
    window win;
    win = window_update_size(win);
    if (e.buf.pos.y < e.scroll) {
        e.scroll = e.buf.pos.y;
    } else if (e.buf.pos.y >= e.scroll + win.height) {
        e.scroll = e.buf.pos.y - win.height + 1;
    }
    win = window_update_cursor(win, e.buf, e.scroll);
    win = window_render_buffer(win, e.buf, e.scroll);
    if (e.visual_marker) {
        auto start = *e.visual_marker;
        auto end = e.buf.pos;
        if (start > end) std::swap(start, end);
        win = window_render_visual_selection(win, start, end, e.scroll);
    }
    if (e.mode == MODE_INSERT) {
        printf("\x1b[6 q");
    } else {
        printf("\x1b[2 q");
    }
    printf("%s", window_to_string(win).c_str());
    return win;
}
