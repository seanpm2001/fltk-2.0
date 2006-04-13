// button.cxx (example2a)

#include <stdlib.h>
#include <stdio.h>
#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Button.h>
#include <fltk/InvisibleBox.h>
#include <fltk/TiledImage.h>
#include <fltk/SharedImage.h>
using namespace fltk;

void beepcb(Widget *, void *) {
  printf("\007"); fflush(stdout);
}

void exitcb(Widget *, void *) {
  exit(0);
}

int main(int argc, char ** argv) {
  register_images();
  Window *window = new Window(320,65);

  window->begin();
  InvisibleBox * ib = new InvisibleBox(0,0,window->w(),window->h());
  ib->image(new TiledImage("./images/bg.gif"));
  window->resizable(ib);

  Button *b1 = new Button(20, 20, 80, 25, "&Beep");
  b1->callback(beepcb,0);
  
  Button *b2 = new Button(120,20, 80, 25, "");
  b2->image(new TiledImage("./images/coucou.png"));

  Button *b3 = new Button(220,20, 80, 25, "E&xit");
  b3->callback(exitcb,0);
  window->end();
  window->show(argc,argv);
  return run();
}

//
// End of "$Id$".
//
