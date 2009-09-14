#include "preferences_display.hpp"
#include <boost/filesystem.hpp>

using namespace Graal::level_editor;

preferences_display::preferences_display(preferences& prefs)
    : Gtk::Dialog("Preferences - Gonstruct"),
      m_prefs(prefs),
      m_pref_graaldir("Select Graal Directory - Gonstruct",
                      Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
      m_pref_selection_border_while_dragging(
          "Show selection border while dragging"),
      m_pref_remember_default_tile("Remember default tile"),
      m_pref_sticky_tile_selection("Sticky tile selection in tileset") {
  update_controls();

  set_border_width(16);

  Gtk::Table& entries = *Gtk::manage(new Gtk::Table(1, 4));
  entries.set_col_spacing(0, 8);

  entries.attach(
    *Gtk::manage(new Gtk::Label("Path to Graal directory:")),
    0, 1, 0, 1,
    Gtk::SHRINK | Gtk::FILL,
    Gtk::SHRINK | Gtk::FILL);
  entries.attach(m_pref_graaldir, 1, 2, 0, 1,
    Gtk::EXPAND | Gtk::FILL,
    Gtk::SHRINK | Gtk::FILL);

  entries.attach(m_pref_selection_border_while_dragging, 0, 2, 1, 2,
    Gtk::EXPAND | Gtk::FILL,
    Gtk::SHRINK | Gtk::FILL);

  entries.attach(m_pref_remember_default_tile, 0, 2, 2, 3,
    Gtk::EXPAND | Gtk::FILL,
    Gtk::SHRINK | Gtk::FILL);

  entries.attach(m_pref_sticky_tile_selection, 0, 2, 3, 4,
    Gtk::EXPAND | Gtk::FILL,
    Gtk::SHRINK | Gtk::FILL);

  get_vbox()->pack_start(entries);

  add_button(Gtk::Stock::APPLY,  Gtk::RESPONSE_APPLY);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(Gtk::Stock::OK,     Gtk::RESPONSE_OK);

  get_vbox()->show_all();
}

void preferences_display::update_controls() {
  if (boost::filesystem::exists(m_prefs.graal_dir))
    m_pref_graaldir.set_filename(Glib::filename_to_utf8(m_prefs.graal_dir));
  m_pref_selection_border_while_dragging.set_active(
      m_prefs.selection_border_while_dragging);
  m_pref_remember_default_tile.set_active(
      m_prefs.default_tile != -1);
  m_pref_sticky_tile_selection.set_active(
      m_prefs.sticky_tile_selection);
}

preferences_display::signal_preferences_changed_type&
preferences_display::signal_preferences_changed() {
  return m_signal_preferences_changed;
}

void preferences_display::on_response(int response_id) {
  switch (response_id) {
  case Gtk::RESPONSE_OK:
    apply();
    hide();
    break;
  case Gtk::RESPONSE_APPLY:
    apply();
    break;
  case Gtk::RESPONSE_CANCEL:
    hide();
    break;
  case Gtk::RESPONSE_DELETE_EVENT:
    hide();
    break;
  default:
    throw std::logic_error("unexpected response id in preferences_dialog");
  }
}

void preferences_display::apply() {
  preference_changes changes = NOTHING_CHANGED;
  // do this C-style to avoid a bunch of pointless conversions
  const Glib::ustring& new_graal_dir(m_pref_graaldir.get_filename());
  gsize bytes_read, bytes_written;
  char* new_graal_dir_utf8 = g_filename_to_utf8(new_graal_dir.c_str(),
                                                new_graal_dir.bytes(),
                                                &bytes_read, &bytes_written,
                                                0);
  if (!new_graal_dir_utf8)
    throw std::runtime_error("Error converting the Graal directory "
                             "path string to UTF-8.");
  try {
    if (bytes_read != new_graal_dir.bytes()) {
      throw std::runtime_error("Error converting all of the Graal directory "
                               "path string to UTF-8.");
    }
    
    if (m_prefs.graal_dir.compare(new_graal_dir_utf8) != 0) {
      m_prefs.graal_dir.assign(new_graal_dir_utf8);
      changes = changes | GRAAL_DIR_CHANGED;

      // reload tile objects
      m_prefs.load_tile_objects();
    }
  } catch (...) {
    g_free(new_graal_dir_utf8);
    throw;
  }
  g_free(new_graal_dir_utf8);

  bool new_selection_border_while_dragging =
    m_pref_selection_border_while_dragging.get_active();
  if (m_prefs.selection_border_while_dragging
      != new_selection_border_while_dragging) {
    m_prefs.selection_border_while_dragging =
      new_selection_border_while_dragging;
    changes |= SHOW_SELECTION_WHILE_DRAGGING_CHANGED;
  }

  bool new_remember_default_tile =
    m_pref_remember_default_tile.get_active();
  if ((m_prefs.default_tile != -1) != new_remember_default_tile) {
    if (new_remember_default_tile)
      m_prefs.default_tile = 0; // does not matter, gets overwritten if not -1
    else
      m_prefs.default_tile = -1;
    changes |= REMEMBER_DEFAULT_TILE_CHANGED;
  }

  bool new_sticky_tile_selection =
    m_pref_sticky_tile_selection.get_active();
  if (m_prefs.sticky_tile_selection
      != new_sticky_tile_selection) {
    m_prefs.sticky_tile_selection =
      new_sticky_tile_selection;
    changes |= STICKY_TILE_SELECTION;
  }

  m_signal_preferences_changed.emit(changes);
}
