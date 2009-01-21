#ifndef GRAAL_LEVEL_EDITOR_TILES_DISPLAY_HPP_
#define GRAAL_LEVEL_EDITOR_TILES_DISPLAY_HPP_

#include "basic_tiles_display.hpp"
#include "level.hpp"
#include <iostream>

namespace Graal {
  namespace level_editor {
    class tiles_display: public basic_tiles_display {
    public:
      virtual tile_buf& get_tile_buf() { return m_tile_buf; }

      void set_tile_buf(tile_buf& buf) {
        m_tile_buf.swap(buf);
        set_surface_buffers();
        update_all();
        queue_draw();
      }
    protected:
      tile_buf m_tile_buf;

      virtual bool on_expose_event(GdkEventExpose* event) {
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
    };
  }
}

#endif
