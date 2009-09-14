#ifndef GRAAL_LEVEL_EDITOR_TILESET_DISPLAY_HPP_
#define GRAAL_LEVEL_EDITOR_TILESET_DISPLAY_HPP_

#include <gtkmm.h>
#include <boost/shared_ptr.hpp>
#include "level.hpp"
#include "tileset.hpp"
#include "preferences.hpp"
#include "image_cache.hpp"

namespace Graal {
  namespace level_editor {
    class window;

    class tileset_display: public Gtk::DrawingArea {
    public:
      tileset_display(preferences& _prefs, image_cache& cache);

      void set_tile_size(int tile_width, int tile_height);
      void update_tileset(const std::string& level_name);

      inline int to_tiles_x(int x);
      inline int to_tiles_y(int y);

      typedef sigc::signal<void, const std::string&> signal_status_update_type;
      signal_status_update_type& signal_status_update();

      typedef sigc::signal<void, int> signal_default_tile_changed_type;
      signal_default_tile_changed_type& signal_default_tile_changed();

      typedef sigc::signal<void, const Cairo::RefPtr<Cairo::Surface>&> signal_tileset_updated_type;
      signal_tileset_updated_type& signal_tileset_updated();

      typedef sigc::signal<void, tile_buf&, int, int> signal_tiles_selected_type;
      signal_tiles_selected_type& signal_tiles_selected();
    protected:
      virtual bool on_expose_event(GdkEventExpose* event);
      void on_button_pressed(GdkEventButton* event);
      void on_button_released(GdkEventButton* event);
      void on_button_motion(GdkEventMotion* event);
      void on_mouse_leave(GdkEventCrossing* event);

      void copy_selection();
      void reset_selection();

      preferences& m_preferences;
      image_cache& m_image_cache;
      int m_tile_width, m_tile_height;

      tile_buf m_selection;
      bool m_selecting;
      int m_select_x, m_select_y, m_select_end_x, m_select_end_y;

      Cairo::RefPtr<Cairo::ImageSurface> m_surface;
    private:
      signal_status_update_type m_signal_status_update;
      signal_default_tile_changed_type m_signal_default_tile_changed;
      signal_tileset_updated_type m_signal_tileset_updated;
      signal_tiles_selected_type m_signal_tiles_selected;
    };
  }
}

#endif
