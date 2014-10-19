#include "tileset_list.hpp"
#include "window.hpp"
#include "helper.hpp"
#include <boost/lexical_cast.hpp>

using namespace Graal;

// edit tileset window
level_editor::edit_tileset::edit_tileset()
: Gtk::Dialog("Edit Tileset", true, false), m_table(2, 5) {
  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  set_border_width(16);

  m_label_image.set_text("Image:");
  m_table.attach(m_label_image, 0, 1, 0, 1);
  m_label_prefix.set_text("Level Prefix:");
  m_table.attach(m_label_prefix, 0, 1, 1, 2);
  m_label_main.set_text("Main Tileset:");
  m_table.attach(m_label_main, 0, 1, 2, 3);
  m_label_x.set_text("X:");
  m_table.attach(m_label_x, 0, 1, 3, 4);
  m_label_y.set_text("Y:");
  m_table.attach(m_label_y, 0, 1, 4, 5);

  m_table.attach(m_edit_image, 1, 2, 0, 1);
  m_table.attach(m_edit_prefix, 1, 2, 1, 2);
  m_table.attach(m_check_main, 1, 2, 2, 3);
  m_table.attach(m_edit_x, 1, 2, 3, 4);
  m_table.attach(m_edit_y, 1, 2, 4, 5);

  m_check_main.signal_toggled().connect_notify(
    sigc::mem_fun(this, &edit_tileset::on_main_toggled)
  );
  m_table.set_spacings(5);
  get_vbox()->pack_start(m_table);

  show_all_children();
}

level_editor::edit_tileset::~edit_tileset() {}

void level_editor::edit_tileset::on_main_toggled() {
  bool enable = !m_check_main.get_active();
  m_edit_x.set_sensitive(enable);
  m_edit_y.set_sensitive(enable);
}

void level_editor::edit_tileset::get(const tileset& _tileset) {
  m_edit_image.set_text(_tileset.name);
  m_edit_prefix.set_text(_tileset.prefix);
  m_edit_x.set_text(boost::lexical_cast<std::string>(_tileset.x));
  m_edit_y.set_text(boost::lexical_cast<std::string>(_tileset.y));
  m_check_main.set_active(_tileset.main);
}

void level_editor::edit_tileset::set(tileset& _tileset) {
  bool is_main = m_check_main.get_active();

  _tileset.name = m_edit_image.get_text();
  _tileset.prefix = m_edit_prefix.get_text();
  _tileset.main = is_main;
  if (is_main) {
    _tileset.x = 0;
    _tileset.y = 0;
  } else {
    helper::parse<int>(m_edit_x.get_text(), _tileset.x);
    helper::parse<int>(m_edit_y.get_text(), _tileset.y);
  }
}

// tileset list window
level_editor::tileset_list::tileset_list(window& _window, preferences& _preferences)
: Gtk::Dialog("Tileset list", _window, false, false), m_window(_window), m_preferences(_preferences) {
  m_list_store = Gtk::ListStore::create(columns);
  m_tree_view.set_model(m_list_store);

  // Add editable "active" column
  m_tree_view.append_column("Active", columns.active);
  Gtk::CellRendererToggle* renderer = dynamic_cast<Gtk::CellRendererToggle*>(m_tree_view.get_column_cell_renderer(0));
  // For some reason there's no set_activatable function for CellRendererToggle on gtkmm 2.16 windows
  Glib::PropertyProxy<bool> activatable = renderer->property_activatable();
  activatable.set_value(true);
  renderer->signal_toggled().connect(sigc::mem_fun(
    this, &tileset_list::on_active_toggled));

  m_tree_view.append_column("Image", columns.image);
  m_tree_view.append_column("Prefix", columns.prefix);
  m_tree_view.append_column("X", columns.x);
  m_tree_view.append_column("Y", columns.y);
  m_tree_view.append_column("Main", columns.main);

  Gtk::ScrolledWindow* scrolled = Gtk::manage(new Gtk::ScrolledWindow());
  scrolled->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrolled->set_shadow_type(Gtk::SHADOW_IN);
  scrolled->add(m_tree_view);

  get_vbox()->pack_start(*scrolled);

  Gtk::Button* button_new = Gtk::manage(new Gtk::Button("New"));
  button_new->signal_clicked().connect(sigc::mem_fun(this, &tileset_list::on_new_clicked));
  get_action_area()->pack_start(*button_new);
  Gtk::Button* button_edit = Gtk::manage(new Gtk::Button("Edit"));
  button_edit->signal_clicked().connect(sigc::mem_fun(this, &tileset_list::on_edit_clicked));
  get_action_area()->pack_start(*button_edit);
  Gtk::Button* button_delete = Gtk::manage(new Gtk::Button("Delete"));
  button_delete->signal_clicked().connect(sigc::mem_fun(this, &tileset_list::on_delete_clicked));
  get_action_area()->pack_start(*button_delete);
  
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  
  set_default_size(400, 300);

  show_all_children();
}

void level_editor::tileset_list::on_show() {
  get();
  Gtk::Dialog::on_show();
}

void level_editor::tileset_list::on_response(int response_id) {
  if (response_id == Gtk::RESPONSE_CLOSE) {
    hide();
  }
}

void level_editor::tileset_list::on_new_clicked() {
  tileset new_tileset;
  edit_tileset dialog;
  dialog.get(new_tileset);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    // save tileset
    dialog.set(new_tileset);
    m_preferences.tilesets.push_back(new_tileset);
    m_window.update_matching_level_displays(new_tileset.prefix);
  }

  get();
}

void level_editor::tileset_list::on_edit_clicked() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    Gtk::TreeRow row = *iter;
    
    // get selected tileset
    tileset& _tileset = *row.get_value(columns.iter);
    edit_tileset dialog;
    dialog.get(_tileset);
    if (dialog.run() == Gtk::RESPONSE_OK) {
      std::string old_prefix = _tileset.prefix;
      std::string old_name = _tileset.name;
      // save tileset
      dialog.set(_tileset);
      // update levels with the previous prefix and new levels
      if (old_prefix != _tileset.prefix)
        m_window.update_matching_level_displays(old_prefix);
      m_window.update_matching_level_displays(_tileset.prefix);
    }

    get();
  }
}

void level_editor::tileset_list::on_delete_clicked() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    Gtk::TreeRow row = *iter;
    tileset_list_type::iterator tileset_iter = row.get_value(columns.iter);
    // Cache prefix to update tileset after
    std::string prefix = tileset_iter->prefix;
    m_preferences.tilesets.erase(tileset_iter);

    m_window.update_matching_level_displays(prefix);
    get();
  }
}

void level_editor::tileset_list::on_active_toggled(const Glib::ustring& path) {
  Gtk::TreeModel::iterator iter = m_list_store->get_iter(path);
  
  if (iter) {
    // Toggle the active state of the currently selected tileset
    Gtk::TreeRow row = *iter;
    tileset_list_type::iterator tileset_iter = row.get_value(columns.iter);

    bool active = !tileset_iter->active;
    tileset_iter->active = active;
    row.set_value(columns.active, active);
    m_window.update_matching_level_displays(tileset_iter->prefix);
  }
}

// get tileset data from the level
void level_editor::tileset_list::get() {
  m_list_store->clear();

  level_editor::tileset_list_type::iterator iter, end;
  end = m_preferences.tilesets.end();

  for (iter = m_preferences.tilesets.begin();
       iter != end;
       iter ++) {
    Gtk::TreeModel::iterator row = m_list_store->append();
    (*row)[columns.active] = iter->active;
    (*row)[columns.iter] = iter;
    (*row)[columns.prefix] = iter->prefix; // TODO: unicode
    (*row)[columns.image] = iter->name; // TODO: unicode
    (*row)[columns.main] = iter->main;
    (*row)[columns.x] = iter->x;
    (*row)[columns.y] = iter->y;
  }
}

level_editor::tileset_list::tileset_columns::~tileset_columns() {}
