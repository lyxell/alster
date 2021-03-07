#ifndef SYNTAX_H
#define SYNTAX_H

#ifdef __cplusplus
extern "C" {
#endif
enum {
    C_STRING,
    C_INVALID,
    C_IDENTIFIER,
    C_SINGLE_LINE_COMMENT,
    C_KEYWORD,
    C_TYPE,
    C_LITERAL_BOOL,
    C_LITERAL_DECIMAL,
    C_LITERAL_OCTAL,
    C_PUNCTUATOR
};

struct token {
    const char* start;
    const char* end;
    int type;
};

struct token tokenize_c(const char*);

#ifdef __cplusplus
}
#endif

#endif
