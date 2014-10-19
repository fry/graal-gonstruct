#pragma once

#include <gtkmm.h>
#include "preferences.hpp"
#include "level_display.hpp"

namespace Graal {
  namespace level_editor {
    class window;

    class toolbar_tools_display: public Gtk::VBox {
    public:
      toolbar_tools_display(window& win, preferences& prefs);
      virtual ~toolbar_tools_display();
    protected:
      void on_hide_npcs_toggled();
      void on_hide_signs_toggled();
      void on_hide_links_toggled();
      void on_new_npc_clicked();
      void on_fade_layers_toggled();

      preferences& m_preferences;
      window& m_window;

      Gtk::CheckButton m_hide_npcs, m_hide_signs, m_hide_links, m_fade_layers;
      Gtk::Button m_button_new_npc;
    };
  }
}
