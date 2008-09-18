#ifndef GRAAL_LEVEL_EDITOR_TILE_OBJECTS_DISPLAY_HPP_
#define GRAAL_LEVEL_EDITOR_TILE_OBJECTS_DISPLAY_HPP_

#include <gtkmm.h>
#include "preferences.hpp"
#include "tiles_display.hpp"

namespace Graal {
  namespace level_editor {
    class tile_objects_display: public Gtk::VBox {
    public:
      tile_objects_display(level_editor::preferences& preferences);

      void set();
      void get();

      // Small wrapper around tiles_display
      void set_tile_size(int tile_width, int tile_height);
      void set_tileset_surface(const Cairo::RefPtr<Cairo::Surface>& surface);

      typedef sigc::signal<void, tile_buf&, int, int> signal_tiles_selected_type;
      signal_tiles_selected_type& signal_tiles_selected();
      typedef sigc::signal<tile_buf> signal_create_tile_object_type;
      signal_create_tile_object_type& signal_create_tile_object();
      
      void on_mouse_pressed(GdkEventButton* event);
    protected:
      Gtk::ComboBoxEntryText m_groups;
      Gtk::ComboBoxEntryText m_objects;

      Gtk::Label m_group_label;
      Gtk::Button m_group_new;
      Gtk::Button m_group_delete;
      Gtk::Button m_group_save;

      Gtk::Label m_object_label;
      Gtk::Button m_object_new;
      Gtk::Button m_object_delete;

      tiles_display m_display;

      preferences& m_preferences;

      void on_group_changed();
      void on_object_changed();

      void on_group_new_clicked();
      void on_group_delete_clicked();
      void on_group_save_clicked();
      void on_object_new_clicked();
      void on_object_delete_clicked();

      signal_tiles_selected_type m_signal_tiles_selected;
      signal_create_tile_object_type m_signal_create_tile_object;
    };
  }
}

#endif
