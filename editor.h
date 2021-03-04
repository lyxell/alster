#pragma once

#include <optional>
#include <set>
#include "buffer.h"
#include "window.h"

enum {
    MODE_NORMAL,
    MODE_INSERT
};

struct editor {
    std::u32string cmd;
    long int mode;
    char status[120];
    buffer buf;
    bool exiting;
    bool saving;
    size_t scroll;
    const char* filename;
    std::set<std::u32string> bindings_normal;
    std::set<std::u32string> bindings_insert;
};

editor editor_handle_command(editor e);
window editor_draw(editor& e, window win);
