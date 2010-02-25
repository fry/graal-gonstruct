#include <GL/glew.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#include <GL/glu.h>

#include <gtkmm.h>
#include <gtkglmm.h>

#include <iostream>

#include "level_display.hpp"
#include "image_cache.hpp"
#include "preferences.hpp"
#include "filesystem.hpp"

#include <boost/filesystem.hpp>

int main(int argc, char** argv) {
  Gtk::Main kit(argc, argv);
  Gtk::GL::init(argc, argv);

  int major, minor;
  Gdk::GL::query_version(major, minor);
  std::cout << "OpenGL extension version - "
            << major << "." << minor << std::endl;

  // Try double-buffered visual


  Graal::level_editor::preferences prefs;

  boost::filesystem::path preferences_path;
  preferences_path = Glib::get_user_config_dir();
  preferences_path = preferences_path / "gonstruct";
  
  prefs.load(preferences_path / "preferences");

  
  Graal::level_editor::filesystem fs(prefs);
  fs.update_cache();
  Graal::level_editor::image_cache cache(fs);
  Graal::level_editor::level_display display(prefs, cache);
  Cairo::RefPtr<Cairo::ImageSurface> surf = Cairo::ImageSurface::create_from_png("D:\\Programme\\Graal4\\levels\\tiles\\pics1.png");
  //Cairo::RefPtr<Cairo::ImageSurface> surf = Cairo::ImageSurface::create_from_png("D:\\Programme\\Graal4\\pics1layers.png");
  display.set_tileset_surface(surf);
  /*Graal::tile_buf buf;
  buf.resize(64, 64);
  display.set_tile_buf(buf);*/
  display.load_level("D:\\Programme\\Graal4\\work\\#gscript\\fry_test.nw");
  //display.load_level("D:\\Programme\\Graal4\\work\\#gscript\\world_e07.nw");
  //display.load_level("D:\\Programme\\Graal4\\layers\\layers.nw");

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
