#include "_precompiled.hpp"
#include "tile_objects_display.hpp"
#include "level.hpp"

using namespace Graal;

level_editor::tile_objects_display::tile_objects_display(level_editor::preferences& preferences)
    : m_preferences(preferences), m_group_label("Group:"),
      m_object_label("Object:") {
  Gtk::Table& table = *Gtk::manage(new Gtk::Table(5, 4));
  pack_start(table, Gtk::PACK_SHRINK);

  table.attach(m_group_label, 0, 1, 0, 1, Gtk::FILL|Gtk::SHRINK, Gtk::FILL|Gtk::SHRINK, 0, 3);
  table.attach(m_groups, 0, 2, 1, 2, Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL);
  table.attach(m_object_label, 0, 1, 2, 3, Gtk::FILL|Gtk::SHRINK, Gtk::FILL|Gtk::SHRINK, 0, 3);
  table.attach(m_objects, 0, 2, 3, 4, Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL);

  Gtk::Entry& groups_entry = *static_cast<Gtk::Entry*>(m_groups.get_child());
  groups_entry.set_editable(false);

  Gtk::Entry& objects_entry = *static_cast<Gtk::Entry*>(m_objects.get_child());
  objects_entry.set_editable(false);

  Gtk::ScrolledWindow& scrolled = *Gtk::manage(new Gtk::ScrolledWindow());
  scrolled.add(m_display);
  pack_start(scrolled);

  Gtk::Image* image;
  int width, height;
  Gtk::IconSize::lookup(Gtk::ICON_SIZE_BUTTON, width, height);

  Gtk::AttachOptions btn_attach_option = Gtk::FILL;
  // Group new
  image = Gtk::manage(new Gtk::Image(Gtk::Stock::NEW, Gtk::ICON_SIZE_MENU));
  m_group_new.set_size_request(width + 4, height + 4);
  m_group_new.add(*image);
  table.attach(m_group_new, 2, 3, 1, 2, btn_attach_option);

  // Group delete
  /*image = Gtk::manage(new Gtk::Image(Gtk::Stock::DELETE, Gtk::ICON_SIZE_MENU));
  m_group_delete.set_size_request(width + 4, height + 4);
  m_group_delete.add(*image);
  table.attach(m_group_delete, 3, 4, 1, 2, btn_attach_option);*/

  // Group save
  image = Gtk::manage(new Gtk::Image(Gtk::Stock::SAVE, Gtk::ICON_SIZE_MENU));
  m_group_save.set_size_request(width + 4, height + 4);
  m_group_save.add(*image);
  table.attach(m_group_save, 4, 5, 1, 2, btn_attach_option);

  // Object new
  image = Gtk::manage(new Gtk::Image(Gtk::Stock::NEW, Gtk::ICON_SIZE_MENU));
  m_object_new.set_size_request(width + 4, height + 4);
  m_object_new.add(*image);
  table.attach(m_object_new, 2, 3, 3, 4, btn_attach_option);

  // Object delete
  image = Gtk::manage(new Gtk::Image(Gtk::Stock::DELETE, Gtk::ICON_SIZE_MENU));
  m_object_delete.set_size_request(width + 4, height + 4);
  m_object_delete.add(*image);
  table.attach(m_object_delete, 3, 4, 3, 4, btn_attach_option);

  m_groups.signal_changed().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_group_changed), true);
  m_objects.signal_changed().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_object_changed), true);

  m_group_new.signal_clicked().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_group_new_clicked));
  /*m_group_delete.signal_clicked().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_group_delete_clicked));*/
  m_group_save.signal_clicked().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_group_save_clicked));

  m_object_new.signal_clicked().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_object_new_clicked));
  m_object_delete.signal_clicked().connect_notify(
      sigc::mem_fun(this, &tile_objects_display::on_object_delete_clicked));

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

  m_groups.clear_items();
  m_objects.clear_items();
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
  level_editor::preferences::tile_object_group_type& object_group = 
      m_preferences.tile_object_groups[m_groups.get_active_text()];
  level_editor::preferences::tile_object_group_type::iterator it, end = object_group.end();
  it = object_group.begin();

  m_objects.clear_items();
  for (; it != end; ++it) {
    m_objects.append_text(it->first);
  }

  it = object_group.begin();
  if (it != end) {
    // Set currently selected object to the first in list
    m_objects.set_active_text(it->first);
  } else {
    // Else unset the currently active element
    // TODO: m_objects.set_active(-1); does not work
    m_objects.insert_text(0, "");
    m_objects.set_active_text("");
    m_objects.remove_text("");
  }
}

void level_editor::tile_objects_display::on_object_changed() {
  // Abort if group doesn't exist
  if (m_preferences.tile_object_groups.find(m_groups.get_active_text()) ==
      m_preferences.tile_object_groups.end()) {
    m_display.clear();
    return;
  }

  level_editor::preferences::tile_object_group_type& object_group = 
      m_preferences.tile_object_groups[m_groups.get_active_text()];

  // Abort if object doesn't exist
  if (object_group.find(m_objects.get_active_text()) ==
      object_group.end()) {
    m_display.clear();
    return;
  }

  tile_buf tiles = object_group[m_objects.get_active_text()];
  m_display.set_tile_buf(tiles);
}

void level_editor::tile_objects_display::on_mouse_pressed(GdkEventButton* event) {
  // copy the buffer since it will be destroyed by the signal handler
  // TODO: might cause problems with multiple connected handlers?
  tile_buf tiles = m_display.get_tile_buf();

  if (!tiles.empty()) {
    m_signal_tiles_selected(tiles, tiles.get_width() / 2, tiles.get_height() / 2);
  }
}

level_editor::tile_objects_display::signal_tiles_selected_type&
level_editor::tile_objects_display::signal_tiles_selected() {
  return m_signal_tiles_selected;
}

level_editor::tile_objects_display::signal_create_tile_object_type&
level_editor::tile_objects_display::signal_create_tile_object() {
  return m_signal_create_tile_object;
}

void level_editor::tile_objects_display::on_group_new_clicked() {
  Gtk::Dialog dialog("New tile objects group", true);
  Gtk::Label label("Name the new group:");
  Gtk::Entry name;
  dialog.get_vbox()->pack_start(label);
  dialog.get_vbox()->pack_start(name);
  dialog.show_all_children();
  dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    m_preferences.tile_object_groups[name.get_text()];
    get();
    m_groups.set_active_text(name.get_text());
  }
}

/*void level_editor::tile_objects_display::on_group_delete_clicked() {}*/

void level_editor::tile_objects_display::on_group_save_clicked() {
  m_preferences.save_tile_objects(m_groups.get_active_text());
}

void level_editor::tile_objects_display::on_object_new_clicked() {
  tile_buf buf = m_signal_create_tile_object();

  if (!buf.empty()) {
    Gtk::Dialog dialog("New tile object", true);
    Gtk::Label label("Name the new object:");
    Gtk::Entry name;
    dialog.get_vbox()->pack_start(label);
    dialog.get_vbox()->pack_start(name);
    dialog.show_all_children();
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    if (dialog.run() == Gtk::RESPONSE_OK) {
      m_preferences.tile_object_groups[m_groups.get_active_text()][name.get_text()] = buf;
      get();
      m_objects.set_active_text(name.get_text());
    }
  }
}

void level_editor::tile_objects_display::on_object_delete_clicked() {
  level_editor::preferences::tile_object_group_type& object_group = 
      m_preferences.tile_object_groups[m_groups.get_active_text()];

  object_group.erase(m_objects.get_active_text());
  // Store currently active group name and restore it after reloading
  Glib::ustring group = m_groups.get_active_text();
  get();
  m_groups.set_active_text(group);
}
