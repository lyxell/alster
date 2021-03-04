#include <fstream>
#include <streambuf>

#include "file.h"
#include "utf8.h"

std::vector<buffer_line> file_load(const char* filename) {
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
    std::vector<buffer_line> lines;
    buffer_line line;
    for (char32_t c : utf8_decode(str)) {
        if (c == '\n') {
            lines.push_back(std::move(line));
            line.clear();
        } else {
            line.push_back(c);
        }
    }
    return lines;
}

void file_save(const char* filename, const std::vector<buffer_line>& lines) {
    std::ofstream out(filename);
    for (auto line : lines) {
        out << utf8_encode(line);
        out << '\n';
    }
    out.close();
}
