#include "tiles_display.hpp"

using namespace Graal;

bool level_editor::tiles_display::on_expose_event(GdkEventExpose* event) {
  Glib::RefPtr<Gdk::Window> window = get_window();

  if (!window || !m_surface)
    return true;

  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
  cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
  cr->clip();

  cr->set_source(m_surface, 0, 0);
  cr->paint();

  return true;
}
