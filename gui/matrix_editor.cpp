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

#include <cmath>
#include <cstdlib>
#include <cwchar>
#include <cwctype>

namespace {
    struct CellDrawer
    {
        int x, y, cx, cy;
        Fl_Color colors[2][2];

        CellDrawer(
                int x_, int y_,
                int cx_, int cy_,
                Fl_Color fg_, Fl_Color bg_, Fl_Color sl_)
            : x(x_)
            , y(y_)
            , cx(cx_)
            , cy(cy_)
        {
            colors[false][0] = bg_;
            colors[false][1] = fl_contrast(fg_, bg_);
            colors[true][0] = sl_;
            colors[true][1] = fl_contrast(fg_, sl_);
        }

        void draw(int i, int j, bool selected, char c)
        {
            fl_draw_box(FL_FLAT_BOX, x + j * cx, y + i * cy, cx, cy, colors[selected][0]);
            fl_color(colors[selected][1]);
            char str[2] = { c, '\0' };
            fl_draw(str, x + j * cx, y + i * cy, cx, cy, 0, nullptr, false);
        }

    };
} // namespace

MatrixEditor::MatrixEditor(
        int x,
        int y,
        int w,
        int h,
        column_p_t first,
        column_p_t last,
        int mx,
        int my)
    : BASE(x, y, w, h)
    , first_(first)
    , last_(last)
    , active_(false)
    , mx_(mx)
    , my_(my)
{
    int dx = Fl::box_dx(FL_DOWN_BOX),
        dy = Fl::box_dy(FL_DOWN_BOX),
        dw = Fl::box_dw(FL_DOWN_BOX),
        dh = Fl::box_dh(FL_DOWN_BOX);

    begin();
      fl_font(FL_SCREEN_BOLD, 18);
      sb1 = new Fl_Scrollbar(
              x + dx,
              y + dy + h - dh - Fl::scrollbar_size(),
              w - dw - Fl::scrollbar_size(),
              Fl::scrollbar_size()
           );
      sb1->type(FL_HORIZONTAL);
      auto sb1full = std::min<ptrdiff_t>(1, std::distance(first, last));
      auto sb1window = std::min(
              sb1full,
              (decltype(sb1full))std::ceil(sb1->w() / fl_width("X")));
      auto sb1start = std::max<int>(sb1full, mx);
      sb1->value(sb1start, sb1window, 1, sb1full);
      sb2 = new Fl_Scrollbar(
              x + dx + w - dw - Fl::scrollbar_size(),
              y + dy,
              Fl::scrollbar_size(),
              h - dh - Fl::scrollbar_size()
           );
      auto sb2full = (first_ != last_)
          ? first_->rows.size()
          : (size_t)1
          ;
      auto sb2window = std::min(
              sb2full,
              (size_t)std::ceil(sb2->h() / fl_height()));
      auto sb2start = std::max<int>(sb2full, my);
      sb2->value(sb2start, sb2window, 1, sb2full);
      resizable(new Fl_Box(sb1->x(), sb2->y(), sb1->w(), sb2->h()));
    end();
}

MatrixEditor::~MatrixEditor()
{}

void MatrixEditor::draw()
{
    fl_draw_box(FL_DOWN_BOX, this->x(), this->y(), this->w(), this->h(), FL_BACKGROUND2_COLOR);

    fl_font(FL_SCREEN_BOLD, 18);
    auto selectionColor = active_
        ? FL_SELECTION_COLOR
        : FL_INACTIVE_COLOR
        ;
    int  cx = fl_width('X'),
         cy = fl_height(),
         x = this->x() + Fl::box_dx(FL_DOWN_BOX),
         y = this->y() + Fl::box_dy(FL_DOWN_BOX),
         w = sb2->x() - this->x(),
         h = sb1->y() - this->y();

    CellDrawer cd(x, y, cx, cy, FL_FOREGROUND_COLOR, FL_BACKGROUND2_COLOR, selectionColor);

    cd.draw(0, 0, false, 'X');
    cd.draw(0, 1, false, 'Y');
    cd.draw(0, 2, true, 'Z');
    cd.draw(1, 2, true, ' ');

    fl_draw_box(FL_FLAT_BOX, sb1->x() + sb1->w(), sb2->y() + sb2->h(), Fl::scrollbar_size(), Fl::scrollbar_size(), FL_BACKGROUND_COLOR);

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
