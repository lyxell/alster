#include "utf8.h"
#include <string.h>

static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;

std::string utf8_encode(const std::u32string& source)
{
	return cvt.to_bytes(source);
}

std::u32string utf8_decode(const std::string& source)
{
    return cvt.from_bytes(source);
}
