#pragma once

#include <gtkmm.h>
#include "window/header.hpp"
#include "link_list.hpp"
#include "sign_list.hpp"
#include "npc_list.hpp"
#include "tileset_list.hpp"

namespace Graal {

namespace level_editor {

class window;
class level_display;

class level_commands {
public:
  level_commands(window& _window, header& _header, preferences& _preferences);

  void hide_lists();
protected:
  void on_action_create_link();
  void on_action_links();
  void on_action_signs();
  void on_action_npcs();
  void on_action_tilesets();
#ifdef G_OS_WIN32
  void on_action_play();
#endif
  void on_action_screenshot();

  void on_switch_level(level_display& disp);

  link_list m_link_list;
  sign_list m_sign_list;
  npc_list m_npc_list;
  tileset_list m_tileset_list;

  window& m_window;
  preferences& m_preferences;
};

}

}
