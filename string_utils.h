#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <memory>
#include <cstdio>

std::wstring MB2W(const char* in, size_t length);
std::wstring MB2W(const char* in);
std::unique_ptr<char, std::default_delete<char[]>> W2MB(std::wstring const& in);

FILE* reopen_read_unicode(FILE*);
FILE* open_read_unicode(const wchar_t*);
FILE* open_write_binary(const wchar_t*);
FILE* open_write_unicode(const wchar_t*);
#define close_file(X) fclose((X));

#endif
