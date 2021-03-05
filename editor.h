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
    std::string cmd;
    long int mode;
    char status[120];
    buffer_position pos;
    std::vector<buffer_line> lines;
    bool exiting;
    bool saving;
    size_t scroll;
    const char* filename;
};

editor editor_handle_command(editor e);
window editor_draw(editor& e);
