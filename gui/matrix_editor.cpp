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
#include "logger.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scrollbar.H>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cwchar>
#include <cwctype>

#define MYFONT \
    fl_font(FL_COURIER_BOLD, 16)

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
        int cursorx,
        int cursory)
    : BASE(x, y, w, h)
    , first_(first)
    , last_(last)
    , active_(false)
    , mx_(1)
    , my_(1)
    , cursorx_(cursorx - 1)
    , cursory_(cursory - 1)
{
    LOGGER(l);
    int dx = Fl::box_dx(FL_DOWN_BOX),
        dy = Fl::box_dy(FL_DOWN_BOX),
        dw = Fl::box_dw(FL_DOWN_BOX),
        dh = Fl::box_dh(FL_DOWN_BOX);

    begin();
      MYFONT;
      sb1 = new Fl_Scrollbar(
              x + dx,
              y + dy + h - dh - Fl::scrollbar_size(),
              w - dw - Fl::scrollbar_size(),
              Fl::scrollbar_size()
           );
      sb1->type(FL_HORIZONTAL);
      auto sb1full = std::max<ptrdiff_t>(1, std::distance(first, last));
      auto sb1window = std::min(
              sb1full,
              (decltype(sb1full))std::ceil(sb1->w() / fl_width("X")));
      windowx_ = sb1window;
      auto sb1start = std::min<int>(sb1full, mx_);
      sb1->value(sb1start, sb1window, 1, sb1full);
      l("sb1 bounds: %d %ld %d %ld\n", sb1start, sb1window, 1, sb1full);
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
      windowy_ = sb2window;
      auto sb2start = std::min<int>(sb2full, my_);
      l("sb2 bounds: %d %ld %d %ld\n", sb2start, sb2window, 1, sb2full);
      sb2->value(sb2start, sb2window, 1, sb2full);
      resizable(new Fl_Box(sb1->x(), sb2->y(), sb1->w(), sb2->h()));
    end();
}

MatrixEditor::~MatrixEditor()
{}

bool MatrixEditor::IsSelected(int i, int j) const
{
    return cursory_ == i
        && cursorx_ == j
        ;
}

void MatrixEditor::draw()
{
    LOGGER(l);
    fl_draw_box(FL_DOWN_BOX, this->x(), this->y(), this->w(), this->h(), FL_BACKGROUND2_COLOR);

    l("active_=%d\n", active_);

    MYFONT;
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

    auto xFull = std::distance(first_, last_);
    auto xWindow = windowx_;
    assert(mx_ - 1 <= xFull);
    auto it = first_;
    std::advance(it, mx_ - 1);
    auto it_end = it;
    int count = xWindow;
    while(count && it_end != last_)
        ++it_end, --count;

    size_t size = (it != it_end)
        ? it->rows.size()
        : 0
        ;
    size_t yWindow = windowy_;
    l("size=%ld, ywindow=%ld, xwindow=%d\n", size, yWindow, xWindow);
    int j = 0;
    for(; it != it_end; ++it, ++j) {
        size_t i = my_ - 1;
        auto ri = it->rows.begin();
        auto condition = [=](size_t i) -> bool {
            return i < size
                && i < yWindow + my_ - 1
                ;
        };
        for(; condition(i); ++i, ++ri) {
            l("%ld,%d:%c ", i, j, **ri);
            cd.draw(i, j, IsSelected(i, j), **ri);
        }
        l("\n");
    }

    fl_draw_box(FL_FLAT_BOX, sb1->x() + sb1->w(), sb2->y() + sb2->h(), Fl::scrollbar_size(), Fl::scrollbar_size(), FL_BACKGROUND_COLOR);

    l("drawing scrollbars\n");

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
                if(px < x() + Fl::box_dx(FL_DOWN_BOX)
                        || px >= x() + w() - Fl::scrollbar_size() - Fl::box_dw(FL_DOWN_BOX)
                        || py < y() + Fl::box_dy(FL_DOWN_BOX)
                        || py >= y() + h() - Fl::scrollbar_size() - Fl::box_dh(FL_DOWN_BOX))
                    break;
                int btn = Fl::event_button();
                Fl::focus(this);

                px -= sb1->x();
                py -= sb2->y();
                MYFONT;
                int cx = fl_width("X");
                int cy = fl_height();
                int j = px / cx;
                int i = py / cy;
                LOGGER(l);
                l("Click at %d,%d, adjusted:%d,%d; cell size %d,%d; world: %d,%d\n", Fl::event_x(), Fl::event_y(), px, py, cx, cy, i, j);
                if(i < windowx_
                        && j < windowy_)
                {
                    l("cursor now at %d,%d\n", i, j);
                    cursorx_ = j;
                    cursory_ = i;
                }
                
                goto FL_FOCUS_;
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
                LOGGER(l);
                int toDel = 0;
                if(!Fl::compose(toDel)) {
                    l("Not composed: %04X\n", Fl::event_key());
                    switch(Fl::event_key())
                    {
                    case FL_Right:
                        cursorx_ = std::min(cursorx_ + 1, windowx_ - 1);
                        l("move right @%d,%d\n", cursory_, cursorx_);
                        redraw();
                        return 1;
                    case FL_Left:
                        cursorx_ = std::max(cursorx_ - 1, 0);
                        l("move left @%d,%d\n", cursory_, cursorx_);
                        redraw();
                        return 1;
                    case FL_Down:
                        cursory_ = std::min(cursory_ + 1, windowy_ - 1);
                        l("move down @%d,%d\n", cursory_, cursorx_);
                        redraw();
                        return 1;
                    case FL_Up:
                        cursory_ = std::max(cursory_ - 1, 0);
                        l("move up @%d,%d\n", cursory_, cursorx_);
                        redraw();
                        return 1;
                    }
                    l("to parent!\n");
                    break;
                }
                l("composed\n");
                if(toDel) {
                    // delete toDel characters to the left
                    // and insert Fl::event_text(), Fl::event_length()
                    l("don't know how to delete %d chars to the left\n", toDel);
                }
                if(Fl::event_state(FL_META|FL_CONTROL|FL_ALT)) {
                    l("modifier pressed, passing to parent\n");
                    break;
                }

                wchar_t wcs[4];
                size_t len = mbstowcs(wcs, Fl::event_text(), 2);
                if(len > 1) {
                    l("%ls is more than one character, not supported\n", wcs);
                    break;
                }
                if(len <= 0
                        || !iswprint(wcs[0])
                        || wcs[0] == L'\t') {
                    l("non printable, to parent\n");
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
