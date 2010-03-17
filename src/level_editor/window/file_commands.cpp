#include "window/file_commands.hpp"
#include "level_display.hpp"
#include "window.hpp"

#include <boost/filesystem/path.hpp>

using namespace Graal::level_editor;

file_commands::file_commands(window& _window, header& _header, preferences& _preferences):
  m_window(_window), m_preferences(_preferences),
  m_fc_open(_window, "Open Level")
{
  _header.action_file_new->signal_activate().connect(
    sigc::mem_fun(*this, &file_commands::on_action_new));
  _header.action_file_open->signal_activate().connect(
    sigc::mem_fun(*this, &file_commands::on_action_open));
  _header.action_file_save->signal_activate().connect(
    sigc::mem_fun(*this, &file_commands::on_action_save));
  _header.action_file_save_as->signal_activate().connect(
    sigc::mem_fun(*this, &file_commands::on_action_save_as));
  _header.action_file_quit->signal_activate().connect(
    sigc::mem_fun(*this, &file_commands::on_action_quit));

  // FileChooserDialog open level
  m_fc_open.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  m_fc_open.add_button("Open", Gtk::RESPONSE_OK);
  m_fc_open.set_select_multiple(true);
  m_fc_open.set_current_folder(m_preferences.graal_dir);

  Gtk::FileFilter nw_filter;
  nw_filter.add_pattern("*.nw");
  nw_filter.add_pattern("*.gmap");
  nw_filter.set_name("Graal Levels (*.nw, *.gmap)");
  m_fc_open.add_filter(nw_filter);
  m_fc_open.set_filter(nw_filter);

  Gtk::FileFilter all_filter;
  all_filter.add_pattern("*");
  all_filter.set_name("All Files");
  m_fc_open.add_filter(all_filter);
}


void file_commands::on_action_new() {
  try {
    std::auto_ptr<level_display> display(m_window.create_level_display());

    display->new_level(m_window.default_tile.get_tile());
    m_window.create_new_page(*Gtk::manage((display.release())));
    m_window.set_level_buttons(true);
  } catch (std::exception& e) {
    m_window.display_error(e.what());
  }
}

void file_commands::on_action_open() {
  if (m_fc_open.run() == Gtk::RESPONSE_OK) {
    std::list<Glib::ustring> files(m_fc_open.get_filenames());
    std::list<Glib::ustring>::const_iterator iter, end = files.end();
    for (iter = files.begin();
         iter != end;) {
      boost::filesystem::path path(*iter);
      ++iter;

      // TODO: dirty hack for now
      m_window.opening_levels = iter != end;
      try {
        // Only activate the last page to load
        m_window.load_level(path, !m_window.opening_levels);
      } catch (const std::exception& e) {
        m_window.display_error(e.what());
      }
    }
  }
  
  m_fc_open.hide();
}

void file_commands::on_action_save() {
  m_window.save_current_page();
}

void file_commands::on_action_save_as() {
  m_window.save_current_page_as();
}

void file_commands::on_action_quit() {
  if (m_window.close_all_levels())
    return;
  m_window.hide();
}
