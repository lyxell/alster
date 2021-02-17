#include "file.h"
#include <fstream>
#include <streambuf>
#include <locale>
#include <stdio.h>
#include <codecvt>
#include "utf8.h"

buffer
file_load(const char* filename) {
    buffer b = {
        {std::make_shared<buffer_line>()},
        {0, 0}
    };
    printf("decoding...\n");
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
    auto u32s = utf8_decode(str);
    printf("decoding done...\n");
    printf("building buffer...\n");
    std::vector<std::shared_ptr<buffer_line>> lines;
    buffer_line line;
    for (char32_t c : u32s) {
        if (c == '\n') {
            lines.push_back(std::make_shared<buffer_line>(line));
            line.clear();
        } else {
            line.push_back(c);
        }
    }
    b = buffer_move_start(b);
    printf("building buffer done...\n");
    return {lines, {0, 0}};
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
