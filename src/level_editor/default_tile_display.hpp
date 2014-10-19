#ifndef GRAAL_LEVEL_EDITOR_DEFAULT_TILE_DISPLAY_HPP_
#define GRAAL_LEVEL_EDITOR_DEFAULT_TILE_DISPLAY_HPP_

#include <gtkmm.h>
#include "basic_tiles_display.hpp"

namespace Graal {
  namespace level_editor {
    class window;
    class default_tile_display: public basic_tiles_display {
    public:
      default_tile_display();
      virtual ~default_tile_display() {}

      void set_tile(int index) {
        m_tile_buf.get_tile(0, 0).index = index;
        update_all();
      }

      int get_tile() const {
        return m_tile_buf.get_tile(0, 0).index;
      }

      tile_buf& get_tile_buf();
    protected:
      virtual bool on_expose_event(GdkEventExpose* event);
      tile_buf m_tile_buf;
    };
  }
}

#endif
