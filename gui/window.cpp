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

#include "window.h"
#include "matrix_editor.h"

#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tile.H>

#include <cassert>
#include <algorithm>

Vindow::Vindow(std::shared_ptr<Model> m, int w, int h, const char* t)
    : Fl_Double_Window(w, h, t)
    , View()
    , model_(m)
    , menu_(nullptr)
    , mainGroup_(nullptr)
{
    assert(model_);
    model_->views.push_back(this);

    // init menu
    Fl_Menu_Item menuitems[] = {
        { "&File",
            0, 0, 0, FL_SUBMENU },
          { "&New...",
              0, (Fl_Callback*)FileNew },
          { "&Open...",
              FL_COMMAND + 'o', (Fl_Callback*)FileOpen },
          { "&Save",
              FL_COMMAND + 's', (Fl_Callback*)FileSave },
          { "Save &As...",
              FL_COMMAND + FL_SHIFT + 's', (Fl_Callback*)FileSaveAs,  },
          { "&Reload",
              FL_F + 5, (Fl_Callback*)FileReload, 0, FL_MENU_DIVIDER },
          { "E&xit",
              FL_COMMAND + 'x', (Fl_Callback*)FileExit, 0 },
          { 0 },
        { "&Edit", 0, 0, 0, FL_SUBMENU },
          { "&Undo",
              FL_COMMAND + 'z', (Fl_Callback*)EditUndo, 0, FL_MENU_DIVIDER },
          { "Cu&t",
              FL_COMMAND + 'x', (Fl_Callback*)EditCut },
          { "&Copy",
              FL_COMMAND + 'c', (Fl_Callback*)EditCopy },
          { "&Paste",
              FL_COMMAND + 'v', (Fl_Callback*)EditPaste, 0, FL_MENU_DIVIDER },
          { "Over&write",
              FL_Insert, (Fl_Callback*)EditOverwrite, 0, FL_MENU_TOGGLE },
          { "Insert &Rest",
              0, (Fl_Callback*)EditInsertRest },
          { "Insert &Blank",
              0, (Fl_Callback*)EditInsertBlank },
          { "C&lear Columns",
              FL_COMMAND + 'b', (Fl_Callback*)EditClearColumns },
          { "&Delete Columns",
              FL_Delete, (Fl_Callback*)EditDeleteColumns, 0, FL_MENU_DIVIDER },
          { "Add WH&AT Section",
              0, (Fl_Callback*)EditAddWhat },
          { "Add WH&O Section",
              0, (Fl_Callback*)EditAddWho },
          { "Delete &Section",
              0, (Fl_Callback*)EditDeleteSection },
          { 0 },
        { "&Window",
            0, 0, 0, FL_SUBMENU },
          { "&New Window",
              FL_COMMAND + 'n', (Fl_Callback*)WindowNew, this },
          { "&Close",
              FL_COMMAND + 'w', (Fl_Callback*)WindowClose, this },
          { "Close &All",
              0, (Fl_Callback*)WindowCloseAll, this },
          { 0 },
        { "&Help",
            0, 0, 0, FL_SUBMENU },
          { "A&bout",
              FL_F + 1, (Fl_Callback*)HelpAbout, this },
          { 0 },
        { 0 }
    };

    auto* mb = new Fl_Menu_Bar(0, 0, w, 30);
    mb->copy(menuitems);

    // callback
    callback(WindowCallback, this);

    // init common components
    container_ =  new Fl_Tile(0, mb->h(), w, h - mb->h());
      // create something strange from FLTK test code...
      constexpr int dx = 150;
      auto* limit = new Fl_Box(container_->x() + dx, container_->y() + dx, container_->w() - 2*dx, container_->h() - 2*dx);
      container_->resizable(limit);

      CreateWhoList();

      CreateWhatList();

      SetLayout(Layout::OUTPUT);

    container_->end();

    // init self
    border(true);
    resizable(container_);

    end();
}

void Vindow::CreateWhoList()
{
      whoGroup_ = new Fl_Group(container_->x(), container_->y(), container_->w() / 4, container_->h() / 2);
      container_->add(whoGroup_);
      whoGroup_->box(FL_DOWN_BOX);
        fl_font(FL_HELVETICA, 14);
        auto* whoLabel = new Fl_Box(5, whoGroup_->y() + 5, fl_width("WHO"), fl_height());
        fl_font(whoLabel->labelfont(), whoLabel->labelsize());
        whoLabel->size(fl_width("WHO"), fl_height());
        whoLabel->label("WHO");
        whoLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        whoGroup_->add(whoLabel);

        auto* scroll = new Fl_Scroll(5, whoLabel->y() + whoLabel->h(), whoGroup_->w() - 10, whoGroup_->h() - whoLabel->h() - 10);
        scroll->box(FL_DOWN_BOX);
        whoGroup_->add(scroll);

        int i = 0;
        for(auto&& who : model_->whos) {
          int w = fl_width(who.name.c_str()) + 0.5 + 2*Fl::box_dw(FL_UP_BOX);
          auto* b = new Fl_Button(
                  scroll->x() + Fl::box_dx(scroll->box()),
                  scroll->y() + Fl::box_dy(scroll->box()) + 20*i,
                  w,
                  20);
          b->down_box(FL_FLAT_BOX);
          b->label(who.name.c_str());
          b->callback(WhoClicked, this);
          ++i;
        }
        scroll->end();
        whoGroup_->resizable(scroll);
      whoGroup_->end();
}

void Vindow::CreateWhatList()
{
      whatGroup_ = new Fl_Group(container_->x(), whoGroup_->y() + whoGroup_->h(), container_->w() / 4, container_->h() / 2);
      container_->add(whatGroup_);
      whatGroup_->box(FL_DOWN_BOX);
        fl_font(FL_HELVETICA, 14);
        auto* whatLabel = new Fl_Box(5, whatGroup_->y() + 5, fl_width("WHAT"), fl_height());
        fl_font(whatLabel->labelfont(), whatLabel->labelsize());
        whatLabel->size(fl_width("WHAT"), fl_height());
        whatLabel->label("WHAT");
        whatLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        whatGroup_->add(whatLabel);

        auto* scroll = new Fl_Scroll(5, whatLabel->y() + whatLabel->h(), whatGroup_->w() - 10, whatGroup_->h() - whatLabel->h() - 10);
        scroll->box(FL_DOWN_BOX);
        whatGroup_->add(scroll);
        auto* b = new Fl_Button(
                scroll->x() + Fl::box_dx(scroll->box()),
                scroll->y() + Fl::box_dy(scroll->box()),
                (int)(fl_width("OUTPUT") + 0.5) + Fl::box_dw(FL_UP_BOX),
                20,
                "OUTPUT");
        b->down_box(FL_FLAT_BOX);
        b->callback(OutputClicked, this);
        int i = 1;
        for(auto&& what : model_->whats) {
          int w = fl_width(what.name.c_str()) + 0.5 + 2*Fl::box_dw(FL_UP_BOX);
          auto* b = new Fl_Button(
                  scroll->x() + Fl::box_dx(scroll->box()),
                  scroll->y() + Fl::box_dy(scroll->box()) + 20*i,
                  w,
                  20);
          b->down_box(FL_FLAT_BOX);
          b->label(what.name.c_str());
          b->callback(WhatClicked, this);
          ++i;
        }
        scroll->end();
        whatGroup_->resizable(scroll);
      whatGroup_->end();
}

Vindow::~Vindow()
{
    auto found = std::find_if(model_->views.begin(), model_->views.end(), [this](View* v) -> bool {
                auto* w = dynamic_cast<Vindow*>(v);
                return w == this;
            });
    if(found == model_->views.end()) {
        fprintf(stderr, "disconnected window destroyed!\n");
        return;
    }

    model_->views.erase(found);
}

// TODO will be useful for param changes, sequence changes etc
// More relvant changes that impact the who/what lists still
// need to update those
#define IGNORE_OWN do{\
    Vindow* w = dynamic_cast<Vindow*>(e->sourceView); \
    if(w == this) return; \
}while(0)

void Vindow::OnEvent(Event* e)
{
    switch(e->type)
    {
        case Event::RELOADED:
            Fl::delete_widget(whoGroup_);
            Fl::delete_widget(whatGroup_);
            CreateWhoList();
            CreateWhatList();
            break;
        case Event::DELETED:
            switch(e->source)
            {
                case Event::WHO:
                    Fl::delete_widget(whoGroup_);
                    CreateWhoList();
                    break;
                case Event::WHAT:
                    Fl::delete_widget(whoGroup_);
                    CreateWhoList();
                    break;
            }
            break;
        case Event::NAME_CHANGED:
            switch(e->source)
            {
                case Event::WHO:
                    Fl::delete_widget(whoGroup_);
                    CreateWhoList();
                    break;
                case Event::WHAT:
                    Fl::delete_widget(whoGroup_);
                    CreateWhoList();
                    break;
            }
            break;
    }
}

void Vindow::SetLayout(Layout lyt, const char* name)
{
    SelectButton(name);
    layout_ = lyt;

    if(mainGroup_) Fl::delete_widget(mainGroup_);
    mainGroup_ = nullptr;

    mainGroup_ = new Fl_Group(whoGroup_->w(), container_->y(), container_->w() - whoGroup_->w(), container_->h());
    mainGroup_->box(FL_DOWN_BOX);
    container_->add(mainGroup_);

    // set new layout
    switch(lyt)
    {
    case Layout::OUTPUT:
        {
            fl_font(FL_HELVETICA, 14);
            auto* label = new Fl_Box(
                    mainGroup_->x() + 5,
                    mainGroup_->y() + 5,
                    fl_width("Editing OUTPUT"),
                    fl_height(),
                    "Editing OUTPUT");
            fl_font(label->labelfont(), label->labelsize());
            label->size(fl_width("Editing OUTPUT"), fl_height());
            label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
            mainGroup_->add(label);

            auto* editor = new MatrixEditor(
                    label->x(),
                    label->y() + label->h(),
                    mainGroup_->w() - 10,
                    mainGroup_->h() - label->h() - 10,
                    model_->output.columns.begin(),
                    model_->output.columns.end());

            mainGroup_->add(editor);
            mainGroup_->resizable(editor);
        }
        break;
    case Layout::WHO:
        {
        }
        break;
    case Layout::WHAT:
        {
            const int THIRD = (mainGroup_->w()) / 3 + 5,
                      TWOTHIRD = mainGroup_->w() - THIRD - 5;

            auto* whatlbl = new Fl_Input(
                mainGroup_->x() + THIRD,
                mainGroup_->y() + 5,
                TWOTHIRD,
                25,
                "WHAT");
            whatlbl->value(name);
            mainGroup_->add(whatlbl);
            auto* bpmlbl = new Fl_Input(
                mainGroup_->x() + THIRD,
                whatlbl->y() + whatlbl->h() + 5,
                TWOTHIRD,
                25,
                "bpm");
            mainGroup_->add(bpmlbl);

            auto* editor = new MatrixEditor(
                    mainGroup_->x() + 5,
                    bpmlbl->y() + bpmlbl->h() + 5,
                    mainGroup_->w() - 10,
                    mainGroup_->h() - whatlbl->h() - 5 - bpmlbl->h() - 5 - 10,
                    model_->output.columns.begin(),
                    model_->output.columns.end());

            mainGroup_->add(editor);
            mainGroup_->resizable(editor);

        }
        break;
    }

    mainGroup_->end();
}

void Vindow::SelectButton(const char* reactivate1)
{
    auto* whoGroup = dynamic_cast<Fl_Group*>(whoGroup_->child(1)),
        * whatGroup = dynamic_cast<Fl_Group*>(whatGroup_->child(1));
    const char* reactivate = (reactivate1 && *reactivate1)
        ? reactivate1
        : "OUTPUT"
        ;
    assert(whoGroup);
    assert(whatGroup);
    for(size_t i = 0; i < whoGroup->children(); ++i) {
        auto* b = dynamic_cast<Fl_Button*>(whoGroup->child(i));
        if(b) {
            b->value(0);
            if(strcmp(b->label(), reactivate) == 0) b->value(1);
        }
    }
    for(size_t i = 0; i < whatGroup->children(); ++i) {
        auto* b = dynamic_cast<Fl_Button*>(whatGroup->child(i));
        if(b) {
            b->value(0);
            if(strcmp(b->label(), reactivate) == 0) b->value(1);
        }
    }
}
