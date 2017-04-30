#include "window.h"

#include <FL/fl_ask.H>

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

void Window::WindowClose(Fl_Widget*, void*)
{}

void Window::WindowCloseAll(Fl_Widget*, void*)
{}

