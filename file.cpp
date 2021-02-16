#include "file.h"
#include <fstream>
#include <streambuf>
#include <locale>
#include <stdio.h>
#include <codecvt>

buffer
file_load(const char* filename) {
    buffer b = {
        {std::make_shared<buffer_line>()},
        {0, 0}
    };
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    std::u32string utf32 = cvt.from_bytes(str);
    for (char32_t c : utf32) {
        if (c == '\n') {
            b = buffer_break_line(b);
        } else {
            b = buffer_insert(b, c, 1);
        }
    }
    b = buffer_move_start(b);
    return b;
}

void
file_save(const char* filename, const buffer& b) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    std::string utf8;
    for (auto line : b.first) {
        auto bytes = cvt.to_bytes(std::u32string(line->begin(), line->end()));
        for (auto b : bytes)
            utf8.push_back(b);
        utf8.push_back('\n');
    }
    std::ofstream out(filename);
    out << utf8;
    out.close();
}
