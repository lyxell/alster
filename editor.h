#pragma once

#include "buffer.h"
#include "window.h"
#include <vector>
#include <string>

enum {
    MODE_NORMAL,
    MODE_INSERT
};

struct editor {
    char cmd[64];
    long int mode;
    buffer_position pos;
    std::vector<std::string> lines;
    bool exiting;
    bool saving;
    size_t scroll;
    const char* filename;
};

editor editor_handle_command(editor e);
window editor_draw(editor& e);
