#include "file.h"
#include <fstream>
#include <streambuf>
#include <locale>
#include <stdio.h>
#include <codecvt>
#include "utf8.h"

buffer file_load(const char* filename) {
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
    buffer_lines lines;
    buffer_line line;
    for (char32_t c : utf8_decode(str)) {
        if (c == '\n') {
            lines.push_back(std::make_shared<buffer_line>(std::move(line)));
            line.clear();
        } else {
            line.push_back(c);
        }
    }
    return {std::move(lines), {0, 0}};
}

void file_save(const char* filename, const buffer& b) {
    std::ofstream out(filename);
    for (auto line : b.first) {
        out << utf8_encode(*line);
        out << '\n';
    }
    out.close();
}
