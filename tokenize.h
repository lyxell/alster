#pragma once

enum {
    TOKEN_NONE,
    TOKEN_COMMENT,
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_LITERAL,
    TOKEN_TYPE,
};

void tokenize(const char32_t*);
