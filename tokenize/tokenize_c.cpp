#include "tokenize.h"
#include <string.h>

void tokenize_c(const char* YYCURSOR, char* color) {
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
            
            // comments
    
            ("/*" ([^*] | ("*" [^/]))* "*""/")      |
            ("//" [^\n]* "\n")

            {
                memset(color_loc, TOKEN_COMMENT, YYCURSOR - token_start);
                continue;
            }
            
            // string literal
            
            "\"" [^"\n]* "\""

            {
                memset(color_loc, TOKEN_STRING_LIT, YYCURSOR - token_start);
                continue;
            }

            // type qualifier

            "const"

            {
                memset(color_loc, TOKEN_TYPE_QUALIFIER, YYCURSOR - token_start);
                continue;
            }
            
            // types

            types = "FILE"      |
                    "bool"      |
                    "char"      |
                    "double"    |
                    "float"     |
                    "size_t"    |
                    "int"       |
                    "void";

            types "*"*

            {
                memset(color_loc, TOKEN_TYPE, YYCURSOR - token_start);
                continue;
            }

            // keywords

            "break"     |
            "case"      |
            "continue"  |
            "default"   |
            "else"      |
            "enum"      |
            "for"       |
            "if"        |
            "return"    |
            "struct"    |
            "switch"    |
            "while"
            
            {
                memset(color_loc, TOKEN_KEYWORD, YYCURSOR - token_start);
                continue;
            }
            
            // preprocessor

            ("#" | "%:") ([^\n] | "\\\n")* "\n" {
                memset(color_loc, TOKEN_PREPROC, YYCURSOR - token_start);
                continue;
            }

            // boolean literals

            "false" |
            "true"

            {
                memset(color_loc, TOKEN_BOOL_LIT, YYCURSOR - token_start);
                continue;
            }

            // integer literal
            
            "0" | [1-9][0-9]*

            {
                memset(color_loc, TOKEN_INT_LIT, YYCURSOR - token_start);
                continue;
            }

            // identifiers

            [a-zA-Z_][a-zA-Z_0-9]*

            {
                memset(color_loc, TOKEN_IDENTIFIER, YYCURSOR - token_start);
                continue;
            }

        */
    }
}
