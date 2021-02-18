#include <iostream>
#include "../utf8.h"
#include "../syntax/syntax.h"
#include "colors.h"

int main() {
    auto v = tokenize_c(U"u\"hello\\nworld\"");
    for (auto [s, e, t] : v) {
        switch (t) {
            case C_STRING_CHAR:
                set_color(COLOR_CYAN);
                break;
            case C_STRING_ESCAPE_SEQUENCE:
                set_color(COLOR_GREEN);
                break;
            case C_STRING_ENCODING_PREFIX:
            case C_STRING_OPENING_QUOTE:
            case C_STRING_CLOSING_QUOTE:
                set_color(COLOR_BLUE);
                break;
            default:
                set_color(COLOR_RESET);
                break;
        }
        std::cout << utf8_encode(std::u32string(s, e));
    }
    std::cout << std::endl;
}
