#include "tokenize.h"
#include <string.h>

/*!include:re2c "syntax/c.re2c" */

void tokenize(const char* YYCURSOR, char* color) {
    const char* YYMARKER;
    const char* cursor_start = YYCURSOR;
    for (;;) {
        const char* token_start = YYCURSOR;
        char* color_loc = color + (token_start - cursor_start);
        /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;

        end = "\x00";
        end { return; }
        *   { continue; }

        c_comment {
            memset(color_loc, TOKEN_COMMENT, YYCURSOR - token_start);
            continue;
        }
        c_literal {
            memset(color_loc, TOKEN_LITERAL, YYCURSOR - token_start);
            continue;
        }
        c_type {
            memset(color_loc, TOKEN_TYPE, YYCURSOR - token_start);
            continue;
        }
        c_keyword {
            memset(color_loc, TOKEN_KEYWORD, YYCURSOR - token_start);
            continue;
        }
        c_identifier {
            memset(color_loc, TOKEN_IDENTIFIER, YYCURSOR - token_start);
            continue;
        }
        */
    }
}
