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
    int mode;
    char status[120];
    buffer buf;
    std::vector<buffer> history;
    std::vector<buffer> future;
    bool exiting;
    bool saving;
    size_t scroll;
    const char* filename;
    std::optional<buffer_position> visual_marker;
    std::set<std::u32string> bindings_normal;
    std::set<std::u32string> bindings_insert;
    std::optional<std::u32string> lua_function;
};

editor editor_handle_command(editor e);
window editor_draw(editor& e, window win);
