#include "editor.h"
#include "window.h"

// TODO: editor should not be modified
window editor_draw(editor& e, window old) {
    window win;
    win = window_update_size(win);
    if (e.pos.y - 1 < e.scroll) {
        e.scroll = e.pos.y - 1;
    } else if (e.pos.y - 1 >= e.scroll + win.height) {
        e.scroll = e.pos.y - win.height;
    }
    window_update_cursor(&win, e.lines, e.pos, e.scroll);
    window_render_buffer(&win, e.lines, e.scroll);
    if (e.mode == MODE_INSERT) {
        printf("\x1b[6 q");
    } else {
        printf("\x1b[2 q");
    }
    static char buffer[30000];
    window_to_string(&win, buffer);
    printf("%s", buffer);
    return win;
}

