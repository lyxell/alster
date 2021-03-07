#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdlib>

struct buffer_position {
    size_t x;
    size_t y;
};

using buffer_char = char;
using buffer_line = std::string;
