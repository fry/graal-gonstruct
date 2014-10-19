#ifndef GRAAL_LEVEL_EDITOR_SIGN_LIST_HPP_
#define GRAAL_LEVEL_EDITOR_SIGN_LIST_HPP_

#include <gtkmm.h>
#include "level.hpp"

namespace Graal {
  namespace level_editor {
    class window;

    class sign_list: public Gtk::Dialog {
    public:
      sign_list(window& _window);

      void get();
      void set();
    protected:
      void on_show();
      void on_response(int response_id);
      void on_create_clicked();
      void on_delete_clicked();
      void on_sign_changed();
      void on_x_changed();
      void on_y_changed();
      void on_text_changed();

      window& m_window;
      Glib::RefPtr<Gtk::ListStore> m_list_store;
      Gtk::TreeView m_tree_view;

      Gtk::TextView m_text_view;

      Gtk::Entry m_edit_x;
      Gtk::Entry m_edit_y;

      class sign_columns: public Gtk::TreeModelColumnRecord {
      public:
        sign_columns() {
          add(iter); add(index); add(x); add(y); add(text);
        }

        Gtk::TreeModelColumn<level::sign_list_type::iterator> iter;
        Gtk::TreeModelColumn<int> index;
        Gtk::TreeModelColumn<int> x;
        Gtk::TreeModelColumn<int> y;
        Gtk::TreeModelColumn<Glib::ustring> text;
      };

    public:
      sign_columns columns;
    };
  }
}

#endif
