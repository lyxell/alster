#include "file.h"
#include <fstream>
#include <streambuf>

buffer
file_load(char* filename) {
    buffer b = {
        {std::make_shared<buffer_line>()},
        {0, 0}
    };
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
    for (auto c : str) {
        if (c == '\n') {
            b = buffer_break_line(b);
        } else {
            b = buffer_insert(b, c, 1);
        }
    }
    b = buffer_move_start(b);
    return b;
}
