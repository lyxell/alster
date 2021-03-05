#include <cassert>
#include <string>
#include "syntax/syntax.h"

token_collection tokenize_c(const char32_t* str) {
    const char32_t* YYCURSOR = str;
    const char32_t* YYMARKER;
    token_collection tokens;
    while (1) {
        const char32_t* YYSTART = YYCURSOR;
        /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = char32_t;

        ([LUu] | [u][8])? ["] [^"\n\x00]* ["] {
            auto res = tokenize_c_string(YYSTART);
            std::copy(res.begin(), res.end(),
                      std::back_inserter(tokens));
            continue;
        }

        [1-9][0-9]* {
            tokens.emplace_back(YYSTART, YYCURSOR, C_LITERAL_DECIMAL);
            continue;
        }

        [0][0-7]* {
            tokens.emplace_back(YYSTART, YYCURSOR, C_LITERAL_OCTAL);
            continue;
        }

        [0-9]+ {
            tokens.emplace_back(YYSTART, YYCURSOR, C_INVALID);
            continue;
        }

        "true" | "false" {
            tokens.emplace_back(YYSTART, YYCURSOR, C_LITERAL_BOOL);
            continue;
        }

        [/][/][^\x00]*[\x00] {
            tokens.emplace_back(YYSTART, YYCURSOR-1, C_SINGLE_LINE_COMMENT);
            return tokens;
        }

        ("FILE" | "bool" | "char" | "const" | "double" | "auto"
        | "float" | "int" | "size_t" | "void" | "struct" | "enum" | "char32_t")
        [*]* {
            tokens.emplace_back(YYSTART, YYCURSOR, C_TYPE);
            continue;
        }

        "break" | "continue" | "else" | "for" | "return" | "if" | "while" |
        "switch" | "case" | "default" | "using" {
            tokens.emplace_back(YYSTART, YYCURSOR, C_KEYWORD);
            continue;
        }

        "[" | "]" | "(" | ")" | "{" | "}" | "." |
        "->" | "++" | "--" | "&" | "*" | "+" | "-" |
        "~" | "!" | "/" | "%" | "<<" | ">>" | "<" | ">" |
        "<=" | ">=" | "==" | "!=" | "^" | "|" | "&&" | "||" |
        "?" | ":" | ";" | "..." | "=" | "*=" | "/=" | "%=" |
        "+=" | "-=" | "<<=" | ">>=" | "&=" | "^=" | "|=" |
        "," | "#" | "##" | "<:" | ":>" | "<%" | "%>" | "%:" | "%:%:"
        {
            tokens.emplace_back(YYSTART, YYCURSOR, C_PUNCTUATOR);
            continue;
        }

        [*]*[a-zA-Z_][a-zA-Z_0-9]* {
            tokens.emplace_back(YYSTART, YYCURSOR, C_IDENTIFIER);
            continue;
        }

        "\x00"
        {
            return tokens;
        }

        *
        {
            assert(YYCURSOR - YYSTART == 1);
            tokens.emplace_back(YYSTART, YYCURSOR, C_INVALID);
            continue;
        }
        */
    }
    return tokens;
}
