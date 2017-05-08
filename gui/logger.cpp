/*
Copyright (c) 2017, Vlad Meșco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "logger.h"
#include "string_utils.h"
#include <cassert>
#include <cstring>
#include <cstdarg>
#include <cstdio>

int Logger::level_ = 0;

Logger::Logger(const char* name)
    : name_(MB2W(name ? name : "the unknown"))
{
    std::wstring padding((size_t)(level_ * 2), L' ');
    fwprintf(stderr, L"%lsEntering %ls\n", padding.c_str(), name_.c_str());
    ++level_;
}

Logger::Logger(const wchar_t* name)
    : name_(name ? name : L"the unknown")
{
    std::wstring padding((size_t)(level_ * 2), L' ');
    fwprintf(stderr, L"%lsEntering %ls\n", padding.c_str(), name_.c_str());
    ++level_;
}

Logger::~Logger()
{
    --level_;
    std::wstring padding((size_t)(level_ * 2), L' ');
    fwprintf(stderr, L"%lsLeaving %ls\n", padding.c_str(), name_.c_str());
}

int Logger::operator()(const wchar_t* fmt, ...)
{
    va_list p;
    va_start(p, fmt);
    std::wstring padding((size_t)(level_ * 2), L' ');
    size_t indentedLen = (padding.size() + wcslen(fmt) + 1);
    wchar_t* indented = (wchar_t*)malloc(indentedLen * sizeof(wchar_t));
    swprintf(indented, indentedLen, L"%ls%ls", padding.c_str(), fmt);
    int rval = vfwprintf(stderr, indented, p);
    free(indented);
    va_end(p);
    return rval;
}
