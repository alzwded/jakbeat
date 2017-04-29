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
    if(feof(f)) c = EOF;
    else c = fgetc(f);
}

Token Tokenizer::operator()()
{
    while(isspace(c) || c == 13 || c == 10)
    {
        c = fgetc(f);
        if(feof(f)) return {TEOF};
        if(c == 10) tokenizer_lineno++;
    }
    if(c == '[') { c = fgetc(f); return {LSQUARE}; }
    if(c == ']') { c = fgetc(f); return {RSQUARE}; }
    if(c == '(') { c = fgetc(f); return {LPAREN}; }
    if(c == ')') { c = fgetc(f); return {RPAREN}; }
    if(c == '=') { c = fgetc(f); return {EQUALS}; }

    std::stringstream ss;
    std::function<bool(char)> conditions[] = {
        [](char c) -> bool {
            return !(isspace(c)
                    || c == 13
                    || c == 10
                    || c == '['
                    || c == ']'
                    || c == '('
                    || c == ')'
                    || c == '='
                    );
        },
        [](char c) -> bool {
            return c != '"';
        },
    };
    bool quoted = (c == '"');
    auto condition = conditions[quoted];
    if(quoted) c = fgetc(f);
    while(condition(c))
    {
        ss << c;
        c = fgetc(f);
        if(feof(f)) break;
    }
    if(quoted) c = fgetc(f);
    //return {STRING, ss.str()}; // curly bracket form crashes msvc 18.00.21005.1
    return Token(STRING, ss.str());
}

