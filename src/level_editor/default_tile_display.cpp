#include "default_tile_display.hpp"
#include "window.hpp"
#include "helper.hpp"

using namespace Graal;

level_editor::default_tile_display::default_tile_display() {
  m_tile_buf.resize(1, 1);
}

tile_buf& level_editor::default_tile_display::get_tile_buf() {
  return m_tile_buf;
}

bool level_editor::default_tile_display::on_expose_event(GdkEventExpose* event) {
  Glib::RefPtr<Gdk::Window> window = get_window();

  if (!window)
    return true;
  if (!m_surface)
    set_surface_buffers();

  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
  cr->set_source(m_surface, 0, 0);
  cr->paint();

  return false;
}
