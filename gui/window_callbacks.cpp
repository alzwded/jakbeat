#include "window.h"

#include <FL/fl_ask.H>

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

void Vindow::WhatClicked(Fl_Widget* w, void* p)
{
    Vindow* me = (Vindow*)p;
    const char* label = w->label();
    fl_alert("WHAT %s selected", label);
    me->SetLayout(Layout::WHAT);
}

void Vindow::WhoClicked(Fl_Widget* w, void* p)
{
    Vindow* me = (Vindow*)p;
    const char* label = w->label();
    fl_alert("WHO %s selected", label);
    me->SetLayout(Layout::WHO);
}

void Vindow::OutputClicked(Fl_Widget*, void* p)
{
    Vindow* me = (Vindow*)p;
    me->SetLayout(Layout::OUTPUT);
    fl_alert("OUTPUT mode enabled");
}

void Vindow::WindowCallback(Fl_Widget* w, void* p)
{
    WindowClose(w, p);
}

