//
// "$Id: Fl_Check_Button.cxx,v 1.44 2003/11/04 08:10:58 spitzak Exp $"
//
// Check button widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <fltk/CheckButton.h>
#include <fltk/Group.h>
#include <fltk/Box.h>
#include <fltk/draw.h>

using namespace fltk;

static void default_glyph(int glyph,
			  int x,int y,int w,int h,
			  const Style* style, Flags flags)
{
  Box* box = style->box();
  box->draw(x, y, w, h, style, flags|OUTPUT);
  box->inset(x, y, w, h);
  if (flags & VALUE) {
    setcolor(inactive(style->textcolor(), flags));
    if (h < 4) {fillrect(x+w/2-1,y+h/2-1,2,2); return;}
    x += 1;
    w = h - 2;
    int d1 = w/3;
    int d2 = w-d1;
    y += (h+d2)/2-d1-2;
    for (int n = 0; n < 3; n++, y++) {
      drawline(x, y, x+d1, y+d1);
      drawline(x+d1, y+d1, x+w-1, y+d1-d2+1);
    }
  }
}

void CheckButton::draw() {
  Button::draw(0, int(textsize())+2);
}

static void revert(Style* s) {
  s->buttonbox_ = NO_BOX;
  //s->box_ = DOWN_BOX;
  s->glyph_ = ::default_glyph;
}
static NamedStyle style("Check_Button", revert, &CheckButton::default_style);
NamedStyle* CheckButton::default_style = &::style;

CheckButton::CheckButton(int x, int y, int w, int h, const char *l)
  : Button(x, y, w, h, l)
{
  style(default_style);
  type(TOGGLE);
  set_flag(ALIGN_LEFT|ALIGN_INSIDE);
}
