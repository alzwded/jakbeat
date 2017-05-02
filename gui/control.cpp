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

#include <algorithm>
#include <cstdio>

void Control::SetWhosName(evData oldName, std::string name)
{
    auto found = std::find_if(model_->whos.begin(), model_->whos.end(), [oldName](WhoEntry& e) -> bool {
                return e.name == oldName;
            });
    if(found == model_->whos.end())
    {
        fprintf(stderr, "Failed to find %s...", oldName.c_str());
        return;
    }
    found->name = name;
    model_->dirty = true;
    Event e = {
        Event::WHO,
        Event::NAME_CHANGED,
        oldName,
        name.c_str(),
        source_
    };
    std::for_each(model_->views.begin(), model_->views.end(), [&e](View* v) {
                v->OnEvent(&e);
            });
}

void Control::SetWhosParam(Control::evData id, std::string key, std::string value)
{
    auto foundEntry = std::find_if(model_->whos.begin(), model_->whos.end(), [id](WhoEntry& e) -> bool {
                return e.name == id;
            });
    if(foundEntry == model_->whos.end()) {
        fprintf(stderr, "Failed to find %s...", id.c_str());
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

    model_->dirty = true;
    Event e = {
        Event::WHO,
        Event::CHANGED,
        id,
        key,
        source_
    };
    std::for_each(model_->views.begin(), model_->views.end(), [&e](View* v) {
                v->OnEvent(&e);
            });
}

void Control::SetWhosSchema(Control::evData id, Schema const* schema)
{
    auto foundEntry = std::find_if(model_->whos.begin(), model_->whos.end(), [id](WhoEntry& e) -> bool {
                return e.name == id;
            });
    if(foundEntry == model_->whos.end()) {
        fprintf(stderr, "Failed to find %s...", id.c_str());
        return;
    }

    foundEntry->schema = schema;

    model_->dirty = true;
    Event e = {
        Event::WHO,
        Event::CHANGED,
        id,
        " schema",
        source_
    };
    std::for_each(model_->views.begin(), model_->views.end(), [&e](View* v) {
                v->OnEvent(&e);
            });
}

void Control::SetWhatsName(Control::evData id, std::string name)
{
    auto found = std::find_if(model_->whats.begin(), model_->whats.end(), [id](WhatEntry& e) -> bool {
                return e.name == id;
            });
    if(found == model_->whats.end())
    {
        fprintf(stderr, "Failed to find %s...", id.c_str());
        return;
    }
    found->name = name;
    model_->dirty = true;
    Event e = {
        Event::WHAT,
        Event::NAME_CHANGED,
        id,
        name.c_str(),
        source_
    };
    std::for_each(model_->views.begin(), model_->views.end(), [&e](View* v) {
                v->OnEvent(&e);
            });
}

void Control::SetWhatsBpm(Control::evData id, std::string bpm)
{
    auto found = std::find_if(model_->whats.begin(), model_->whats.end(), [id](WhatEntry& e) -> bool {
                return e.name == id;
            });
    if(found == model_->whats.end())
    {
        fprintf(stderr, "Failed to find %s...", id.c_str());
        return;
    }
    found->bpm = bpm;
    model_->dirty = true;
    Event e = {
        Event::WHAT,
        Event::CHANGED,
        id,
        "bpm",
        source_
    };
    std::for_each(model_->views.begin(), model_->views.end(), [&e](View* v) {
                v->OnEvent(&e);
            });
}
