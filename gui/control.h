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
#ifndef CONTROL_H
#define CONTROL_H

#include <memory>
#include <cwchar>
#include "model.h"
#include "view.h"

class Control
{
    std::shared_ptr<Model> model_;
    View* source_;

public:
    Control(
            std::shared_ptr<Model> model,
            View* source)
        : model_(model)
        , source_(source)
    {}

    typedef std::wstring evData;

    int AddWho();
    int AddWhat();
    void SetWhosName(evData id, std::wstring name);
    void SetWhosSchema(evData id, Schema const* schema);
    void SetWhosParam(evData id, std::wstring key, std::wstring value);
    void SetWho(evData id, std::wstring name, Schema const* schema, WhoEntry::Params params);
    void SetWhatsName(evData id, std::wstring name);
    void SetWhatsBpm(evData id, std::wstring name);
    void DeleteWho(evData id);
    void DeleteWhat(evData id);

    void InsertColumn(evData id, column_p_t before, wchar_t c = ' ');
    void SetCell(evData id, column_p_t column, int row, wchar_t c);
    void DeleteColumn(column_p_t column);

private:
    auto FindWhat(evData id) -> decltype(model_->whats)::iterator;
    Event::Source InsertColumnPrivate(evData id, column_p_t before, wchar_t c = ' ');
    void Fire(Event* ev);
};

#endif
