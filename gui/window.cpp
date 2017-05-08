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
#include "logger.h"
#include "string_utils.h"

#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tile.H>

#include <cassert>
#include <algorithm>
#include <type_traits>

#define WHO_GROUP_BUTTON_START 1u
#define WHAT_GROUP_BUTTON_START 1u

Vindow::Vindow(
        std::shared_ptr<Model> m,
        std::vector<Schema> const& drumSchemas,
        std::vector<Schema> const& whatSchemas,
        const wchar_t* t,
        int w,
        int h)
    : Fl_Double_Window(w, h)
    , View()
    , model_(m)
    , drumSchemas_(drumSchemas)
    , whatSchemas_(whatSchemas)
    , active_(L"OUTPUT")
    , menu_(nullptr)
    , mainGroup_(nullptr)
    , whoGroup_(nullptr)
    , whatGroup_(nullptr)
    , editor_(nullptr)
{
    assert(model_);
    model_->views.push_back(this);
    copy_label(W2MB(t).get());

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
#if 0
        default overwrite;
        but, when in insert mode, it would allow you to type on the current
            row and insert '.'/' ' on the others, just like typing at the
            end of the column;
        could be a flag on the model
          { "Overwr&ite",
              Fl_Insert, (Fl_Callback*)EditOverwrite, this, FL_MENU_TOGGLE|FL_MENU_VALUE },
#endif
          { "Insert &Rest",
              FL_COMMAND + '0', (Fl_Callback*)EditInsertRest, this },
          { "Insert &Blank",
              FL_COMMAND + 'b', (Fl_Callback*)EditInsertBlank, this },
          { "C&lear",
              FL_BackSpace, (Fl_Callback*)EditClearColumns, this },
          { "&Delete Columns",
              FL_Delete, (Fl_Callback*)EditDeleteColumns, this, FL_MENU_DIVIDER },
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

    // window init
    constexpr int dx = 150;
    size_range(2 * dx, 2 * dx);

    // init common components
    typedef std::remove_pointer<decltype(container_)>::type container_t;
    container_ =  new container_t(0, mb->h(), w, h - mb->h());
      // create something strange from FLTK test code...
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

int Vindow::AddControlsFromSchema(
        std::function<std::string(const wchar_t*)> getter,
        decltype(Schema::attributes)::const_iterator first,
        decltype(Schema::attributes)::const_iterator last,
        int x, int y, int w, int h)
{
    if(first == last) return y + h + 5;

    auto&& att = *first;
    switch(att.type)
    {
    case Schema::STUB:
        return AddControlsFromSchema(
                getter,
                ++first, last,
                x, y, w, h);
    case Schema::STRING:
        {
            auto* inp = new Fl_Input(
                    x,
                    y,
                    w,
                    h);
            inp->copy_label(W2MB(att.name).get());
            inp->value(getter(att.name).c_str());
            inp->callback(ParamChanged, this);
        }
        break;
    case Schema::READ_ONLY_STRING:
        {
            auto* inp = new Fl_Input(
                    x,
                    y,
                    w,
                    h);
            inp->copy_label(W2MB(att.name).get());
            inp->readonly(true);
            inp->value(getter(att.name).c_str());
            inp->callback(ParamChanged, this);
        }
        break;
    case Schema::NUMBER:
        {
            auto* inp = new Fl_Int_Input(
                    x,
                    y,
                    w,
                    h);
            inp->copy_label(W2MB(att.name).get());
            inp->value(getter(att.name).c_str());
            inp->callback(ParamChanged, this);
        }
        break;
    case Schema::SUBSCHEMA:
        {
            int total = att.children.size() * h + (att.children.size() - 1) * 5 + 2*Fl::box_dy(FL_DOWN_BOX);
            auto attname = W2MB(att.name);

            auto* title = new Fl_Box(
                    x - fl_width(attname.get()),
                    y,
                    fl_width(attname.get()),
                    h);
            title->copy_label(attname.get());
            title->align(FL_ALIGN_INSIDE|FL_ALIGN_RIGHT);
            auto* b = new Fl_Group(x, y, w, total);
            b->box(FL_DOWN_BOX);
            b->begin();
              int wholeW = b->w() - Fl::box_dw(FL_DOWN_BOX);
              int newX = b->x() + 100;
              int newW = wholeW - 100;
              int at = AddControlsFromSchema(
                      getter,
                      att.children.begin(),
                      att.children.end(),
                      newX,
                      b->y() + Fl::box_dy(FL_DOWN_BOX),
                      newW,
                      h);
              auto* dummy = new Fl_Box(newX, at, newW, h);
              b->resizable(dummy);
            b->end();
            return AddControlsFromSchema(
                    getter,
                    ++first, last,
                    x, at, w, h);
        }
        break;
    }
    ++first;
    return AddControlsFromSchema(getter, first, last, x, y + h + 5, w, h);
}

void Vindow::CreateWhoList()
{
    int x = container_->x(),
        y = container_->y(),
        w = container_->w() / 4,
        h = container_->h() / 2;
    if(whoGroup_) {
        x = whoGroup_->x();
        y = whoGroup_->y();
        w = whoGroup_->w();
        h = whoGroup_->h();
        Fl::delete_widget(whoGroup_);
    }

    container_->begin();
      whoGroup_ = new Fl_Group(x, y, w, h);
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
          int w = fl_width(W2MB(who.name).get()) + 0.5 + 2*Fl::box_dw(FL_UP_BOX);
          auto* b = new Fl_Button(
                  scroll->x() + Fl::box_dx(scroll->box()),
                  scroll->y() + Fl::box_dy(scroll->box()) + 20*i,
                  w,
                  20);
          b->down_box(FL_FLAT_BOX);
          b->copy_label(W2MB(who.name).get());
          b->callback(WhoClicked, this);
          ++i;
        }
        scroll->end();
        whoGroup_->resizable(scroll);
      whoGroup_->end();
    container_->end();
}

void Vindow::CreateWhatList()
{
    assert(whoGroup_);
    int x = container_->x(),
        y = container_->y() + whoGroup_->h(),
        w = container_->w() / 4,
        h = container_->h() - whoGroup_->h();
    if(whatGroup_) {
        x = whatGroup_->x();
        y = whatGroup_->y();
        w = whatGroup_->w();
        h = whatGroup_->h();
        Fl::delete_widget(whatGroup_);
    }
    container_->begin();
      whatGroup_ = new Fl_Group(x, y, w, h);
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
          int w = fl_width(W2MB(what.name).get()) + 0.5 + 2*Fl::box_dw(FL_UP_BOX);
          auto* b = new Fl_Button(
                  scroll->x() + Fl::box_dx(scroll->box()),
                  scroll->y() + Fl::box_dy(scroll->box()) + 20*i,
                  w,
                  20);
          b->down_box(FL_FLAT_BOX);
          b->copy_label(W2MB(what.name).get());
          b->callback(WhatClicked, this);
          ++i;
        }
        scroll->end();
        whatGroup_->resizable(scroll);
      whatGroup_->end();
    container_->end();
}

Vindow::~Vindow()
{
    LOGGER(l);
    auto found = std::find_if(model_->views.begin(), model_->views.end(), [this](View* v) -> bool {
                auto* w = dynamic_cast<Vindow*>(v);
                return w == this;
            });
    if(found == model_->views.end()) {

        l(L"disconnected window destroyed!\n");
        return;
    }

    model_->views.erase(found);
}

void Vindow::OnEvent(Event* e)
{
    LOGGER(l);
    switch(e->type)
    {
        case Event::RELOADED:
            CreateWhoList();
            CreateWhatList();
            break;
        case Event::DELETED:
            switch(e->source)
            {
                case Event::WHO:
                    CreateWhoList();
                    if(active_.compare(e->targetId) == 0) {
                        SetLayout(Layout::OUTPUT);
                        SelectButton(L"OUTPUT");
                    }
                    break;
                case Event::WHAT:
                    CreateWhatList();
                    if(active_.compare(e->targetId) == 0) {
                        SetLayout(Layout::OUTPUT);
                        SelectButton(L"OUTPUT");
                    }
                    break;
            }
            break;
        case Event::NAME_CHANGED:
            switch(e->source)
            {
                case Event::WHO:
                    CreateWhoList();
                    if(active_.compare(e->targetId) == 0
                            && layout_ == Layout::WHO)
                    {
                        if(e->sourceView == static_cast<View*>(this)) {
                            SelectButton(e->changed.c_str(), Layout::WHO);
                        } else {
                            SetLayout(Layout::WHO, e->changed);
                        }
                    }
                    break;
                case Event::WHAT:
                    CreateWhatList();
                    if(active_.compare(e->targetId) == 0
                            && layout_ == Layout::WHAT)
                    {
                        if(editor_) editor_->SetTarget(e->changed.c_str());
                        if(e->sourceView == static_cast<View*>(this)) {
                            SelectButton(e->changed.c_str(), Layout::WHAT);
                        } else {
                            SelectButton(e->changed.c_str(), Layout::WHAT);
                            SetLayout(Layout::WHAT, e->changed);
                        }
                    }
                    break;
            }
            break;
        case Event::CHANGED:
            {
                Layout lyt = layout_;
                switch(e->source)
                {
                case Event::WHO: lyt = Layout::WHO; break;
                case Event::WHAT: lyt = Layout::WHAT; break;
                case Event::OUTPUT: lyt = Layout::OUTPUT; break;
                }

                if(active_.compare(e->targetId) == 0
                        && layout_ == lyt)
                {
                    if(e->sourceView != static_cast<View*>(this)
                            || e->changed == L" schema")
                    {
                        SetLayout(lyt, e->targetId);
                    } else {
                        if(editor_) editor_->Update();
                    }
                }
            }
            break;
    }
    container_->redraw();
}

void Vindow::SetLayout(Layout lyt, std::wstring const& name)
{
    LOGGER(l);
    int mx = 0, my = 0;
    if(editor_
            && active_ == name)
    {
        l(L"keeping old coordinates: %d,%d\n", my, mx);
        l(L"active=%ls name=%ls\n", active_.c_str(), name.c_str());
        mx = editor_->mx();
        my = editor_->my();
    }
    SelectButton(name.c_str(), lyt);
    layout_ = lyt;
    editor_ = nullptr;

    if(mainGroup_) Fl::delete_widget(mainGroup_);
    mainGroup_ = nullptr;

    mainGroup_ = new Fl_Group(whoGroup_->w(), container_->y(), container_->w() - whoGroup_->w(), container_->h());
    mainGroup_->box(FL_DOWN_BOX);
    container_->add(mainGroup_);

    const int THIRD = 100,
              TWOTHIRD = mainGroup_->w() - THIRD - 5;

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
                    Control(model_, this),
                    label->x(),
                    label->y() + label->h(),
                    mainGroup_->w() - 10,
                    mainGroup_->h() - label->h() - 10,
                    active_,
                    model_->output,
                    model_->whats.size(),
                    mx,
                    my);
            editor_ = editor;

            mainGroup_->add(editor);
            mainGroup_->resizable(editor);
        }
        break;
    case Layout::WHO:
        {
            auto* wholbl = new Fl_Input(
                    mainGroup_->x() + THIRD,
                    mainGroup_->y() + 5,
                    TWOTHIRD,
                    25,
                    "WHO");
            wholbl->value(W2MB(name).get());
            wholbl->callback(WhoNameChanged, this);
            mainGroup_->add(wholbl);

            auto* schemas = new Fl_Input_Choice(
                    mainGroup_->x() + THIRD,
                    wholbl->y() + wholbl->h() + 5,
                    TWOTHIRD,
                    25,
                    "stereo");
            schemas->input()->readonly(true);
            schemas->callback(SchemaChanged, this);
            for(auto&& s : drumSchemas_) {
                schemas->add(W2MB(s.name).get());
            }

            auto found = std::find_if(model_->whos.begin(), model_->whos.end(), [name](WhoEntry& e) -> bool {
                        return e.name == name;
                    });
            if(found == model_->whos.end()) {
                fl_alert("Failed to find %s...", W2MB(name).get());
                break;
            }
            auto& who = *found;

            schemas->value(W2MB(who.schema->name).get());

            std::function<std::string(const wchar_t*)> getter = [&who](const wchar_t* key) -> std::string {
                auto found = std::find_if(who.params.begin(), who.params.end(), [key](WhoEntry::Params::const_reference e) -> bool {
                            return e.first == key;
                        });
                if(found == who.params.end()) return "";
                return W2MB(found->second).get();
            };

            auto at = AddControlsFromSchema(
                    getter,
                    who.schema->attributes.begin(),
                    who.schema->attributes.end(),
                    schemas->x(),
                    schemas->y() + schemas->h() + 5,
                    TWOTHIRD,
                    25);

            auto* dummy = new Fl_Box(schemas->x(), at, schemas->w(), 25);
            mainGroup_->resizable(dummy);
        }
        break;
    case Layout::WHAT:
        {
            auto* whatlbl = new Fl_Input(
                    mainGroup_->x() + THIRD,
                    mainGroup_->y() + 5,
                    TWOTHIRD,
                    25,
                    "WHAT");
            whatlbl->value(W2MB(name).get());
            whatlbl->callback(WhatNameChanged, this);
            mainGroup_->add(whatlbl);

            auto* bpmlbl = new Fl_Int_Input(
                    mainGroup_->x() + THIRD,
                    whatlbl->y() + whatlbl->h() + 5,
                    TWOTHIRD,
                    25,
                    "bpm");
            bpmlbl->callback(WhatBpmChanged, this);
            auto foundWhat = std::find_if(model_->whats.begin(), model_->whats.end(), [name](WhatEntry& e) -> bool {
                        return e.name == name;
                    });
            if(foundWhat == model_->whats.end()) {
                fl_alert("Failed to find %s...", W2MB(name).get());
                break;
            }
            auto&& what = *foundWhat;
            bpmlbl->value(W2MB(what.bpm).get());
            mainGroup_->add(bpmlbl);

            auto* editor = new MatrixEditor(
                    Control(model_, this),
                    mainGroup_->x() + THIRD,
                    bpmlbl->y() + bpmlbl->h() + 5,
                    TWOTHIRD,
                    mainGroup_->h() - whatlbl->h() - 5 - bpmlbl->h() - 5 - 10,
                    active_,
                    what.columns,
                    model_->whos.size(),
                    mx, my);
            editor_ = editor;

            mainGroup_->add(editor);
            mainGroup_->resizable(editor);
        }
        break;
    }

    mainGroup_->end();
}

void Vindow::SelectButton(std::wstring const& reactivate1, Layout lyt)
{
    auto* whoGroup = dynamic_cast<Fl_Group*>(whoGroup_->child(WHO_GROUP_BUTTON_START)),
        * whatGroup = dynamic_cast<Fl_Group*>(whatGroup_->child(WHAT_GROUP_BUTTON_START));
    std::wstring reactivate = (!reactivate1.empty())
        ? reactivate1
        : L"OUTPUT"
        ;
    active_ = reactivate;
    assert(whoGroup);
    assert(whatGroup);
    for(size_t i = 0; i < whoGroup->children(); ++i) {
        auto* b = dynamic_cast<Fl_Button*>(whoGroup->child(i));
        if(b) {
            b->value(0);
            if(MB2W(b->label()) == reactivate
                    && lyt != Layout::WHAT)
                b->value(1);
        }
    }
    for(size_t i = 0; i < whatGroup->children(); ++i) {
        auto* b = dynamic_cast<Fl_Button*>(whatGroup->child(i));
        if(b) {
            b->value(0);
            if(MB2W(b->label()) == reactivate
                    && lyt != Layout::WHO)
                b->value(1);
        }
    }
}
