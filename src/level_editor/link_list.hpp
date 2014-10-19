#ifndef GRAAL_LEVEL_EDITOR_LINK_LIST_HPP_
#define GRAAL_LEVEL_EDITOR_LINK_LIST_HPP_

#include <gtkmm.h>
#include "level.hpp"

namespace Graal {
  namespace level_editor {
    class window;
    class edit_link: public Gtk::Dialog {
    public:
      edit_link(window& _window);
      virtual ~edit_link();

      void get(const link& _link);
      link get_link();
    protected:
      window& m_window;

      Gtk::Entry m_edit_x;
      Gtk::Entry m_edit_y;
      Gtk::Entry m_edit_width;
      Gtk::Entry m_edit_height;

      Gtk::Entry m_edit_destination;
      Gtk::Entry m_edit_new_x;
      Gtk::Entry m_edit_new_y;
    };

    class link_list: public Gtk::Dialog {
    public:
      link_list(window& _window);

      void get();
    protected:
      void on_show();
      void on_response(int response_id);
      void on_edit_clicked();
      void on_delete_clicked();

      window& m_window;
      Glib::RefPtr<Gtk::ListStore> m_list_store;
      Gtk::TreeView m_tree_view;

      class link_columns: public Gtk::TreeModelColumnRecord {
      public:
        link_columns() {
          add(iter); add(destination); add(new_x); add(new_y);
        }
        virtual ~link_columns();

        Gtk::TreeModelColumn<level::link_list_type::iterator> iter;
        Gtk::TreeModelColumn<Glib::ustring> destination;
        Gtk::TreeModelColumn<Glib::ustring> new_x;
        Gtk::TreeModelColumn<Glib::ustring> new_y;
      };

    public:
      link_columns columns;
    };
  }
}

#endif

