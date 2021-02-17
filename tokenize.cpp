#include "tokenize.h"
#include <string.h>
#include "utf8.h"

#define COLOR_RED "\033[31m"
#define COLOR_BLUE "\033[34m"
#define COLOR_GREY "\033[90m"
#define COLOR_RESET "\033[0m"

/*!include:re2c "syntax/c.re2c" */

void print_encoded(const std::u32string& str) {
    printf(utf8_encode(str).c_str());
}

void tokenize(const char32_t* YYCURSOR) {
    const char32_t* YYMARKER;
    size_t y = 1;
    size_t x = 1;
    printf("\033[%ld;%ldH\033[K", y, x);
    const char32_t* ptr = YYCURSOR;
    for (;;) {
        int token;
        /*!re2c
        re2c:define:YYCTYPE = char32_t;
        re2c:yyfill:enable = 0;
        re2c:flags:8 = 0;
        re2c:flags:u = 1;

        end = "\x00";

        end          { return; }
        "\n"         { token = TOKEN_NONE; goto render; }
        *            { token = TOKEN_NONE; goto render; }
        c_literal    { token = TOKEN_LITERAL; goto render; }
        c_type       { token = TOKEN_TYPE; goto render; }
        c_keyword    { token = TOKEN_KEYWORD; goto render; }
        c_identifier { token = TOKEN_IDENTIFIER; goto render; }
        */
render:
        switch (token) {
            case TOKEN_COMMENT:
                printf(COLOR_GREY);
                break;
            case TOKEN_TYPE:
                printf(COLOR_BLUE);
                break;
            default:
                printf(COLOR_RESET);
                break;
        }
        while (ptr < YYCURSOR) {
            if (*ptr == '\n') {
                y++;
                x = 1;
                if (y < 23)
                    printf("\033[%ld;%ldH\033[K", y, x);
            } else {
                if (y < 23)
                    print_encoded(std::u32string(ptr, ptr+1));
                x++;
            }
            ptr++; 
        }
    }
}
