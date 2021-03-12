#include "syntax/syntax.h"
#include <assert.h>
#include <stdbool.h>

struct token tokenize_c(const char* str) {
    const char* YYCURSOR = str;
    const char* YYMARKER;
    while (1) {
        const char* YYSTART = YYCURSOR;
        /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = char;
        re2c:flags:case-ranges = 1;

        ([LUu] | [u][8])? ["] [^"\n\x00]* ["] {
            return (struct token) {YYSTART, YYCURSOR, C_STRING};
        }

        [1-9][0-9]* {
            return (struct token) {YYSTART, YYCURSOR, C_LITERAL_DECIMAL};
        }

        [0][0-7]* {
            return (struct token) {YYSTART, YYCURSOR, C_LITERAL_OCTAL};
        }

        [0-9]+ {
            return (struct token) {YYSTART, YYCURSOR, C_INVALID};
        }

        "true" | "false" {
            return (struct token) {YYSTART, YYCURSOR, C_LITERAL_BOOL};
        }

        [/][/][^\x00]*[\x00] {
            return (struct token) {YYSTART, YYCURSOR - 1, C_SINGLE_LINE_COMMENT};
        }

        ("FILE" | "bool" | "char" | "const" | "double" | "auto"
        | "float" | "int" | "size_t" | "void" | "struct" | "enum" | "char32_t")
        [*]* {
            return (struct token) {YYSTART, YYCURSOR, C_TYPE};
        }

        "break" | "continue" | "else" | "for" | "return" | "if" | "while" |
        "switch" | "case" | "default" | "using" {
            return (struct token) {YYSTART, YYCURSOR, C_KEYWORD};
        }

        "[" | "]" | "(" | ")" | "{" | "}" | "." |
        "->" | "++" | "--" | "&" | "*" | "+" | "-" |
        "~" | "!" | "/" | "%" | "<<" | ">>" | "<" | ">" |
        "<=" | ">=" | "==" | "!=" | "^" | "|" | "&&" | "||" |
        "?" | ":" | ";" | "..." | "=" | "*=" | "/=" | "%=" |
        "+=" | "-=" | "<<=" | ">>=" | "&=" | "^=" | "|=" |
        "," | "#" | "##" | "<:" | ":>" | "<%" | "%>" | "%:" | "%:%:"
        {
            return (struct token) {YYSTART, YYCURSOR, C_PUNCTUATOR};
        }

        [*]*[a-zA-Z_][a-zA-Z_0-9]* {
            return (struct token) {YYSTART, YYCURSOR, C_IDENTIFIER};
        }

        "\x00"
        {
            return (struct token) {YYSTART, YYSTART, C_INVALID};
        }

        *
        {
            assert(YYCURSOR - YYSTART == 1);
            return (struct token) {YYSTART, YYCURSOR, C_INVALID};
            continue;
        }
        */
    }
    assert(false);
    return (struct token) {0};
}
