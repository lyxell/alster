#include "editor.h"
#include "window.h"

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

