#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <memory>

std::wstring MB2W(const char* in, size_t length);
std::wstring MB2W(const char* in);
std::unique_ptr<char> W2MB(std::wstring const& in);

#endif
