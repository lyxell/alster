#include "editor.h"
#include "window.h"

// pure function
static editor editor_handle_command_normal(editor e) {
    if (e.bindings_normal.find(e.cmd) != e.bindings_normal.end()) {
        e.lua_function = e.cmd;
        e.cmd = {};
        return e;
    }
    // check if e.cmd is a prefix of some command
    if (e.bindings_normal.upper_bound(e.cmd) != e.bindings_normal.end()) {
        auto next = *e.bindings_normal.upper_bound(e.cmd);
        if (next.find(e.cmd) == 0) {
            return e;
        }
    }
    e.cmd = {};
    return e;
}

// pure function
static editor editor_handle_command_insert(editor e) {
    if (e.bindings_insert.find(e.cmd) != e.bindings_insert.end()) {
        e.lua_function = e.cmd;
        e.cmd = {};
        return e;
    }
    // check if e.cmd is a prefix of some command
    if (e.bindings_insert.upper_bound(e.cmd) != e.bindings_insert.end()) {
        auto next = *e.bindings_insert.upper_bound(e.cmd);
        if (next.find(e.cmd) == 0) {
            return e;
        }
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

