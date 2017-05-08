/*
Copyright (c) 2016-2017, Vlad Mesco
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
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <errorassert.h>

#include <tokenizer.h>
#include <parser.h>
#include <parser_types.h>

int tokenizer_lineno = 1;

Tokenizer::Tokenizer(FILE* f_)
    : f(f_)
{
    if(feof(f)) c = WEOF;
    else c = fgetwc(f);
}

Token Tokenizer::operator()()
{
    while(iswspace(c) || c == L'\u000D' || c == L'\u000A')
    {
        if(c == L'\u000A') tokenizer_lineno++;
        c = fgetwc(f);
        if(feof(f)) return {TEOF};
    }
    if(c == L'[') { c = fgetwc(f); return {LSQUARE}; }
    if(c == L']') { c = fgetwc(f); return {RSQUARE}; }
    if(c == L'(') { c = fgetwc(f); return {LPAREN}; }
    if(c == L')') { c = fgetwc(f); return {RPAREN}; }
    if(c == L'=') { c = fgetwc(f); return {EQUALS}; }

    std::wstringstream ss;
    std::function<bool(wchar_t)> conditions[] = {
        [](wchar_t c) -> bool {
            return !(isspace(c)
                    || c == L'\u000A'
                    || c == L'\u000D'
                    || c == L'['
                    || c == L']'
                    || c == L'('
                    || c == L')'
                    || c == L'='
                    );
        },
        [](wchar_t c) -> bool {
            return c != L'"';
        },
    };
    bool quoted = (c == L'"');
    auto condition = conditions[quoted];
    if(quoted) c = fgetwc(f);
    while(condition(c))
    {
        if(c == L'\u000A') tokenizer_lineno++;
        ss << c;
        c = fgetwc(f);
        if(feof(f)) break;
    }
    if(quoted) c = fgetwc(f);
    //return {STRING, ss.str()}; // curly bracket form crashes msvc 18.00.21005.1
    return Token(STRING, ss.str());
}

