#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <memory>

std::wstring MB2W(const char* in, size_t length);
std::wstring MB2W(const char* in);
std::unique_ptr<char> W2MB(std::wstring const& in);
#ifdef _MSC_VER
# define wfopen _wfopen
#else
# include <cstdio>
FILE* wfopen(const wchar_t*, const wchar_t*);
#endif

#endif
