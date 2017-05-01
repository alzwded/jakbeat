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
#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <list>
#include <memory>
#include <vector>
#include "view.h"

struct Schema
{
    enum Type
    {
        STRING,
        READ_ONLY_STRING,
        STUB,
        NUMBER,
        SUBSCHEMA
    };
    struct Attribute
    {
        const char* name;
        Type type;
        std::vector<Attribute> children;
    };
    const char* name;
    std::vector<Attribute> attributes;
};

typedef std::list<char> row_t;
typedef std::list<row_t> rows_t;
typedef rows_t::iterator row_p_t;

struct Column
{
    std::list<row_p_t> rows;
};

typedef Column column_t;
typedef std::list<Column> columns_t;
typedef columns_t::iterator column_p_t;

struct WhoEntry
{
    std::string name;
    Schema const* schema;
    typedef std::list<std::pair<std::string, std::string>> Params;
    Params params;
};

struct WhatEntry
{
    std::string name;
    std::string bpm;
    Schema const* schema;
    column_p_t start, end;
};

struct Output
{
    columns_t columns;
    rows_t rows;
};

struct Model
{
    std::string path;
    std::list<View*> views;
    std::list<WhoEntry> whos;
    std::list<WhatEntry> whats;
    Output output;
    columns_t columns;
    rows_t rows;

    bool dirty = false;
};

#endif
