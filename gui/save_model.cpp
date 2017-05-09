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

#include "model.h"
#include "logger.h"
#include "string_utils.h"
#include <algorithm>
#include <functional>

static void save_schema(FILE* f, std::wstring const& padding, std::vector<Schema::Attribute> const& attribs, WhoEntry::Params const& params)
{
    LOGGER(l);
    for(auto&& attr : attribs) {
        auto foundValue = std::find_if(params.begin(), params.end(), [&attr](std::pair<std::wstring, std::wstring> const& p) -> bool {
            return p.first == attr.name;
        });
        switch(attr.type) {
        case Schema::Type::STRING:
            if(foundValue == params.end()
                    || foundValue->second.empty())
            {
                l(L"skipping blank attribute %ls\n", attr.name);
                break;
            }
            fwprintf(f, L"%ls%ls = \"%ls\"\n", padding.c_str(), attr.name, foundValue->second.c_str());
            break;
        case Schema::Type::STUB:
            {
                if(attr.children.size() != 1
                        || attr.children.front().type != Schema::Type::READ_ONLY_STRING)
                {
                    l(L"%s is a stub but has invalid children\n", attr.name);
                    break;
                }
                fwprintf(f, L"%ls%ls = \"%ls\"\n", padding.c_str(), attr.name, attr.children.front().name);
            }
            break;
        case Schema::Type::NUMBER:
            if(foundValue == params.end()
                    || foundValue->second.empty())
            {
                l(L"skipping blank attribute %ls\n", attr.name);
                break;
            }
            fwprintf(f, L"%ls%ls = %ls\n", padding.c_str(), attr.name, foundValue->second.c_str());
            break;
        case Schema::Type::SUBSCHEMA:
            {
                fwprintf(f, L"%ls%ls = (\n", padding.c_str(), attr.name);
                std::wstring newPadding(padding.size() + 4, L' ');
                save_schema(f, newPadding, attr.children, params);
                fwprintf(f, L"%ls)\n", padding.c_str());
            }
            break;
        case Schema::Type::READ_ONLY_STRING:
            l(L"READ_ONLY_STRING %ls should be part of a stub\n", attr.name);
            // FALLTHROUGH
        default:
            l(L"skipping %ls\n", attr.name);
            break;
        }
    }
}

void save_model(std::shared_ptr<Model> m, std::wstring path)
{
    LOGGER(l);
    FILE* f = open_write_unicode(path.c_str());
    if(!f) {
        l(L"failed to open %ls\n", path.c_str());
        return;
    }

    l(L"Writing WHO section, %ld entries\n", m->whos.size());
    fwprintf(f, L"[WHO]\n");
    for(auto&& who : m->whos) {
        l(L"writing %ls\n", who.name.c_str());
        fwprintf(f, L"%ls = (\n", who.name.c_str());
        std::wstring padding(4, L' ');
        save_schema(f, padding, who.schema->attributes, who.params);
        fwprintf(f, L")\n");
    }
    fwprintf(f, L"\n");

    l(L"Writing WHAT section, %ld entries\n", 1 + m->whats.size());
    fwprintf(f, L"[WHAT]\n");
    l(L"Writing Output\n");
    // TODO write output
    l(L"Writing WHAT definitions\n");
    for(auto&& what : m->whats) {
        fwprintf(f, L"%ls = ( bpm = %ls )\n", what.name.c_str(), what.bpm.c_str());
    }
    fwprintf(f, L"\n");

    l(L"Writing %ld WHAT sections\n", m->whats.size());
    for(auto&& what : m->whats) {
        fwprintf(f, L"[%ls]\n", what.name.c_str());
        fwprintf(f, L"\n");
    }

    m->dirty = false;
    m->path = path;

    fclose(f);

    Event e = {
        Event::GLOBAL,
        Event::SAVED,
        L"",
        m->path,
        nullptr
    };
    std::for_each(m->views.begin(), m->views.end(), [&e](View* v) {
                v->OnEvent(&e);
            });
}

