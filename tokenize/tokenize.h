#pragma once

enum {
    TOKEN_NONE,
    TOKEN_IDENTIFIER,
    TOKEN_BOOL_LIT,
    TOKEN_STRING_LIT,
    TOKEN_KEYWORD,
    TOKEN_INT_LIT,
    TOKEN_TYPE_QUALIFIER,
    TOKEN_COMMENT,
    TOKEN_TYPE,
    TOKEN_PREPROC
};

void tokenize_c(const char*, char*);
