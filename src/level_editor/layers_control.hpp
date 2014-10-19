#pragma once

#include <gtkmm.h>
#include "preferences.hpp"
#include "level_display.hpp"

namespace Graal {
  namespace level_editor {
    class window;

    class layers_control: public Gtk::HBox {
    public:
      // TODO: window is currently required to be initialized on construction
      layers_control(window& win, preferences& prefs);
      ~layers_control();
    protected:
      void on_layer_changed();
      void on_switch_level_display(level_display& display);
      void on_layer_visible_toggled();
      void on_add_layer();
      void on_delete_layer();

      preferences& m_preferences;
      window& m_window;

      Gtk::SpinButton m_spin_layer;
      Gtk::CheckButton m_layer_visible;
      Gtk::Button m_button_new_layer, m_button_delete_layer;
    };
  }
}
