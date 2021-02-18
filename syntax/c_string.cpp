#include <cassert>
#include "syntax.h"

/**
 * Expects a null terminated string.
 */
token_collection
tokenize_c_string(const char32_t* str) {
    const char32_t* YYCURSOR = str;
    const char32_t* YYMARKER;
    token_collection tokens;
/*!re2c
    re2c:yyfill:enable = 0;
    re2c:define:YYCTYPE = char32_t;
    [uUL] "\"" {
        tokens.emplace_back(str, str + 1, C_STRING_ENCODING_PREFIX);
        tokens.emplace_back(str + 1, str + 2, C_STRING_OPENING_QUOTE);
        goto content;
    }
    "u8\"" {
        tokens.emplace_back(str, str + 2, C_STRING_ENCODING_PREFIX);
        tokens.emplace_back(str + 2, str + 3, C_STRING_OPENING_QUOTE);
        goto content;
    }
    "\"" {
        tokens.emplace_back(str, str + 1, C_STRING_OPENING_QUOTE);
        goto content;
    }
    * {
        assert(false);
    }
*/
content:
    while (1) {
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = char32_t;
        "\"" {
            tokens.emplace_back(YYCURSOR - 1, YYCURSOR,
                                C_STRING_CLOSING_QUOTE);
            return tokens;
        }
        [^\n\\] {
            tokens.emplace_back(YYCURSOR - 1, YYCURSOR, C_STRING_CHAR);
            continue;
        }
        "\\a" | "\\b" | "\\f" | "\\n" | "\\r" | "\\t" | "\\v" {
            tokens.emplace_back(YYCURSOR - 2, YYCURSOR,
                                C_STRING_ESCAPE_SEQUENCE);
            continue;
        }
        * {
            assert(false);
        }
    */
    }
    return tokens;
}
