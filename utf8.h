#pragma once
#include <locale>
#include <codecvt>

std::string utf8_encode(const std::u32string& source);
std::u32string utf8_decode(const std::string& source);

std::string utf8_getchar();
