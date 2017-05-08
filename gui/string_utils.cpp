#include "string_utils.h"

#include <cwchar>
#include <cstring>
#include <cstdlib>
#include <string>
#include <memory>

std::wstring MB2W(const char* in, size_t length)
{
    std::unique_ptr<char> store(new char[length + 1]);
    strncpy(store.get(), in, length);
    store.get()[length] = '\0';
    const char* s = store.get();
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));
    int len = mbsrtowcs(nullptr, &s, 0, &ps);
    if(len <= 0) return {};

    std::unique_ptr<wchar_t> ws(new wchar_t[len + 1]);
    size_t written = mbsrtowcs(ws.get(), &s, len + 1, &ps);
    if(written != len) return {};

    return std::wstring(ws.get());
}

std::wstring MB2W(const char* in)
{
    return MB2W(in, strlen(in));
}

std::unique_ptr<char> W2MB(std::wstring const& in)
{
    const wchar_t* s = in.c_str();
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));
    int len = wcsrtombs(nullptr, &s, 0, &ps);
    if(len <= 0) return {};

    std::unique_ptr<char> mbs(new char[len + 1]);
    size_t written = wcsrtombs(mbs.get(), &s, len + 1, &ps);
    if(written != len) return {};

    return std::move(mbs);
}
