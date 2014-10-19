#include "window/level_commands.hpp"
#include "level_display.hpp"
#include "window.hpp"

using namespace Graal::level_editor;

level_commands::level_commands(window& _window, header& _header, preferences& _preferences):
  m_link_list(_window),
  m_sign_list(_window),
  m_npc_list(_window),
  m_tileset_list(_window, _preferences),
  m_window(_window),
  m_preferences(_preferences)
{
  _header.action_level_create_link->signal_activate().connect(
    sigc::mem_fun(*this, &level_commands::on_action_create_link));
  _header.action_level_links->signal_activate().connect(
    sigc::mem_fun(*this, &level_commands::on_action_links));
  _header.action_level_signs->signal_activate().connect(
    sigc::mem_fun(*this, &level_commands::on_action_signs));
  _header.action_level_npcs->signal_activate().connect(
    sigc::mem_fun(*this, &level_commands::on_action_npcs));
  _header.action_level_tilesets->signal_activate().connect(
    sigc::mem_fun(*this, &level_commands::on_action_tilesets));
#ifdef G_OS_WIN32
  _header.action_level_play->signal_activate().connect(
    sigc::mem_fun(*this, &level_commands::on_action_play));
#endif
  _header.action_level_screenshot->signal_activate().connect(
    sigc::mem_fun(*this, &level_commands::on_action_screenshot));
}

void level_commands::on_action_links() {
  m_link_list.present();
}

void level_commands::on_action_npcs() {
  m_npc_list.present();
}

void level_commands::on_action_tilesets() {
  m_tileset_list.present();
}

void level_commands::on_action_signs() {
  m_sign_list.present();
}

void level_commands::on_action_screenshot() {
  Gtk::FileChooserDialog dialog(m_window, "Save Level Screenshot", Gtk::FILE_CHOOSER_ACTION_SAVE);
  Gtk::FileFilter filter;
  filter.add_pattern("*.png");
  filter.set_name("PNG Image (*.png)");
  dialog.add_filter(filter);
  dialog.set_do_overwrite_confirmation(true);

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  if (dialog.run() == Gtk::RESPONSE_OK) {
    Cairo::RefPtr<Cairo::ImageSurface> surface = m_window.get_current_level_display()->render_level();
    // TODO: do utf8 magic here, too lazy
    surface->write_to_png(dialog.get_filename());
  }
}

#ifdef G_OS_WIN32
void level_commands::on_action_play() {
  if (m_window.save_current_page()) {
    std::string path = m_window.get_current_level_display()->get_current_level_path().string();
    g_assert(!path.empty());
    std::string cmd = m_preferences.graal_dir + "/graaleditor.exe";
    char play_arg[] = "-play";
    gchar* argv[] = { &cmd[0], &path[0], play_arg, 0 };
    GError* error = 0;
    gdk_spawn_on_screen(
        m_window.get_screen()->gobj(),
        m_preferences.graal_dir.c_str(),
        argv,
        0, // envp
        (GSpawnFlags) (G_SPAWN_STDOUT_TO_DEV_NULL
                       | G_SPAWN_STDERR_TO_DEV_NULL),
        0, // setup func
        0, // user data
        0, // pid ptr
        &error);
    if (error) {
      Glib::ustring message(error->message);
      g_error_free(error);
      m_window.display_error(message);
    }
  }
}
#endif // G_OS_WIN32

// create a new link
void level_commands::on_action_create_link() {
  level_display* current = m_window.get_current_level_display();
  if (current->has_selection()) {
    link new_link = current->create_link();

    edit_link link_window(m_window);
    link_window.get(new_link);
    if (link_window.run() == Gtk::RESPONSE_OK) {
      new_link = link_window.get_link();
      m_window.get_current_level()->links.push_back(new_link);

      // update link list & level
      m_link_list.get();
      // TODO: also probably shouldn't be here
      current->invalidate();
    }
  }
}

void level_commands::on_switch_level(level_display&) {
  m_link_list.get();
  m_npc_list.get();
  m_sign_list.get();
}

void level_commands::hide_lists() {
  m_link_list.hide();
  m_npc_list.hide();
  m_sign_list.hide();
}
