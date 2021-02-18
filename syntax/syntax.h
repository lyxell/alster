#pragma once

#include <vector>
#include <tuple>

enum {
    C_STRING_ENCODING_PREFIX,
    C_STRING_OPENING_QUOTE,
    C_STRING_CLOSING_QUOTE,
    C_STRING_CHAR,
    C_STRING_ESCAPE_SEQUENCE,
    C_ERROR,
    C_IDENTIFIER,
    C_SINGLE_LINE_COMMENT,
    C_KEYWORD,
    C_TYPE,
    C_LITERAL_BOOL,
    C_LITERAL_DECIMAL,
    C_LITERAL_OCTAL
};

using token_collection = std::vector<std::tuple<const char32_t*, const char32_t*, int>>;

token_collection tokenize_c_string(const char32_t*);
token_collection tokenize_c(const char32_t*);
