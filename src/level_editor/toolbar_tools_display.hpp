#pragma once

#include <gtkmm.h>
#include "preferences.hpp"
#include "level_display.hpp"

namespace Graal {
  namespace level_editor {
    class window;
    class toolbar_tools_display: public Gtk::VBox {
    public:
      // TODO: window is currently required to be initialized on construction
      toolbar_tools_display(window& win, preferences& prefs);
    protected:
      void on_hide_npcs_toggled();
      void on_hide_signs_toggled();
      void on_hide_links_toggled();
      void on_layer_changed();
      void on_switch_level_display(level_display& display);
      void on_new_npc_clicked();
      void on_layer_visible_toggled();

      preferences& m_preferences;
      window& m_window;

      Gtk::SpinButton m_spin_layer;
      Gtk::CheckButton m_hide_npcs, m_hide_signs, m_hide_links, m_layer_visible;
      Gtk::Button m_button_new_npc;
    };
  }
}
