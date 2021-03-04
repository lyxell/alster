#pragma once
#include "buffer.h"
#include <vector>

std::vector<buffer_line> file_load(const char*);
void file_save(const char*, const std::vector<buffer_line>&);
