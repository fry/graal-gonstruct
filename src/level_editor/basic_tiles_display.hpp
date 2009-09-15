#ifndef GRAAL_BASIC_TILES_DISPLAY_HPP_
#define GRAAL_BASIC_TILES_DISPLAY_HPP_

#include <gtkmm.h>
#include "tileset_display.hpp"

namespace Graal {
  namespace level_editor {
    class basic_tiles_display: public Gtk::DrawingArea {
    public:
      basic_tiles_display() {}
      basic_tiles_display(int tile_width, int tile_height);

      void update_all();
      void update_tiles(int x, int y, int width, int height);
      void update_tile(Cairo::RefPtr<Cairo::Context>& ct, int x, int y);
      virtual void update_tile(Cairo::RefPtr<Cairo::Context>& ct, const tile& _tile, int x, int y);
      

      // Calls update_all
      void set_tile_size(int tile_width, int tile_height);
      // Calls update_all
      void set_tileset_surface(const Cairo::RefPtr<Cairo::Surface>& surface);
      // Doesn't call update_all
      void set_surface_buffers();

      // Calls update_all
      void clear();

      virtual tile_buf& get_tile_buf() = 0;

      virtual ~basic_tiles_display() {}
    protected:
      int m_tile_width, m_tile_height;

      //virtual bool on_expose_event(GdkEventExpose* event);

      Cairo::RefPtr<Cairo::Surface> m_surface;
      Cairo::RefPtr<Cairo::Surface> m_tileset_surface;
    };
  }
}

#endif
