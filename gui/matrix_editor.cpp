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

#include "matrix_editor.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scrollbar.H>

#include <cstdlib>
#include <cwchar>
#include <cwctype>

MatrixEditor::MatrixEditor(
        int x,
        int y,
        int w,
        int h,
        column_p_t start,
        column_p_t end)
    : BASE(x, y, w, h)
    , start_(start)
    , end_(end)
    , active_(false)
#if 0
    , sb1(new Fl_Scrollbar(
            x,
            y + h - Fl::scrollbar_size(),
            w - Fl::scrollbar_size(),
            Fl::scrollbar_size()
         ))
    , sb2(new Fl_Scrollbar(
            x + w - Fl::scrollbar_size(),
            y,
            Fl::scrollbar_size(),
            h - Fl::scrollbar_size()
         ))
#endif
#if 0
    , sb1(
            x + Fl::box_dw(FL_DOWN_BOX),
            y - Fl::box_dy(FL_DOWN_BOX) + h - Fl::scrollbar_size(),
            w - Fl::box_dw(FL_DOWN_BOX),
            Fl::scrollbar_size()
         )
    , sb2(
            x - Fl::box_dw(FL_DOWN_BOX) + w - Fl::scrollbar_size(),
            y + Fl::box_dh(FL_DOWN_BOX),
            Fl::scrollbar_size(),
            h - Fl::box_dh(FL_DOWN_BOX)
         )
#endif
{
    begin();
    sb1 = new Fl_Scrollbar(
            x,
            y + h - Fl::scrollbar_size(),
            w - Fl::scrollbar_size(),
            Fl::scrollbar_size()
         );
    sb1->type(FL_HORIZONTAL);
    sb2 = new Fl_Scrollbar(
            x + w - Fl::scrollbar_size(),
            y,
            Fl::scrollbar_size(),
            h - Fl::scrollbar_size()
         );
    auto* b = new Fl_Box(x, y, w - Fl::scrollbar_size(), h - Fl::scrollbar_size());
    resizable(b);
    this->end();
}

MatrixEditor::~MatrixEditor()
{}

void MatrixEditor::draw()
{
    fl_draw_box(FL_DOWN_BOX, x(), y(), w(), h(), FL_BACKGROUND2_COLOR);

    fl_font(FL_SCREEN_BOLD, 18);
    auto selectionColor = active_
        ? FL_SELECTION_COLOR
        : FL_INACTIVE_COLOR
        ;
    fl_draw_box(FL_FLAT_BOX, x() + Fl::box_dx(FL_DOWN_BOX), y() + Fl::box_dy(FL_DOWN_BOX), fl_width('X'), fl_height(), selectionColor);
    fl_color(fl_contrast(FL_FOREGROUND_COLOR, selectionColor));
    fl_draw("X", x() + Fl::box_dx(FL_DOWN_BOX), y() + Fl::box_dy(FL_DOWN_BOX), fl_width("X"), fl_height(), 0, nullptr, false);
    //fl_draw_box(FL_FLAT_BOX, x() + Fl::box_dx(FL_DOWN_BOX) + fl_width("X"), y() + Fl::box_dy(FL_DOWN_BOX), fl_width("X"), fl_height(), FL_BACKGROUND2_COLOR);
    fl_color(fl_contrast(FL_FOREGROUND_COLOR, FL_BACKGROUND2_COLOR));
    fl_draw("Y", x() + Fl::box_dx(FL_DOWN_BOX) + fl_width("X"), y() + Fl::box_dy(FL_DOWN_BOX), fl_width("Y"), fl_height(), 0, nullptr, false);

    fl_draw_box(FL_FLAT_BOX, x() + w() - Fl::scrollbar_size(), y() + h() - Fl::scrollbar_size(), Fl::scrollbar_size() - Fl::box_dx(FL_DOWN_BOX), Fl::scrollbar_size() - Fl::box_dx(FL_DOWN_BOX), FL_BACKGROUND_COLOR);
    Fl_Widget* s = sb1;
    s->damage(damage());
    s->draw();
    s = sb2;
    s->damage(damage());
    s->draw();
}

int MatrixEditor::handle(int ev)
{
    switch(ev)
    {
        case FL_PUSH:
            {
                int px = Fl::event_x(),
                    py = Fl::event_y();
                if(px < x() + Fl::scrollbar_size()
                        || px >= x() + w() - Fl::scrollbar_size()
                        || py < y() + Fl::scrollbar_size()
                        || py >= y() + h() - Fl::scrollbar_size())
                    break;
                int btn = Fl::event_button();
                Fl::focus(this);
                goto FL_FOCUS_;
                // code
                return 1;
            }
        case FL_DRAG:
            break;
        case FL_RELEASE:
            break;
        case FL_MOVE:
            Fl::compose_reset();
            break;
        case FL_FOCUS:
FL_FOCUS_:  // code
            active_ = true;
            redraw();
            return 1;
        case FL_UNFOCUS:
            active_ = false;
            redraw();
            // code
            return 1;
        case FL_KEYBOARD:
            {
                int toDel = 0;
                if(!Fl::compose(toDel)) break;
                if(toDel) {
                    // delete toDel characters to the left
                    // and insert Fl::event_text(), Fl::event_length()
                    fprintf(stderr, "don't know how to delete %d chars to the left\n", toDel);
                }
                if(Fl::event_state(FL_META|FL_CONTROL|FL_ALT)) break;
                wchar_t wcs[4];
                size_t len = mbstowcs(wcs, Fl::event_text(), 2);
                if(len > 1) {
                    fprintf(stderr, "%ls is more than one character, not supported\n", wcs);
                    break;
                }
                if(len <= 0
                        || !iswprint(wcs[0])
                        || wcs[0] == L'\t') {
                    break;
                }
                fprintf(stderr, "Pushed %ls in editor\n", wcs);
            }
            return 1;
        case FL_PASTE:
            {
                auto* text = Fl::event_text();
                auto  len = Fl::event_length();
            }
            return 1;
    }
    return BASE::handle(ev);
}
