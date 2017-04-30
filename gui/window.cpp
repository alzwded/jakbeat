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

#include <FL/Fl_Menu_Item.H>

Window::Window(std::shared_ptr<Model> m, int w, int h, const char* t)
    : Fl_Double_Window(w, h, t)
    , View()
    , model_(m)
    , menu_(nullptr)
{
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
              0, (Fl_Callback*)EditDeleteSection, 0, FL_MENU_DIVIDER },
          { 0 },
        { "&Window",
            0, 0, 0, FL_SUBMENU },
          { "&New Window",
              FL_COMMAND + 'n', (Fl_Callback*)WindowNew },
          { "&Close",
              FL_COMMAND + 'w', (Fl_Callback*)WindowClose },
          { "Close &All",
              0, (Fl_Callback*)WindowCloseAll },
          { 0 },
        { 0 }
    };

    auto* mb = new Fl_Menu_Bar(0, 0, w, 30);
    mb->copy(menuitems);
    // init common components
}

Window::~Window()
{
    delete menu_;
}

void Window::OnEvent(Event* e)
{
}

void Window::SetLayout(Layout lyt)
{
    layout_ = lyt;
}
