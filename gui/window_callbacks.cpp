#include "window.h"
#include "control.h"

#include <FL/fl_ask.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_Choice.H>

#include <cassert>
#include <algorithm>
#include <vector>

void Vindow::FileNew(Fl_Widget*, void*)
{}

void Vindow::FileOpen(Fl_Widget*, void*)
{}

void Vindow::FileSave(Fl_Widget*, void*)
{}

void Vindow::FileSaveAs(Fl_Widget*, void*)
{}

void Vindow::FileReload(Fl_Widget*, void*)
{}

void Vindow::FileExit(Fl_Widget*, void*)
{
    extern bool is_any_model_dirty();
    if(is_any_model_dirty())
    {
        int which = fl_choice("There are unsaved documents.\nAre you sure you want to exit?\n(Hint: use Vindow/Close to close just one window)", "&No", nullptr, "&Yes");
        if(which != 2) return;
    }
    exit(0);
}

void Vindow::EditUndo(Fl_Widget*, void*)
{}

void Vindow::EditCut(Fl_Widget*, void*)
{}

void Vindow::EditCopy(Fl_Widget*, void*)
{}

void Vindow::EditPaste(Fl_Widget*, void*)
{}

void Vindow::EditOverwrite(Fl_Widget*, void*)
{}

void Vindow::EditInsertRest(Fl_Widget*, void*)
{}

void Vindow::EditInsertBlank(Fl_Widget*, void*)
{}

void Vindow::EditClearColumns(Fl_Widget*, void*)
{}

void Vindow::EditDeleteColumns(Fl_Widget*, void*)
{}

void Vindow::EditAddWhat(Fl_Widget*, void*)
{}

void Vindow::EditAddWho(Fl_Widget*, void*)
{}

void Vindow::EditDeleteSection(Fl_Widget*, void*)
{}

void Vindow::WindowNew(Fl_Widget*, void* p)
{
    extern void create_window(std::shared_ptr<Model>);
    Vindow* me = (Vindow*)p;
    create_window(me->model_);
}

void Vindow::WindowClose(Fl_Widget*, void* p)
{
    extern void destroy_window(Vindow*);
    Vindow* me = (Vindow*)p;
    if(me->model_->dirty
            && me->model_->views.size() == 1)
    {
        int which = fl_choice("There are unsaved changes in the document.\nClosing this window will discard all changes.\nDo you want to close this document?", "&No", nullptr, "&Yes");
        if(which != 2) return;
    }
    return destroy_window(me);
}

void Vindow::WindowCloseAll(Fl_Widget*, void* p)
{
    extern void destroy_window(Vindow*);
    Vindow* me = (Vindow*)p;
    if(me->model_->dirty)
    {
        int which = fl_choice("There are unsaved changes in the document.\nDo you want to close this document, discarding changes?", "&No", nullptr, "&Yes");
        if(which != 2) return;
    }
    std::vector<View*> toDestroy;
    std::copy_if(me->model_->views.begin(), me->model_->views.end(), std::back_inserter(toDestroy), [me](View* view) -> bool {
                auto* w = dynamic_cast<Vindow*>(view);
                if(w == me) return false;
                return w != nullptr;
            });
    for(auto* view : toDestroy)
    {
        auto* w = dynamic_cast<Vindow*>(view);
        if(w) destroy_window(w);
    }
    return destroy_window(me);
}

void Vindow::HelpAbout(Fl_Widget*, void*)
{
    fl_alert("JakBeat GUI\nCopyright (c) 2015-2017 Vlad Meșco\nAvailable under the 2-Clause BSD License");
}

void Vindow::WhatClicked(Fl_Widget* w, void* p)
{
    Vindow* me = (Vindow*)p;
    const char* label = w->label();
    me->SetLayout(Layout::WHAT, label);
}

void Vindow::WhoClicked(Fl_Widget* w, void* p)
{
    Vindow* me = (Vindow*)p;
    const char* label = w->label();
    me->SetLayout(Layout::WHO, label);
}

void Vindow::OutputClicked(Fl_Widget* w, void* p)
{
    Vindow* me = (Vindow*)p;
    me->SetLayout(Layout::OUTPUT);
}

void Vindow::WindowCallback(Fl_Widget* w, void* p)
{
    WindowClose(w, p);
}

void Vindow::WhoNameChanged(Fl_Widget* w, void* p)
{
    auto* inp = (Fl_Input*)w;
    auto* me = (Vindow*)p;

    const char* newName = inp->value();
    const char* oldName = me->active_.c_str();

    assert(newName);
    if(std::any_of(me->model_->whos.begin(), me->model_->whos.end(), [newName](WhoEntry const& e) -> bool {
                    return e.name == newName;
                })
            || *newName == '\0')
    {
        fl_alert("Name needs to be unique and not null");
        inp->value(oldName);
        //Fl::focus(inp); // doesn't work because e.g. the tab key is
                          // handled later...
        return;
    }

    Control ctrl(me->model_, me);
    ctrl.SetWhosName(oldName, newName);
}

void Vindow::WhatNameChanged(Fl_Widget* w, void* p)
{
    auto* inp = (Fl_Input*)w;
    auto* me = (Vindow*)p;

    const char* newName = inp->value();
    const char* oldName = me->active_.c_str();

    assert(newName);
    if(std::any_of(me->model_->whos.begin(), me->model_->whos.end(), [newName](WhoEntry const& e) -> bool {
                    return e.name == newName;
                })
            || *newName == '\0'
            || strcmp(newName, "OUTPUT") == 0)
    {
        fl_alert("Name needs to be unique and not null");
        inp->value(oldName);
        //Fl::focus(inp); // doesn't work because e.g. the tab key is
                          // handled later...
        return;
    }

    Control ctrl(me->model_, me);
    ctrl.SetWhatsName(oldName, newName);
}

void Vindow::SchemaChanged(Fl_Widget* w, void* p)
{
    auto* inp = (Fl_Input_Choice*)w;
    auto* me = (Vindow*)p;

    auto&& who = me->active_;
    int idx = inp->menubutton()->value();
    assert(idx >= 0 && idx < me->drumSchemas_.size());
    Schema const* schema = &me->drumSchemas_[idx];

    Control ctrl(me->model_, me);
    ctrl.SetWhosSchema(who, schema);
}

void Vindow::ParamChanged(Fl_Widget* w, void* p)
{
    auto* inp = (Fl_Input*)w;
    auto* me = (Vindow*)p;

    auto&& who = me->active_;
    const char* param = inp->label();
    const char* value = inp->value();

    Control ctrl(me->model_, me);
    ctrl.SetWhosParam(who, param, value);
}
