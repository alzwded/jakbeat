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

#include "control.h"
#include "logger.h"
#include "string_utils.h"

#include <algorithm>
#include <cassert>
#include <cstdio>

#ifndef USE_REGEX_RENAME
# ifdef __GNUC__
#  define USE_REGEX_RENAME
# endif
#endif

#ifdef USE_REGEX_RENAME
# include <regex>
# include <sstream>
#endif

#define FIND_WHAT(id, then) \
    auto found = FindWhat(id); \
    if(found == model_->whats.end()) then;

#define DIRTY() do{\
    model_->dirty = true; \
}while(0)

auto Control::FindWhat(evData id) -> decltype(model_->whats)::iterator
{
    LOGGER(l);
    auto found = std::find_if(model_->whats.begin(), model_->whats.end(), [id](WhatEntry& e) -> bool {
                return e.name == id;
            });
    if(found == model_->whats.end())
    {
        l(L"Failed to find %ls...", id.c_str());
    }
    return found;
}

void Control::SetWhosName(evData oldName, std::wstring name)
{
    LOGGER(l);
    auto found = std::find_if(model_->whos.begin(), model_->whos.end(), [oldName](WhoEntry& e) -> bool {
                return e.name == oldName;
            });
    if(found == model_->whos.end())
    {
        l(L"Failed to find %ls...", oldName.c_str());
        return;
    }
    found->name = name;
    DIRTY();
    Event e = {
        Event::WHO,
        Event::NAME_CHANGED,
        oldName,
        name.c_str(),
        source_
    };
    Fire(&e);
}

void Control::SetWhosParam(Control::evData id, std::wstring key, std::wstring value)
{
    LOGGER(l);
    auto foundEntry = std::find_if(model_->whos.begin(), model_->whos.end(), [id](WhoEntry& e) -> bool {
                return e.name == id;
            });
    if(foundEntry == model_->whos.end()) {
        l(L"Failed to find %ls...", id.c_str());
        return;
    }
    auto&& params = foundEntry->params;

    auto found = std::find_if(params.begin(), params.end(), [key](WhoEntry::Params::reference p) -> bool {
                return p.first == key;
            });
    if(found == params.end()) {
        params.emplace_back(key, value);
    } else {
        found->second = value;
    }

    DIRTY();
    Event e = {
        Event::WHO,
        Event::CHANGED,
        id,
        key,
        source_
    };
    Fire(&e);
}

void Control::SetWhosSchema(Control::evData id, Schema const* schema)
{
    LOGGER(l);
    auto foundEntry = std::find_if(model_->whos.begin(), model_->whos.end(), [id](WhoEntry& e) -> bool {
                return e.name == id;
            });
    if(foundEntry == model_->whos.end()) {
        l(L"Failed to find %ls...", id.c_str());
        return;
    }

    foundEntry->schema = schema;

    DIRTY();
    Event e = {
        Event::WHO,
        Event::CHANGED,
        id,
        L" schema",
        source_
    };
    Fire(&e);
}

void Control::SetWhatsName(Control::evData id, std::wstring name)
{
    LOGGER(l);
    auto found = std::find_if(model_->whats.begin(), model_->whats.end(), [id](WhatEntry& e) -> bool {
                return e.name == id;
            });
    if(found == model_->whats.end())
    {
        l(L"Failed to find %ls...", id.c_str());
        return;
    }
    found->name = name;
    DIRTY();
    Event e = {
        Event::WHAT,
        Event::NAME_CHANGED,
        id,
        name.c_str(),
        source_
    };
    Fire(&e);
#ifdef USE_REGEX_RENAME
    std::wstringstream pattern;
    pattern << L"\\b" << id << L"\\b";
    std::wregex r(pattern.str(), std::wregex::ECMAScript);
    std::wstringstream buf;
    std::regex_replace(std::ostreambuf_iterator<wchar_t>(buf),
            model_->output.begin(), model_->output.end(), r, name);
    auto newOutput = buf.str();
    if(newOutput != model_->output) {
        model_->output = newOutput;
        Event e = {
            Event::OUTPUT,
            Event::CHANGED,
            L"OUTPUT",
            L"",
            source_
        };
        Fire(&e);
    }
#endif
}

void Control::SetWhatsBpm(Control::evData id, std::wstring bpm)
{
    LOGGER(l);
    FIND_WHAT(id, return);
    found->bpm = bpm;
    DIRTY();
    Event e = {
        Event::WHAT,
        Event::CHANGED,
        id,
        L"bpm",
        source_
    };
    Fire(&e);
}

Event::Source Control::InsertColumnPrivate(evData id, column_p_t before, wchar_t c)
{
    LOGGER(l);
    if(id.empty()
            || id == L"OUTPUT")
    {
#if 1
        l(L"using text editor, should not be called\n");
#else
        Column col;
        auto size = model_->whats.size();
        model_->output.insert(before, column_t(size, c));
        DIRTY();
        return Event::OUTPUT;
#endif
    }

    FIND_WHAT(id, return Event::WHAT);
    auto size = model_->whos.size();
    found->columns.insert(before, column_t(size, c));
    DIRTY();
    return Event::WHAT;
}

void Control::InsertColumn(evData id, column_p_t before, wchar_t c)
{
    LOGGER(l);
    auto source = InsertColumnPrivate(id, before, c);
    if(source == Event::GLOBAL) return;
    Event e = {
        source,
        Event::CHANGED,
        id,
        L"",
        source_
    };
    Fire(&e);
}

void Control::Fire(Event* ev)
{
    LOGGER(l);
    std::for_each(model_->views.begin(), model_->views.end(), [&ev](View* v) {
                v->OnEvent(ev);
            });
}

void Control::SetCell(evData what, column_p_t before, int row, wchar_t c)
{
    LOGGER(l);
    if(what.empty()
            || what == L"OUTPUT")
    {
#if 1
        l(L"using text editor, should not be called\n");
#else
        if(before == model_->output.end())
        {
            (void) InsertColumnPrivate(what, before, ' ');
            auto pos = model_->output.end();
            std::advance(pos, -1);
            return SetCell(what, pos, row, c);
        }
        auto pos = before->begin();
        std::advance(pos, row);
        *pos = c;
        DIRTY();
        Event e  = {
            Event::OUTPUT,
            Event::CHANGED,
            what,
            L"",
            source_
        };
        Fire(&e);
#endif
        return;
    }

    FIND_WHAT(what, return);
    if(before == found->columns.end())
    {
        (void) InsertColumnPrivate(what, before, (c != ' ') ? '.' : ' ');
        auto pos = found->columns.end();
        std::advance(pos, -1);
        return SetCell(what, pos, row, c);
    }
    auto pos = before->begin();
    std::advance(pos, row);
    *pos = c;
    DIRTY();
    Event e  = {
        Event::WHAT,
        Event::CHANGED,
        what,
        L"",
        source_
    };
    Fire(&e);
}

void Control::BlankCell(evData id, int col, int row)
{
    LOGGER(l);
    if(id.empty()
            || id == L"OUTPUT")
    {
#if 1
        l(L"using text editor, should not be called\n");
#else
        auto it = model_->output.begin();
        std::advance(it, col);
        auto& col = *it;
        if(row < 0 || row >= col.size())
        {
            l(L"my=%d is out of bounds\n", row);
            return;
        }
        auto cit = col.begin();
        std::advance(cit, row);
        auto& cell = *cit;
        cell = (cell == L' ')
            ? L' '
            : L'.'
            ;
        DIRTY();    

        Event e = {
            Event::OUTPUT,
            Event::CHANGED,
            id,
            L"",
            source_
        };
        Fire(&e);
#endif

        return;
    } else {
        FIND_WHAT(id, return);
        WhatEntry& we = *found;
        auto it = we.columns.begin();
        std::advance(it, col);
        auto& col = *it;
        if(row < 0 || row >= col.size())
        {
            l(L"my=%d is out of bounds\n", row);
            return;
        }
        auto cit = col.begin();
        std::advance(cit, row);
        auto& cell = *cit;
        cell = (cell == L' ')
            ? L' '
            : L'.'
            ;
        DIRTY();

        Event e = {
            Event::WHAT,
            Event::CHANGED,
            id,
            L"",
            source_
        };
        Fire(&e);

        return;
    }
}

void Control::InsertText(int mpos, int pos, std::wstring const& text)
{
    LOGGER(l);
    l(L"model: %d view: %d %ls\n", mpos, pos, text.c_str());
    model_->output.insert(mpos, text);
    DIRTY();
    Event e = {
        Event::OUTPUT,
        Event::TEXT_INSERTED,
        std::to_wstring(pos),
        text,
        source_
    };
    Fire(&e);
}

void Control::DeleteText(int mpos, int mlength, int pos, int length)
{
    LOGGER(l);
    l(L"model: %d+%d view: %d+%d\n", mpos, mlength, pos, length);
    model_->output.erase(mpos, mlength);
    DIRTY();
    Event e = {
        Event::OUTPUT,
        Event::TEXT_DELETED,
        std::to_wstring(pos),
        std::to_wstring(length),
        source_
    };
    Fire(&e);
}

void Control::DeleteWhat(evData id)
{
    LOGGER(l);
    auto found = std::find_if(model_->whats.begin(), model_->whats.end(), [&id](WhatEntry& what) -> bool {
                return what.name == id;
            });
    if(found == model_->whats.end()) {
        l(L"%ls not found in model\n", id.c_str());
        return;
    }
    model_->whats.erase(found);
    Event e = {
        Event::WHAT,
        Event::DELETED,
        id,
        L"",
        source_
    };
    Fire(&e);
}

void Control::DeleteWho(evData id)
{
    LOGGER(l);
    size_t nth = 0;
    auto found = model_->whos.end();
    for(auto it = model_->whos.begin(); it != model_->whos.end(); ++it, ++nth) {
        if(it->name == id) {
            found = it;
            break;
        }
    }
    if(found == model_->whos.end()) {
        l(L"%ls not found in model\n", id.c_str());
        return;
    }
    l(L"cleaning up row %ld\n", nth);
    std::for_each(model_->whats.begin(), model_->whats.end(), [nth, &l](WhatEntry& e) {
                l(L"cleaning up in %ls\n", e.name.c_str());
                std::for_each(e.columns.begin(), e.columns.end(), [nth](column_t& col) {
                            auto pos = col.begin();
                            std::advance(pos, nth);
                            col.erase(pos);
                        });
            });

    l(L"erasing entry\n");
    model_->whos.erase(found);
    Event e = {
        Event::WHO,
        Event::DELETED,
        id,
        L"",
        source_
    };
    Fire(&e);
}
