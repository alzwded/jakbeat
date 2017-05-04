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
#include <cassert>
#include <cstring>
#include <cstdarg>
#include <cstdio>

int Logger::level_ = 0;

Logger::Logger(const char* name)
    : name_(strdup(name ? name : "the unknown"))
{
    std::string padding((size_t)(level_ * 2), ' ');
    fprintf(stderr, "%sEntering %s\n", padding.c_str(), name);
    ++level_;
}

Logger::~Logger()
{
    --level_;
    std::string padding((size_t)(level_ * 2), ' ');
    fprintf(stderr, "%sLeaving %s\n", padding.c_str(), name_);
    free(name_);
}

int Logger::operator()(const char* fmt, ...)
{
    va_list p;
    va_start(p, fmt);
    std::string padding((size_t)(level_ * 2), ' ');
    char* indented = (char*)malloc(padding.size() + strlen(fmt) + 1);
    sprintf(indented, "%s%s", padding.c_str(), fmt);
    int rval = vfprintf(stderr, indented, p);
    free(indented);
    va_end(p);
    return rval;
}
