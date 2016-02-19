/*
Copyright (c) 2016, Vlad Meșco
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
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <errorassert.h>

#include <parser.h>
#include <parser_types.h>

#define TEOF (0)

int tokenizer_lineno = 1;

struct Token
{
    typedef int Type;
    Type type;
    std::string value;
    Token(Type type_, std::string value_ = "")
        : type(type_), value(value_)
    {}
};

struct Tokenizer
{
    FILE* f;
    char c;

    Tokenizer(FILE* f_)
        : f(f_)
    {
        if(feof(f)) c = EOF;
        else c = fgetc(f);
    }

    Token operator()()
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
};

int main(int argc, char* argv[])
{
    std::string fileName = "test.wav";
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "-v") == 0) {
            printf("jakbeat v%d\nCopyright Vlad Mesco 2016\n\n", VERSION);
            exit(0);
        } else if(strcmp(argv[i], "-w") == 0) {
            ++i;
            ASSERT(i < argc);
            fileName.assign(argv[i]);
        }
    }

    extern void* ParseAlloc(void* (*)(size_t));
    extern void Parse(void*, int, char*, File*);
    extern void ParseFree(void*, void (*)(void*));
    extern void Render(File, std::string);

    File f;
    Tokenizer tok(stdin);
    auto pParser = ParseAlloc(malloc);
    do {
        auto t = tok();
        printf("%d %s\n", t.type, (t.type == STRING) ? t.value.c_str() : "");
        char* s = strdup(t.value.c_str());
        Parse(pParser, t.type, s, &f);
        if(t.type == TEOF) break;
    } while(1);
    ParseFree(pParser, free);

    Render(f, fileName);

    return 0;
}
