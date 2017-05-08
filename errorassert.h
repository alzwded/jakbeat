/*
Copyright (c) 2014-2017, Vlad Mesco
All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ERRORASSERT_H
#define ERRORASSERT_H

#include <cstdarg>
#include <cstdlib>
#include <string>
#include <sstream>
#include <cwchar>

#ifdef _MSC_VER
# define __func__ __FUNCTION__
#endif

#ifdef __GNUC__
# define ASSERT(X, ...) do{ if(!(X)) error_assert(__FILE__, __LINE__, __func__, #X, ##__VA_ARGS__); }while(0)
#else
# define ASSERT(X, ...) do{ if(!(X)) error_assert(__FILE__, __LINE__, __func__, #X, __VA_ARGS__); }while(0)
#endif

template<typename T, typename... ARGS>
std::wstring error_message(T head, ARGS... tail);

inline std::wstring error_message(void)
{
    return L"";
}

template<typename T, typename... Y>
std::wstring error_message(T head, Y... tail)
{
    std::wstringstream s;
    s << head << error_message(tail...);
    return s.str();
}

template<typename... T>
void error_assert(char const* file, int line, char const* func, char const* assertion, T... args)
{
    fwprintf(stderr, L"Assertion failed in %s:%d:%s: %s\n",
            file, line, func, assertion);
    auto msg = error_message(args...);
    if(!msg.empty()) {
        fwprintf(stderr, L"%ls\n", msg.c_str());
    }

    exit(2);
}

#endif
