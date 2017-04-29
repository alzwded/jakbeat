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

class Control
{
    std::shared_ptr<Model> model_;

public:
    Control(std::shared_ptr<Model> model)
        : model_(model)
    {}

    int AddWho();
    int AddWhat();
    void SetWhosName(int id, std::string name);
    void SetWhosSchema(int id, Schema const* schema);
    void SetWhosParams(int id, WhoEntry::Params params);
    void SetWho(int id, std::string name, Schema const* schema, WhoEntry::Params params);
    void SetWhatsName(int id, std::string name);
    void DeleteWho(int id);
    void DeleteWhat(int id);

    void InsertColumn(column_p_t before, char c = ' ');
    void DeleteColumn(column_p_t column);
    void SetCell(column_p_t column, int row, char c);
    void SetOutputCell(column_p_t before, int row);
};

#endif
