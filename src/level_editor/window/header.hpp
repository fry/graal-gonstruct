#pragma once

#include <gtkmm.h>

namespace Graal {

namespace level_editor {

class header: public Gtk::VBox {
public:
  header();

  Glib::RefPtr<Gtk::AccelGroup> get_accel_group();
  const Glib::RefPtr<Gtk::AccelGroup> get_accel_group() const;
  
  Gtk::MenuBar& get_menubar();
  Gtk::Toolbar& get_toolbar();
protected:
  Glib::RefPtr<Gtk::UIManager> m_ui_manager;

  Gtk::MenuBar* m_menubar;
  Gtk::Toolbar* m_toolbar;
public:
  const Glib::RefPtr<Gtk::ActionGroup> group_level_actions;
  
  const Glib::RefPtr<Gtk::Action> action_file;
  const Glib::RefPtr<Gtk::Action> action_file_new;
  const Glib::RefPtr<Gtk::Action> action_file_open;
  const Glib::RefPtr<Gtk::Action> action_file_save;
  const Glib::RefPtr<Gtk::Action> action_file_save_as;
  const Glib::RefPtr<Gtk::Action> action_file_quit;
  
  const Glib::RefPtr<Gtk::Action> action_edit;
  const Glib::RefPtr<Gtk::Action> action_edit_undo;
  const Glib::RefPtr<Gtk::Action> action_edit_redo;
  const Glib::RefPtr<Gtk::Action> action_edit_cut;
  const Glib::RefPtr<Gtk::Action> action_edit_copy;
  const Glib::RefPtr<Gtk::Action> action_edit_paste;
  const Glib::RefPtr<Gtk::Action> action_edit_delete;
  const Glib::RefPtr<Gtk::Action> action_edit_preferences;
  
  const Glib::RefPtr<Gtk::Action> action_level;
  const Glib::RefPtr<Gtk::Action> action_level_create_link;
  const Glib::RefPtr<Gtk::Action> action_level_links;
  const Glib::RefPtr<Gtk::Action> action_level_signs;
  const Glib::RefPtr<Gtk::Action> action_level_npcs;
  const Glib::RefPtr<Gtk::Action> action_level_tilesets;
  const Glib::RefPtr<Gtk::Action> action_level_play;
  const Glib::RefPtr<Gtk::Action> action_level_screenshot;
  
  const Glib::RefPtr<Gtk::Action> action_help;
  const Glib::RefPtr<Gtk::Action> action_help_about;
};
  
}

}
