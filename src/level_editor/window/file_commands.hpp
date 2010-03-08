#pragma once

#include <gtkmm.h>

#include "header.hpp"
#include "preferences.hpp"

namespace Graal {

namespace level_editor {

class window;

class file_commands {
public:
  file_commands(window& _window, header& _header, preferences& _preferences);

protected:
  void on_action_new();
  void on_action_open();
  void on_action_save();
  void on_action_save_as();
  void on_action_quit();

  window& m_window;
  preferences& m_preferences;

  // FileChooserDialogs to use
  Gtk::FileChooserDialog m_fc_open;
};

}

}
