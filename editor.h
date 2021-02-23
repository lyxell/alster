#pragma once

#include <optional>
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
    std::optional<buffer_position> visual_marker;
};

editor editor_handle_command(editor e);
window editor_draw(const editor& e, window win);
