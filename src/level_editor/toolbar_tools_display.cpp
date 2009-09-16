#include "toolbar_tools_display.hpp"
#include "window.hpp"

using namespace Graal;

level_editor::toolbar_tools_display::toolbar_tools_display(window& win, preferences& prefs)
: m_window(win), m_preferences(prefs),
  m_hide_npcs("Hide NPCs"), m_hide_signs("Hide signs"), m_hide_links("Hide links"),
  m_button_new_npc("New NPC"), m_button_new_layer("+"), m_button_delete_layer("-") {
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
  Gtk::HBox& layer_settings = *Gtk::manage(new Gtk::HBox(false, 5));
  m_spin_layer.signal_changed().connect_notify(
    sigc::mem_fun(this, &toolbar_tools_display::on_layer_changed));

  Gtk::Label& layer_label = *Gtk::manage(new Gtk::Label("Current Layer:"));
  layer_settings.pack_start(layer_label, Gtk::PACK_SHRINK);
  layer_settings.pack_start(m_spin_layer, Gtk::PACK_SHRINK);

  Gtk::Label& layer_visiblity_label = *Gtk::manage(new Gtk::Label("Visible:"));
  layer_settings.pack_start(layer_visiblity_label, Gtk::PACK_SHRINK);

  m_layer_visible.signal_toggled().connect_notify(
    sigc::mem_fun(this, &toolbar_tools_display::on_layer_visible_toggled));

  layer_settings.pack_start(m_layer_visible, Gtk::PACK_SHRINK);

  m_button_new_layer.signal_clicked().connect_notify(
    sigc::mem_fun(this, &toolbar_tools_display::on_add_layer));
  layer_settings.pack_start(m_button_new_layer, Gtk::PACK_SHRINK);

  m_button_delete_layer.signal_clicked().connect_notify(
    sigc::mem_fun(this, &toolbar_tools_display::on_delete_layer));
  layer_settings.pack_start(m_button_delete_layer, Gtk::PACK_SHRINK);

  pack_start(layer_settings, Gtk::PACK_SHRINK);

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

  m_layer_visible.set_active(display->get_layer_visibility(layer));
}

void level_editor::toolbar_tools_display::on_switch_level_display(level_display& display) {
  int layer_count = display.get_level()->get_layer_count();
  m_spin_layer.set_range(0, layer_count - 1);
  //m_spin_layer.set_value(0);
  m_spin_layer.set_increments(1, 1);

  int layer = m_spin_layer.get_value_as_int();
  m_layer_visible.set_active(display.get_layer_visibility(layer));

  m_button_delete_layer.set_sensitive(layer_count > 1);
}

void level_editor::toolbar_tools_display::on_new_npc_clicked() {
  m_window.get_current_level_display()->drag_new_npc();
}

void level_editor::toolbar_tools_display::on_layer_visible_toggled() {
  int layer = m_spin_layer.get_value_as_int();
  m_window.get_current_level_display()->set_layer_visibility(layer, m_layer_visible.get_active());
}

void level_editor::toolbar_tools_display::on_add_layer() {
  int layer = m_spin_layer.get_value_as_int();
  level_display& display = *m_window.get_current_level_display();
  display.get_level()->insert_layer(layer + 1);
  on_switch_level_display(display);
  display.set_layer_visibility(layer + 1, true);
  display.update_all();

  m_spin_layer.set_value(layer + 1);
}

void level_editor::toolbar_tools_display::on_delete_layer() {
  int layer = m_spin_layer.get_value_as_int();
  level_display& display = *m_window.get_current_level_display();

  m_spin_layer.set_value(layer - 1);
  display.get_level()->delete_layer(layer);
  on_switch_level_display(display);
  display.update_all();
}
