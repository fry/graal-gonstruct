#include <GL/glew.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#include <GL/glu.h>

#include <gtkmm.h>
#include <gtkglmm.h>

#include <iostream>

#include "ogl_tiles_display.hpp"

int main(int argc, char** argv) {
  Gtk::Main kit(argc, argv);
  Gtk::GL::init(argc, argv);

  int major, minor;
  Gdk::GL::query_version(major, minor);
  std::cout << "OpenGL extension version - "
            << major << "." << minor << std::endl;

  // Try double-buffered visual


  Graal::level_editor::ogl_tiles_display display;
  Cairo::RefPtr<Cairo::ImageSurface> surf = Cairo::ImageSurface::create_from_png("D:\\Programme\\Graal4\\pics1layers.png");
  display.set_tileset_surface(surf);
  Graal::tile_buf buf;
  buf.resize(64, 64);
  display.set_tile_buf(buf);

  Gtk::Window wnd;
  wnd.set_reallocate_redraws(true);
  Gtk::ScrolledWindow scroll;
  scroll.add(display);
  Gtk::VBox vbox;
  wnd.add(vbox);
  vbox.pack_start(scroll);
  wnd.show_all();
  kit.run(wnd);
  return 0;
}
