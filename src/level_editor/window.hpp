#ifndef GRAAL_LEVEL_EDITOR_WINDOW_HPP_
#define GRAAL_LEVEL_EDITOR_WINDOW_HPP_

#include <gtkmm.h>
#include "level.hpp"
#include "level_display.hpp"
#include "tileset_display.hpp"
#include "filesystem.hpp"
#include "link_list.hpp"
#include "sign_list.hpp"
#include "npc_list.hpp"
#include "tileset_list.hpp"
#include "default_tile_display.hpp"
#include "preferences_display.hpp"
#include "copy_cache.hpp"
#include "tile_objects_display.hpp"

namespace Graal {
  namespace level_editor {
    class toolbar_tools_display;
    class layers_control;

    class window: public Gtk::Window {
    public:
      typedef std::map<std::string, tileset> tileset_map_type;

      void load_level(const boost::filesystem::path& file_path, bool activate = true);
      level_display* get_nth_level_display(int n);
      level_display* get_current_level_display();
      void set_current_page(const level_display& display);
      void create_new_page(level_display& display, const std::string& name, bool activate = true);

      void update_matching_level_displays(const std::string& prefix);
      void on_tileset_update(const Cairo::RefPtr<Cairo::ImageSurface>& surface);

      boost::shared_ptr<level>& get_current_level();
      void display_error(const Glib::ustring& message);

      void set_status(const std::string& text);
      void set_level_buttons(bool enabled = true);
      void tiles_selected(tile_buf& selection, int x, int y);
      void set_default_tile(int tile_index);

      tile_buf get_current_tile_selection();

      bool save_current_page();
      bool save_current_page_as();

      Cairo::RefPtr<Cairo::ImageSurface> get_image(const std::string& file_name);

      window(preferences& _prefs);

      virtual ~window();

      tileset_display display_tileset;
      default_tile_display default_tile;
      tileset_map_type tilesets;
      filesystem fs;
      boost::shared_ptr<basic_cache> copy_cache;

      typedef sigc::signal<void, level_display&> signal_switch_level_display_type;
      signal_switch_level_display_type& signal_switch_level_display();

    protected:
      class tab_label: public Gtk::HBox {
      public:
        tab_label(const Glib::ustring& label);
        typedef Glib::SignalProxy0<void> signal_proxy_close;

        signal_proxy_close close_event();
        void set_label(const Glib::ustring& label);
        void set_unsaved_status(bool status);
      protected:
        Gtk::Label m_label;
        Gtk::Label m_unsaved;
        Gtk::Button m_button;
      };

      image_cache m_image_cache;
      preferences& m_preferences;

      link_list m_link_list;
      sign_list m_sign_list;
      npc_list m_npc_list;
      tileset_list m_tileset_list;
      preferences_display m_prefs_display;
      toolbar_tools_display* m_tools;
      layers_control* m_layers_control;
      tile_objects_display m_tile_objects;

      Gtk::Statusbar m_status_bar;
      Gtk::Label m_status;

      int m_tile_width;
      int m_tile_height;

      virtual bool on_delete_event(GdkEventAny* event);

      void on_action_new();
      void on_action_open();
      void on_action_save();
      void on_action_save_as();
      void on_action_quit();
      void on_action_create_link();
      void on_action_links();
      void on_action_signs();
      void on_action_npcs();
      void on_action_tilesets();
#ifdef WIN32
      void on_action_play();
#endif
      void on_action_prefs();
      void on_action_about();
      void on_action_undo();
      void on_action_redo();
      void on_action_cut();
      void on_action_copy();
      void on_action_paste();
      void on_action_delete();
      void on_action_screenshot();
      void on_close_level_clicked(Gtk::ScrolledWindow& scrolled, level_display& display);
      void on_switch_page(GtkNotebookPage* page, guint page_num);
      void on_preferences_changed(preferences_display::preference_changes c);
      void on_tileset_expose_event(GdkEventExpose* event);

      bool close_all_levels();
      void update_cache();
      std::auto_ptr<level_display> create_level_display();

      boost::shared_ptr<level> m_level;

      Gtk::Notebook m_nb_levels;
      Gtk::Notebook m_nb_toolset;
      
      // FileChooserDialogs to use
      Gtk::FileChooserDialog m_fc_open;
      Gtk::FileChooserDialog m_fc_save;

      Glib::RefPtr<Gtk::ActionGroup> m_level_actions;
      Glib::RefPtr<Gtk::UIManager> m_ui;
      //Glib::RefPtr<Gdk::Pixbuf> m_logo;

      // True if we're in the middle of opening level[s] and don't want update_all spam
      bool m_opening_levels;
    private:
      signal_switch_level_display_type m_signal_switch_level_display;
    };
  }
}

#endif
