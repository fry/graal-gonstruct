#ifndef GRAAL_LEVEL_EDITOR_NPC_LIST_HPP_
#define GRAAL_LEVEL_EDITOR_NPC_LIST_HPP_

#include <gtkmm.h>
#include "level.hpp"

namespace Graal {
  namespace level_editor {
    class window;
    class edit_npc: public Gtk::Dialog {
    public:
      edit_npc();

      void set(const npc& _npc);
      npc get_npc();
    protected:
      Graal::npc m_npc;

      Gtk::Entry m_edit_image;
      Gtk::Entry m_edit_x;
      Gtk::Entry m_edit_y;

      GtkWidget* m_view;
    };

    class npc_list: public Gtk::Dialog {
    public:
      npc_list(window& _window);

      void get();
    protected:
      void on_show();
      void on_response(int response_id);
      void on_edit_clicked();
      void on_delete_clicked();

      window& m_window;
      Glib::RefPtr<Gtk::ListStore> m_list_store;
      Gtk::TreeView m_tree_view;

      class npc_columns: public Gtk::TreeModelColumnRecord {
      public:
        npc_columns() {
          add(iter); add(image); add(x); add(y);
        }

        Gtk::TreeModelColumn<level::npc_list_type::iterator> iter;
        Gtk::TreeModelColumn<Glib::ustring> image;
        Gtk::TreeModelColumn<int> x;
        Gtk::TreeModelColumn<int> y;
      };

    public:
      npc_columns columns;
    };
  }
}

#endif
