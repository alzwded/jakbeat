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

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <cwchar>

struct Logger
{
    Logger(const char* name);
    Logger(const wchar_t* name);
    ~Logger();
    int operator()(const wchar_t* fmt, ...);
private:
    static int level_;
    std::wstring name_;
};

#define LOGGERQ(X) X
#define LOGGER5(title, name)\
    Logger name(title)
#ifdef __GNUC__
# define LOGGER4(FN, LN, name) LOGGER5((std::string(FN) +  "@" LN).c_str(), name)
# define LOGGER31(FN, LN, name) LOGGER4(FN, LOGGERQ(#LN), name)
# define LOGGER3(FN, LN, name) LOGGER31(FN, LN, name)
# define LOGGER2(FN, LN, name) LOGGER3(FN, LOGGERQ(LN), name)
# define LOGGER(name) LOGGER2(__PRETTY_FUNCTION__, __LINE__, name)
#elif defined(_MSC_VER)
# define LOGGER4(FL, FN, LN, name) LOGGER5((std::string(FL) + std::string(FN) +  "@" LN).c_str(), name)
# define LOGGER31(FL, FN, LN, name) LOGGER4(FL, FN, LOGGERQ(#LN), name)
# define LOGGER3(FL, FN, LN, name) LOGGER31(FL, FN, LN, name)
# define LOGGER2(FL, FN, LN, name) LOGGER3(FL, FN, LOGGERQ(LN), name)
# define LOGGER(name) LOGGER2(__FILE__, __func__, __LINE__, name)
#endif

#endif
