#include "_precompiled.hpp"
#include "tile_objects_display.hpp"

using namespace Graal;

level_editor::tile_objects_display::tile_objects_display(level_editor::preferences& preferences)
    : m_preferences(preferences) {
  pack_start(m_groups, Gtk::PACK_SHRINK);
  pack_start(m_objects, Gtk::PACK_SHRINK);

  Gtk::Entry& groups_entry = *static_cast<Gtk::Entry*>(m_groups.get_child());
  groups_entry.set_editable(false);

  Gtk::Entry& objects_entry = *static_cast<Gtk::Entry*>(m_objects.get_child());
  objects_entry.set_editable(false);

  Gtk::ScrolledWindow& scrolled = *Gtk::manage(new Gtk::ScrolledWindow());
  scrolled.add(m_display);
  pack_start(scrolled);

  m_groups.signal_changed().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_group_changed), true);
  m_objects.signal_changed().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_object_changed), true);

  m_display.add_events(Gdk::BUTTON_PRESS_MASK);
  m_display.signal_button_press_event().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_mouse_pressed), true);
  show_all_children();
}

void level_editor::tile_objects_display::set() {
}

void level_editor::tile_objects_display::get() {
  level_editor::preferences::tile_objects_type::iterator it, end =
      m_preferences.tile_object_groups.end();
  it = m_preferences.tile_object_groups.begin();

  m_groups.clear();
  m_objects.clear();
  for (; it != end; ++it) {
    m_groups.append_text(it->first);
  }

  // Set active text to first item
  it = m_preferences.tile_object_groups.begin();
  if (it != end) {
    m_groups.set_active_text(it->first);
  }
}

void level_editor::tile_objects_display::set_tile_size(int tile_width, int tile_height) {
  m_display.set_tile_size(tile_width, tile_height);
  m_display.update_all();
}

void level_editor::tile_objects_display::set_tileset_surface(const Cairo::RefPtr<Cairo::Surface>& surface) {
  m_display.set_tileset_surface(surface);
}

void level_editor::tile_objects_display::on_group_changed() {
  // Abort if group doesn't exist
  if (m_preferences.tile_object_groups.find(m_groups.get_active_text()) ==
      m_preferences.tile_object_groups.end()) {
    return;
  }
  level_editor::preferences::tile_object_group_type object_group = 
      m_preferences.tile_object_groups[m_groups.get_active_text()];
  level_editor::preferences::tile_object_group_type::iterator it, end = object_group.end();
  it = object_group.begin();

  m_objects.clear();
  for (; it != end; ++it) {
    m_objects.append_text(it->first);
  }

  it = object_group.begin();
  if (it != end) {
    // Set currently selected object to the first in list
    m_objects.set_active_text(it->first);
  }
}

void level_editor::tile_objects_display::on_object_changed() {
  // Abort if group doesn't exist
  if (m_preferences.tile_object_groups.find(m_groups.get_active_text()) ==
      m_preferences.tile_object_groups.end()) {
    return;
  }

  level_editor::preferences::tile_object_group_type object_group = 
      m_preferences.tile_object_groups[m_groups.get_active_text()];

  // Abort if object doesn't exist
  if (object_group.find(m_objects.get_active_text()) ==
      object_group.end()) {
    return;
  }

  tile_buf& tiles = object_group[m_objects.get_active_text()];
  m_display.get_tile_buf() = tiles;
  m_display.set_surface_buffers();
  m_display.update_all();
  m_display.queue_draw();
}

void level_editor::tile_objects_display::on_mouse_pressed(GdkEventButton* event) {
  // copy the buffer since it will be destroyed by the signal handler
  // TODO: might cause problems with multiple connected handlers?
  tile_buf tiles = m_display.get_tile_buf();

  if (tiles.get_width() > 0 && tiles.get_height() > 0) {
    m_signal_tiles_selected(tiles, tiles.get_width() / 2, tiles.get_height() / 2);
  }
}

level_editor::tile_objects_display::signal_tiles_selected_type&
level_editor::tile_objects_display::signal_tiles_selected() {
  return m_signal_tiles_selected;
}
