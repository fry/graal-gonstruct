#ifndef GRAAL_LEVEL_EDITOR_npc_LIST_HPP_
#define GRAAL_LEVEL_EDITOR_npc_LIST_HPP_

#include <gtkmm.h>
#include "level.hpp"
#include "preferences.hpp"

namespace Graal {
  namespace level_editor {
    class window;
    class edit_tileset: public Gtk::Dialog {
    public:
      edit_tileset();

      void get(const tileset& _tileset);
      void set(tileset& _tileset);
    protected:
      void on_main_toggled();

      Gtk::Label m_label_image;
      Gtk::Label m_label_prefix;
      Gtk::Label m_label_x;
      Gtk::Label m_label_y;
      Gtk::Label m_label_main;

      Gtk::Entry m_edit_image;
      Gtk::Entry m_edit_prefix;
      Gtk::Entry m_edit_x;
      Gtk::Entry m_edit_y;
      Gtk::CheckButton m_check_main;

      Gtk::Table m_table;
    };

    class tileset_list: public Gtk::Dialog {
    public:
      tileset_list(window& _window, preferences& _preferences);

      void get();
    protected:
      void on_show();
      void on_response(int response_id);
      void on_edit_clicked();
      void on_delete_clicked();
      void on_new_clicked();

      window& m_window;
      preferences& m_preferences;
      Glib::RefPtr<Gtk::ListStore> m_list_store;
      Gtk::TreeView m_tree_view;

      class tileset_columns: public Gtk::TreeModelColumnRecord {
      public:
        tileset_columns() {
          add(iter); add(image); add(prefix); add(main); add(x); add(y);
        }

        Gtk::TreeModelColumn<tileset_list_type::iterator> iter;
        Gtk::TreeModelColumn<Glib::ustring> image;
        Gtk::TreeModelColumn<Glib::ustring> prefix;
        Gtk::TreeModelColumn<bool> main;
        Gtk::TreeModelColumn<int> x;
        Gtk::TreeModelColumn<int> y;
      };

    public:
      tileset_columns columns;
    };
  }
}

#endif
