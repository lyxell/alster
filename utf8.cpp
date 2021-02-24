#include "utf8.h"
#include <string.h>

std::string utf8_encode(const std::u32string& source)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
	return cvt.to_bytes(source);
}

std::u32string utf8_decode(const std::string& source)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    return cvt.from_bytes(source);
}

char32_t utf8_getchar() {
    std::string input;
    input.push_back(char(getchar()));
    while (1) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
        std::u32string res = cvt.from_bytes(input);
        if (cvt.converted() == input.size()) return res[0];
        input.push_back(char(getchar()));
    }
    return '\0';
}
