#include "string_utils.h"

#include <cwchar>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <memory>

#ifdef _MSC_VER
# define WIN32_LEAN_AND_MEAN
# define VC_EXTRALEAN
# include <windows.h>
// TODO use the functions below instead of the CRT versions (which break
//     on windows because msvcrt refuses any UTF-8 locale)
//     Using these should be enough to support unicode; fltk should accept
//     utf8 for its own internal use, and I use wide strings which are
//     OK on windows and seem to be okay on linux)
//
//     WideCharToMultiByte(CP_UTF8, 0, in.c_str(), in.size(), nullptr, &outLen, nullptr, nullptr);
//     WideCharToMultiByte(CP_UTF8, 0, in.c_str(), in.size(), out, &outLen, nullptr, nullptr);
//     MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, in, length, nullptr, &outLen, nullptr, nullptr);
//     MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, in, length, out, &outLen, nullptr, nullptr);
#endif

std::wstring MB2W(const char* in, size_t length)
{
    std::unique_ptr<char, std::default_delete<char[]>> store(new char[length + 1]);
    strncpy(store.get(), in, length);
    store.get()[length] = '\0';
    const char* s = store.get();
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));
#ifdef _MSC_VER
    int len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, in, length, nullptr, 0);
#else
    size_t len = mbsrtowcs(nullptr, &s, 0, &ps);
#endif
    if(len <= 0) return {};

    std::unique_ptr<wchar_t, std::default_delete<wchar_t[]>> ws(new wchar_t[len + 1]);
#ifdef _MSC_VER
    int written = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, in, length, ws.get(), len + 1);
#else
    size_t written = mbsrtowcs(ws.get(), &s, len + 1, &ps);
#endif
    if(written != len) return {};

    return std::wstring(ws.get());
}

std::wstring MB2W(const char* in)
{
    return MB2W(in, strlen(in));
}

std::unique_ptr<char, std::default_delete<char[]>> W2MB(std::wstring const& in)
{
    const wchar_t* s = in.c_str();
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));
#ifdef _MSC_VER
    int len = WideCharToMultiByte(CP_UTF8, 0, s, in.size(), nullptr, 0, nullptr, nullptr);
#else
    int len = wcsrtombs(nullptr, &s, 0, &ps);
#endif
    if(len <= 0) return {};

    std::unique_ptr<char, std::default_delete<char[]>> mbs(new char[len + 1]);
#ifdef _MSC_VER
    int written = WideCharToMultiByte(CP_UTF8, 0, s, in.size(), mbs.get(), len + 1, nullptr, nullptr);
#else
    size_t written = wcsrtombs(mbs.get(), &s, len + 1, &ps);
#endif
    if(written != len) return {};

    return std::move(mbs);
}

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>

FILE* open_read_unicode(const wchar_t* path)
{
    return _wfopen(path, L"rt, ccs=UTF-8");
}
FILE* reopen_read_unicode(FILE* f)
{
    int hr = _setmode( _fileno(f), _O_U8TEXT);
    return f;
}
FILE* open_write_binary(const wchar_t* path)
{
    return _wfopen(path, L"wb");
}
FILE* open_write_unicode(const wchar_t* path)
{
    return _wfopen(path, L"wt, ccs=UTF-8");
}
#else
FILE* open_read_unicode(const wchar_t* path)
{
    return fopen(W2MB(path).get(), "r");
}
FILE* reopen_read_unicode(FILE* f)
{
    return f;
}
FILE* open_write_binary(const wchar_t* path)
{
    return fopen(W2MB(path).get(), "wb");
}
FILE* open_write_unicode(const wchar_t* path)
{
    return fopen(W2MB(path).get(), "w");
}
#endif
