//
// "$Id: Fl_key_name.cxx,v 1.12 2004/01/19 21:38:41 spitzak Exp $"
//
// Turn a fltk (X) keysym + fltk shift flags into a human-readable string.
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

#include <fltk/events.h>
#include <fltk/Widget.h>
#include <fltk/draw.h>
#include <ctype.h>
#include <string.h>
#ifndef _WIN32
#include <fltk/x.h>
#endif
using namespace fltk;

#if !defined(_WIN32) && !defined(__APPLE__)
# define USE_X11 1
#endif

// This table must be in numeric order by fltk (X) keysym number.
// On X the table is much shorter as it is only the names that
// are not returned correctly by XKeysymToString().
struct Keyname {int key; const char* name;};
static Keyname table[] = {
#if !USE_X11
  {BackSpaceKey, "Backspace"},
  {TabKey,	"Tab"},
  {ClearKey,	"Clear"},
  {ReturnKey,	"Return"}, // older fltk said "Enter"
  {PauseKey,	"Pause"},
  {ScrollLockKey,"ScrollLock"},
  {EscapeKey,	"Escape"},
  {HomeKey,	"Home"},
  {LeftKey,	"Left"},
  {UpKey,	"Up"},
  {RightKey,	"Right"},
  {DownKey,	"Down"},
#endif
  {PageUpKey,	"PageUp"}, // X says "Prior"
  {PageDownKey,	"PageDown"}, // X says "Next"
#if !USE_X11
  {EndKey,	"End"},
  {PrintKey,	"Print"},
  {InsertKey,	"Insert"},
  {fltk::MenuKey,"Menu"},
  {NumLockKey,	"NumLock"},
  {KeypadEnter,	"KeypadEnter"},
  {LeftShiftKey,"LeftShift"},
  {RightShiftKey,"RightShift"},
  {LeftCtrlKey, "LeftCtrl"},
  {RightCtrlKey,"RightCtrl"},
  {CapsLockKey,	"CapsLock"},
  {LeftMetaKey,	"LeftMeta"},
  {RightMetaKey,"RightMeta"},
  {LeftAltKey,	"LeftAlt"},
  {RightAltKey,	"RightAlt"},
  {DeleteKey,	"Delete"}
#endif
};

/*!
  Unparse a key symbol such as fltk::SpaceKey optionally or'd with
  shift flags such as fltk::SHIFT, into a human-readable string like
  "Alt+N". If the shortcut is zero an empty string is returned. The
  return value points at a static buffer that is overwritten with each
  call.
*/
const char* fltk::key_name(int shortcut) {
  static char buf[20];
  char *p = buf;
  if (!shortcut) {*p = 0; return buf;}
  if (shortcut & META) {strcpy(p,"Meta+"); p += 5;}
  if (shortcut & ALT) {strcpy(p,"Alt+"); p += 4;}
  if (shortcut & SHIFT) {strcpy(p,"Shift+"); p += 6;}
  if (shortcut & CTRL) {strcpy(p,"Ctrl+"); p += 5;}
  int key = shortcut & 0xFFFF;

  // binary search the table for a match:
  int a = 0;
  int b = sizeof(table)/sizeof(*table);
  const char* q = 0;
  while (a < b) {
    int c = (a+b)/2;
    if (table[c].key == key) {q = table[c].name; break;}
    if (table[c].key < key) a = c+1;
    else b = c;
  }
  if (!q) {
#if USE_X11
    if (key <= 32 || key >= 0x100) q = XKeysymToString(key);
#else
    if (key >= F0Key && key <= LastFunctionKey) {
      *p++ = 'F';
      if (key > F9Key) *p++ = (key-F0Key)/10+'0';
      *p++ = (key-F0Key)%10 + '0';
      *p = 0;
      return buf;
    }
    if (key >= Keypad && key <= KeypadLast) {
      // mark keypad keys with Keypad prefix
      strcpy(p,"Keypad"); p += 6;
      *p++ = uchar(key & 127);
      *p = 0;
      return buf;
    }
#endif
  }
  if (q) {
    if (p == buf) return q;
    strcpy(p, q);
    return buf;
  }
  // if all else fails use the keysym as a character:
  *p++ = uchar(key);
  *p = 0;
  return buf;
}

//
// End of "$Id: Fl_key_name.cxx,v 1.12 2004/01/19 21:38:41 spitzak Exp $"
//
