#include "window.h"

#include <FL/fl_ask.H>

#include <cassert>

void Window::FileNew(Fl_Widget*, void*)
{}

void Window::FileOpen(Fl_Widget*, void*)
{}

void Window::FileSave(Fl_Widget*, void*)
{}

void Window::FileSaveAs(Fl_Widget*, void*)
{}

void Window::FileReload(Fl_Widget*, void*)
{}

void Window::FileExit(Fl_Widget*, void*)
{
    extern bool is_any_model_dirty();
    if(is_any_model_dirty())
    {
        int which = fl_choice("There are unsaved documents.\nAre you sure you want to exit?\n(Hint: use Window/Close to close just one window)", "&No", nullptr, "&Yes");
        if(which != 2) return;
    }
    exit(0);
}

void Window::EditUndo(Fl_Widget*, void*)
{}

void Window::EditCut(Fl_Widget*, void*)
{}

void Window::EditCopy(Fl_Widget*, void*)
{}

void Window::EditPaste(Fl_Widget*, void*)
{}

void Window::EditOverwrite(Fl_Widget*, void*)
{}

void Window::EditInsertRest(Fl_Widget*, void*)
{}

void Window::EditInsertBlank(Fl_Widget*, void*)
{}

void Window::EditClearColumns(Fl_Widget*, void*)
{}

void Window::EditDeleteColumns(Fl_Widget*, void*)
{}

void Window::EditAddWhat(Fl_Widget*, void*)
{}

void Window::EditAddWho(Fl_Widget*, void*)
{}

void Window::EditDeleteSection(Fl_Widget*, void*)
{}

void Window::WindowNew(Fl_Widget*, void*)
{}

void Window::WindowClose(Fl_Widget*, void* p)
{
    extern void destroy_window(Window*);
    Window* me = (Window*)p;
    if(me->model_->dirty
            && me->model_->views.size() == 1)
    {
        int which = fl_choice("There are unsaved changes in the document.\nClosing this window will discard all changes.\nDo you want to close this document?", "&No", nullptr, "&Yes");
        if(which != 2) return;
    }
    return destroy_window(me);
}

void Window::WindowCloseAll(Fl_Widget*, void* p)
{
    extern void destroy_window(Window*);
    Window* me = (Window*)p;
    if(me->model_->dirty)
    {
        int which = fl_choice("There are unsaved changes in the document.\nDo you want to close this document, discarding changes?", "&No", nullptr, "&Yes");
        if(which != 2) return;
    }
    for(auto* view : me->model_->views)
    {
        auto* w = dynamic_cast<Window*>(view);
        if(w == me) continue;
        if(w) destroy_window(w);
    }
    return destroy_window(me);
}

void Window::WindowCallback(Fl_Widget* w, void* p)
{
    WindowClose(w, p);
}

