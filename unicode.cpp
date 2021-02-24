#include <locale>
#include <cassert>

/*!include:re2c "re2c/includes.re" */

bool is_regex_word(char32_t c) {
    const char32_t* YYCURSOR = &c;
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:flags:unicode = 1;
    re2c:define:YYCTYPE = char32_t;
    regex_word {
        return true;
    }
    * {
        return false;
    }
    */
    assert(false);
    return true;
}

bool is_regex_space(char32_t c) {
    const char32_t* YYCURSOR = &c;
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:flags:unicode = 1;
    re2c:define:YYCTYPE = char32_t;
    regex_space {
        return true;
    }
    * {
        return false;
    }
    */
    assert(false);
    return true;
}
