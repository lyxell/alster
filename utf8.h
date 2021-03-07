#pragma once
#include <locale>
#include <codecvt>

std::u32string utf8_decode(const std::string& source);

std::string utf8_getchar();
