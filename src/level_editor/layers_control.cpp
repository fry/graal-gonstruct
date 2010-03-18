#include "layers_control.hpp"
#include "window.hpp"

using namespace Graal;

level_editor::layers_control::layers_control(window& win, preferences& prefs)
: Gtk::HBox(false, 4), m_window(win), m_preferences(prefs),
  m_button_new_layer("+"), m_button_delete_layer("-") {
  // Layer selection
  m_spin_layer.signal_changed().connect_notify(
    sigc::mem_fun(this, &layers_control::on_layer_changed));

  Gtk::Label& layer_label = *Gtk::manage(new Gtk::Label("Layer:"));
  pack_start(layer_label, Gtk::PACK_SHRINK);
  pack_start(m_spin_layer, Gtk::PACK_SHRINK);

  //Gtk::Label& layer_visiblity_label = *Gtk::manage(new Gtk::Label("Visible:"));
  //pack_start(layer_visiblity_label, Gtk::PACK_SHRINK);

  m_layer_visible.signal_toggled().connect_notify(
    sigc::mem_fun(this, &layers_control::on_layer_visible_toggled));

  pack_start(m_layer_visible, Gtk::PACK_SHRINK);

  m_button_new_layer.signal_clicked().connect_notify(
    sigc::mem_fun(this, &layers_control::on_add_layer));
  pack_start(m_button_new_layer, Gtk::PACK_SHRINK);

  m_button_delete_layer.signal_clicked().connect_notify(
    sigc::mem_fun(this, &layers_control::on_delete_layer));
  pack_start(m_button_delete_layer, Gtk::PACK_SHRINK);


  win.signal_switch_level_display().connect(
    sigc::mem_fun(this, &layers_control::on_switch_level_display));
}

void level_editor::layers_control::on_layer_changed() {
  level_display* display = m_window.get_current_level_display();
  int layer = m_spin_layer.get_value_as_int();

  display->set_active_layer(layer);

  m_layer_visible.set_active(display->get_layer_visibility(layer));
}

void level_editor::layers_control::on_switch_level_display(level_display& display) {
  int layer_count = display.get_current_level()->get_layer_count();
  m_spin_layer.set_range(0, layer_count - 1);
  //m_spin_layer.set_value(0);
  m_spin_layer.set_increments(1, 1);

  int layer = display.get_active_layer();
  m_spin_layer.set_value(layer);
  m_layer_visible.set_active(display.get_layer_visibility(layer));

  m_button_delete_layer.set_sensitive(layer_count > 1);
}

void level_editor::layers_control::on_layer_visible_toggled() {
  int layer = m_spin_layer.get_value_as_int();
  m_window.get_current_level_display()->set_layer_visibility(layer, m_layer_visible.get_active());
}

void level_editor::layers_control::on_add_layer() {
  int layer = m_spin_layer.get_value_as_int();
  level_display& display = *m_window.get_current_level_display();
  display.get_current_level()->insert_layer(layer + 1);
  display.set_active_layer(layer + 1);
  on_switch_level_display(display);
}

void level_editor::layers_control::on_delete_layer() {
  int layer = m_spin_layer.get_value_as_int();
  level_display& display = *m_window.get_current_level_display();

  m_spin_layer.set_value(layer - 1);
  display.get_current_level()->delete_layer(layer);
  on_switch_level_display(display);
}
