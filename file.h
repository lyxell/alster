#pragma once
#include "buffer.h"

buffer file_load(const char*);
void file_save(const char*, const buffer&);
