#pragma once

#include "header.hpp"

namespace Graal {

namespace level_editor {

class window;

class edit_commands {
public:
  edit_commands(window& _window, header& _header);
protected:
  void on_action_prefs();
  void on_action_undo();
  void on_action_redo();
  void on_action_cut();
  void on_action_copy();
  void on_action_paste();
  void on_action_delete();

  window& m_window;
};

}

}
