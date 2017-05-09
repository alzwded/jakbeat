/*
Copyright (c) 2017, Vlad Mesco
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
#include <tokenizer.h>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <errorassert.h>

#include <file.h>
#include <parser.h>
#include <parser_types.h>
#include <string_utils.h>

#define VERSIONQ(X) #X
#define VERSION2(X) VERSIONQ(X)
#define MYVERSION VERSION2(VERSION)

#ifdef _MSC_VER
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef __GNUC__
    setlocale(LC_CTYPE, "C.UTF-8");
#endif
    std::wstring fileName = L"test.wav";
    for(int i = 1; i < argc; ++i) {
#ifdef _MSC_VER
        if(wcscmp(argv[i], L"-v") == 0) {
#else
        if(strcmp(argv[i], "-v") == 0) {
#endif
            wprintf(L"jakbeat v%s\nCopyright Vlad Mesco 2016\n\n", MYVERSION);
            exit(0);
#ifdef _MSC_VER
        } else if(wcscmp(argv[i], L"-w") == 0) {
#else
        } else if(strcmp(argv[i], "-w") == 0) {
#endif
            ++i;
            ASSERT(i < argc);
#ifdef _MSC_VER
            fileName.assign(argv[i]);
#else
            fileName = MB2W(argv[i]);
#endif
        }
    }

    extern void* ParseAlloc(void* (*)(size_t));
    extern void Parse(void*, int, wchar_t*, File*);
    extern void ParseFree(void*, void (*)(void*));
    extern void Render(File, std::wstring);

    File f;
    reopen_read_unicode(stdin);
    Tokenizer tok(stdin);
    auto pParser = ParseAlloc(malloc);
    do {
        auto t = tok();
        wprintf(L"%d %ls\n", t.type, (t.type == STRING) ? t.value.c_str() : L"");
        wchar_t* s = wcsdup(t.value.c_str());
        Parse(pParser, t.type, s, &f);
        if(t.type == TEOF) break;
    } while(1);
    ParseFree(pParser, free);

    Render(f, fileName);

    return 0;
}
