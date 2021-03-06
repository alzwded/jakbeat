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
#ifndef MATRIX_EDITOR_H
#define MATRIX_EDITOR_H

#include "model.h"
#include "control.h"
#include <FL/Fl_Widget.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <cwchar>

class MatrixEditor
: public Fl_Group
{
    typedef Fl_Group BASE;
public:
    MatrixEditor(
            Control ctrl,
            int x,
            int y,
            int w,
            int h,
            std::wstring target,
            columns_t& columns,
            int nrows,
            int mx = 0,
            int my = 0);
    ~MatrixEditor() override;

    void draw() override;
    int handle(int) override;
    void resize(int, int, int, int) override;
    int mx() const;
    int my() const;
    void Update(int nrows);
    void Update() { Update(nrows_); }
    void SetTarget(std::wstring target);
    void SetCursor(int, int);

private:
    bool IsSelected(int, int) const;
    static void Scrolled(Fl_Widget*, void*);

private:
    Control ctrl_;
    std::wstring target_;
    columns_t& columns_;
    int nrows_;
    bool active_;
    Fl_Scrollbar* sb1,* sb2;
    int mx_, my_;
    int cursorx_, cursory_;
    int windowx_, windowy_;
};

#endif
