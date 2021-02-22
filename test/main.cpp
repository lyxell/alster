#include <vector>
#include <iostream>
#include <cassert>

#include "../editor.h"
#include "../utf8.h"

std::u32string buffer_to_string(const buffer& b) {
    std::u32string out;
    for (auto line_ptr : b.lines) {
        out += *line_ptr;
        out += '\n';
    }
    out.pop_back();
    return out;
}

int main() {
    std::vector<std::pair<std::u32string, std::u32string>> test_cases = {
        // basic insertion, deletion
        {U"ihello", U"hello"},
        {U"ihello\edd", U""},
        // bracket balancing
        {U"i((", U"(())"},
        {U"i((\x7f\x7f", U""},
        {U"i(\x7f", U""},
        {U"ihello()", U"hello()"},
        // indentation
        {U"ihello\rworld", U"hello\nworld"},
        {U"ihello\rworld", U"hello\nworld"},
        {U"i{\r", U"{\n    \n}"},
        {U"i{\r{\r{\r", U"{\n    {\n        {\n            \n        }\n    }\n}"},
    };
    for (auto [l, r] : test_cases) {
        editor e {};
        e.buf = {{std::make_shared<buffer_line>()}, {0, 0}};
        e.mode = MODE_NORMAL;
        for (auto ch : l) {
            e.cmd.push_back(ch);
            e = editor_handle_command(std::move(e));
        }
        auto res = buffer_to_string(e.buf);
        if (res != r) {
            std::cout << "wrote: " << std::endl;
            std::cout<< utf8_encode(l) << std::endl;
            std::cout << "expected: " << std::endl;
            std::cout << utf8_encode(r) << std::endl;
            std::cout << "got: " << std::endl;
            std::cout << utf8_encode(res) << std::endl;
            return 1;
        }
    }
    return 0;
}
