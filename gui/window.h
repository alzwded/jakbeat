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
#ifndef WINDOW_H
#define WINDOW_H

#include "view.h"
#include "model.h"
#include <FL/Fl.H>
#include <Fl/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tile.H>

// if you're wondering why it's called Vindow,
// it's because a particular FL header has a typedef ... Window
// in the global ns...
struct Vindow
: public Fl_Double_Window
, public View
{
    Vindow(
            std::shared_ptr<Model> m,
            std::vector<Schema> const& drumSchemas,
            std::vector<Schema> const& whatSchemas,
            const char* t = "jakbeat-gui",
            int w = 600,
            int h = 400);
    ~Vindow() override;
    void OnEvent(Event*) override;

    std::shared_ptr<Model> GetModel() const
    {
        return model_;
    }

private:
    // callbacks
    static void FileNew(Fl_Widget*, void*);
    static void FileOpen(Fl_Widget*, void*);
    static void FileSave(Fl_Widget*, void*);
    static void FileSaveAs(Fl_Widget*, void*);
    static void FileReload(Fl_Widget*, void*);
    static void FileExit(Fl_Widget*, void*);
    static void EditUndo(Fl_Widget*, void*);
    static void EditCut(Fl_Widget*, void*);
    static void EditCopy(Fl_Widget*, void*);
    static void EditPaste(Fl_Widget*, void*);
    static void EditOverwrite(Fl_Widget*, void*);
    static void EditInsertRest(Fl_Widget*, void*);
    static void EditInsertBlank(Fl_Widget*, void*);
    static void EditClearColumns(Fl_Widget*, void*);
    static void EditDeleteColumns(Fl_Widget*, void*);
    static void EditAddWhat(Fl_Widget*, void*);
    static void EditAddWho(Fl_Widget*, void*);
    static void EditDeleteSection(Fl_Widget*, void*);
    static void WindowNew(Fl_Widget*, void*);
    static void WindowClose(Fl_Widget*, void*);
    static void WindowCloseAll(Fl_Widget*, void*);
    static void HelpAbout(Fl_Widget*, void*);

    static void WindowCallback(Fl_Widget*, void*);

    static void WhatClicked(Fl_Widget*, void*);
    static void WhoClicked(Fl_Widget*, void*);
    static void OutputClicked(Fl_Widget*, void*);

    static void WhoNameChanged(Fl_Widget*, void*);
    static void SchemaChanged(Fl_Widget*, void*);
    static void ParamChanged(Fl_Widget*, void*);
    static void WhatNameChanged(Fl_Widget*, void*);

private:
    enum class Layout
    {
        OUTPUT,
        WHO,
        WHAT,
    };

    void SetLayout(Layout, const char* = "");

    void CreateWhoList();
    void CreateWhatList();
    int AddControlsFromSchema(
        std::function<const char*(const char*)> getter,
        decltype(Schema::attributes)::const_iterator first,
        decltype(Schema::attributes)::const_iterator last,
        int x, int y, int w, int h);

    void SelectButton(const char* reactivate = nullptr);

private:
    std::shared_ptr<Model> model_;
    std::vector<Schema> const& drumSchemas_;
    std::vector<Schema> const& whatSchemas_;
    Layout layout_;
    std::string active_;

private:
    Fl_Menu_Bar* menu_;
    Fl_Group* mainGroup_, * whoGroup_, * whatGroup_;
    Fl_Tile* container_;
};

#endif
