#pragma once

#include <gtkmm.h>
#include "level.hpp"

namespace Graal {

namespace level_editor {

class edit_npc: public Gtk::Dialog {
public:
  edit_npc();
  virtual ~edit_npc();

  void set(const npc& _npc);
  npc get_npc();
protected:
  Graal::npc m_npc;

  Gtk::Entry m_edit_image;
  Gtk::Entry m_edit_x;
  Gtk::Entry m_edit_y;

  GtkWidget* m_view;
};

}

}
