#include "window.h"
#include "control.h"
#include "logger.h"
#include "string_utils.h"

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

void Vindow::EditInsertRest(Fl_Widget*, void* p)
{
    LOGGER(l);
    auto me = (Vindow*)p;
    assert(me->editor_);
    Control ctrl(me->model_, me);
    if(me->layout_ == Vindow::Layout::OUTPUT) {
        auto pos = me->model_->output.begin();
        std::advance(pos, me->editor_->mx());
        ctrl.InsertColumn(me->active_, pos, L'.');
        assert(me->editor_);
        me->editor_->Update(me->model_->whats.size());
    } else if(me->layout_ == Vindow::Layout::WHAT) {
        auto found = std::find_if(me->model_->whats.begin(), me->model_->whats.end(), [me](WhatEntry& what) -> bool {
                    return what.name == me->active_;
                });
        if(found == me->model_->whats.end()) {
            l(L"%ls not found\n", me->active_.c_str());
            return;
        }
        auto pos = found->columns.begin();
        std::advance(pos, me->editor_->mx());
        ctrl.InsertColumn(me->active_, pos, L'.');
        assert(me->editor_);
        me->editor_->Update(me->model_->whos.size());
    }
}

void Vindow::EditInsertBlank(Fl_Widget*, void* p)
{
    LOGGER(l);
    auto me = (Vindow*)p;
    assert(me->editor_);
    Control ctrl(me->model_, me);
    if(me->layout_ == Vindow::Layout::OUTPUT) {
        auto pos = me->model_->output.begin();
        std::advance(pos, me->editor_->mx());
        ctrl.InsertColumn(me->active_, pos, ' ');
        assert(me->editor_);
        me->editor_->Update(me->model_->whats.size());
    } else if(me->layout_ == Vindow::Layout::WHAT) {
        auto found = std::find_if(me->model_->whats.begin(), me->model_->whats.end(), [me](WhatEntry& what) -> bool {
                    return what.name == me->active_;
                });
        if(found == me->model_->whats.end()) {
            l(L"%s not found\n", me->active_);
            return;
        }
        auto pos = found->columns.begin();
        std::advance(pos, me->editor_->mx());
        ctrl.InsertColumn(me->active_, pos, ' ');
        assert(me->editor_);
        me->editor_->Update(me->model_->whos.size());
    }
}

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
    me->SetLayout(Layout::WHAT, MB2W(label));
    me->redraw();
}

void Vindow::WhoClicked(Fl_Widget* w, void* p)
{
    Vindow* me = (Vindow*)p;
    const char* label = w->label();
    me->SetLayout(Layout::WHO, MB2W(label));
    me->redraw();
}

void Vindow::OutputClicked(Fl_Widget* w, void* p)
{
    Vindow* me = (Vindow*)p;
    me->SetLayout(Layout::OUTPUT);
    me->redraw();
}

void Vindow::WindowCallback(Fl_Widget* w, void* p)
{
    WindowClose(w, p);
}

void Vindow::WhoNameChanged(Fl_Widget* w, void* p)
{
    auto* inp = (Fl_Input*)w;
    auto* me = (Vindow*)p;

    std::wstring newName = MB2W(inp->value());
    std::wstring oldName = me->active_;

    if(std::any_of(me->model_->whos.begin(), me->model_->whos.end(), [newName](WhoEntry const& e) -> bool {
                    return e.name == newName;
                })
            || newName.empty())
    {
        fl_alert("Name needs to be unique and not null");
        inp->value(W2MB(oldName).get());
        // Fl::focus(inp); // doesn't work because e.g. the tab key is
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

    std::wstring newName = MB2W(inp->value());
    std::wstring oldName = me->active_;

    if(std::any_of(me->model_->whats.begin(), me->model_->whats.end(), [newName](WhatEntry const& e) -> bool {
                    return e.name == newName;
                })
            || newName.empty()
            || newName == L"OUTPUT")
    {
        fl_alert("Name needs to be unique and not null");
        inp->value(W2MB(oldName).get());
        // Fl::focus(inp); // doesn't work because e.g. the tab key is
                           // handled later...
        return;
    }

    Control ctrl(me->model_, me);
    ctrl.SetWhatsName(oldName, newName);
}

void Vindow::WhatBpmChanged(Fl_Widget* w, void* p)
{
    auto* inp = dynamic_cast<Fl_Input*>(w);
    auto* me = (Vindow*)p;

    auto&& what = me->active_;
    auto bpm = MB2W(inp->value());

    Control ctrl(me->model_, me);
    ctrl.SetWhatsBpm(what, bpm);
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
    auto* inp = dynamic_cast<Fl_Input*>(w);
    auto* me = (Vindow*)p;

    auto&& who = me->active_;
    std::wstring param = MB2W(inp->label());
    std::wstring value = MB2W(inp->value());

    Control ctrl(me->model_, me);
    ctrl.SetWhosParam(who, param, value);
}
