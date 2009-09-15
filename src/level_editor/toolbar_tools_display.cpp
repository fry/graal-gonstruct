#include "toolbar_tools_display.hpp"
#include "window.hpp"

using namespace Graal;

level_editor::toolbar_tools_display::toolbar_tools_display(window& win, preferences& prefs)
: m_window(win), m_preferences(prefs),
  m_hide_npcs("Hide NPCs"), m_hide_signs("Hide signs"), m_hide_links("Hide links"),
  m_button_new_npc("New NPC") {
  m_button_new_npc.signal_clicked().connect(
      sigc::mem_fun(this, &toolbar_tools_display::on_new_npc_clicked));
  pack_start(m_button_new_npc, Gtk::PACK_SHRINK);

  m_hide_npcs.signal_toggled().connect_notify(
      sigc::mem_fun(this, &toolbar_tools_display::on_hide_npcs_toggled));
  pack_start(m_hide_npcs, Gtk::PACK_SHRINK);

  m_hide_signs.signal_toggled().connect_notify(
      sigc::mem_fun(this, &toolbar_tools_display::on_hide_signs_toggled));
  pack_start(m_hide_signs, Gtk::PACK_SHRINK);

  m_hide_links.signal_toggled().connect_notify(
      sigc::mem_fun(this, &toolbar_tools_display::on_hide_links_toggled));
  pack_start(m_hide_links, Gtk::PACK_SHRINK);

  // Layer selection
  m_spin_layer.signal_changed().connect_notify(
      sigc::mem_fun(this, &toolbar_tools_display::on_layer_changed));
  pack_start(m_spin_layer, Gtk::PACK_SHRINK);

  win.signal_switch_level_display().connect(
      sigc::mem_fun(this, &toolbar_tools_display::on_switch_level_display));
}

void level_editor::toolbar_tools_display::on_hide_npcs_toggled() {
  m_preferences.hide_npcs = m_hide_npcs.get_active();
  m_button_new_npc.set_sensitive(!m_preferences.hide_npcs);
  level_display& display = *m_window.get_current_level_display();
  if (display.npc_selected())
    display.clear_selection();
  display.queue_draw();
}

void level_editor::toolbar_tools_display::on_hide_signs_toggled() {
  m_preferences.hide_signs = m_hide_signs.get_active();
  m_window.get_current_level_display()->queue_draw();
}

void level_editor::toolbar_tools_display::on_hide_links_toggled() {
  m_preferences.hide_links = m_hide_links.get_active();
  m_window.get_current_level_display()->queue_draw();
}

void level_editor::toolbar_tools_display::on_layer_changed() {
  level_display* display = m_window.get_current_level_display();
  int layer = m_spin_layer.get_value_as_int();

  display->set_active_layer(layer);
}
void level_editor::toolbar_tools_display::on_switch_level_display(level_display& display) {
  m_spin_layer.set_range(0, display.get_level()->get_layer_count() - 1);
  m_spin_layer.set_value(0);
  m_spin_layer.set_increments(1, 1);
}

void level_editor::toolbar_tools_display::on_new_npc_clicked() {
  m_window.get_current_level_display()->drag_new_npc();
}
