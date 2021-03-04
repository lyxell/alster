#include <fstream>
#include <streambuf>

#include "file.h"
#include "utf8.h"

buffer file_load(const char* filename) {
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
    return {std::move(lines), {0, 0}};
}

void file_save(const char* filename, const buffer& buf) {
    std::ofstream out(filename);
    for (auto line : buf.lines) {
        out << utf8_encode(line);
        out << '\n';
    }
    out.close();
}
