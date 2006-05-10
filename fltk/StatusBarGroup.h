//
// "$Id"
//
// The Status StatusBarGroup is strip that can be put in the bottom edge of a Pack,
// usually it contains a status bar.
// it redims according to its parent width and keeps original given height
//
// Copyright 2002-2006 by Bill Spitzak and others.
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

#ifndef fltk_StatusStatusBarGroup_h
#define fltk_StatusStatusBarGroup_h

#include <fltk/Group.h>
#include <fltk/InvisibleBox.h>
#include <stdlib.h>
#include <string.h>

namespace fltk {

class FL_API StatusBarGroup : public Group {
public:
  StatusBarGroup(int X, int Y, int W, int H, const char* l=0) : Group(X, Y, W, H, l) {init(); }
  //! statusbar default constructor
  StatusBarGroup(int H=24) : Group(0, 0, 0, H, 0) {init(); }
  virtual ~StatusBarGroup();

  static NamedStyle* default_style;
  void layout();
  enum Position {SBAR_LEFT=0,SBAR_CENTER,SBAR_RIGHT};
  
  //! set a text in the status bar at a given alignment
  void set(const char * t, Position f=SBAR_RIGHT);
  //! set a formatable (printf-like) text in the status bar at a given alignment
  void set(Position f, const char * format, ... );
  //! set a default box to all texts  inside the status bar
  void child_box(Box* b) {for(int i=0;i<3;i++) child_box(b,(Position)i);}
  //! set a default box to text at particular position inside the status bar
  void child_box(Box* b, Position i) {b_[i]= b;if(tf_[i]) tf_[i]->box(b);}

protected:
  //! draw a label on a particular (left,middle, right) position 
  void draw_label(Position pos, const char * label);
  //! adapt box position and size according to pos and statusbar dims
  void update_box(InvisibleBox *b, Position pos);

private:
  void init();
  void resize_from_parent();
  
  // text fields up to three fields
  InvisibleBox* tf_[3];   // 3 position possible and cumulable left, middle, center fields
  Box* b_[3]; // box style for fields, default to no box
};

}
#endif

//
// End of "$Id"
//
