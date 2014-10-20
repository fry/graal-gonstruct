#include "header.hpp"

using namespace Graal;

namespace {
  const char ui_xml[] =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='FileNew'/>"
    "      <menuitem action='FileOpen'/>"
    "      <menuitem action='FileSave'/>"
    "      <menuitem action='FileSaveAs'/>"
    "      <separator/>"
    "      <menuitem action='FileQuit'/>"
    "    </menu>"
    "    <menu action='EditMenu'>"
    "      <menuitem action='EditUndo'/>"
    "      <menuitem action='EditRedo'/>"
    "      <separator/>"
    "      <menuitem action='EditCut'/>"
    "      <menuitem action='EditCopy'/>"
    "      <menuitem action='EditPaste'/>"
    "      <menuitem action='EditDelete'/>"
    "      <separator/>"
    "      <menuitem action='EditPreferences'/>"
    "    </menu>"
    "    <menu action='LevelMenu'>"
    "      <menuitem action='LevelCreateLink'/>"
    "      <menuitem action='LevelLinks'/>"
    "      <separator/>"
    "      <menuitem action='LevelSigns'/>"
    "      <separator/>"
    "      <menuitem action='LevelNPCs'/>"
    "      <menuitem action='LevelTilesets'/>"
#ifdef WIN32
    "      <menuitem action='LevelPlay'/>"
#endif
    "      <menuitem action='LevelScreenshot'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HelpAbout'/>"
    "    </menu>"
    "  </menubar>"
    "  <toolbar name='ToolBar'>"
    "    <toolitem action='FileNew'/>"
    "    <toolitem action='FileOpen'/>"
    "    <toolitem action='FileSave'/>"
    "    <toolitem action='FileSaveAs'/>"
    "    <separator/>"
    "    <toolitem action='LevelCreateLink'/>"
    "    <toolitem action='LevelLinks'/>"
    "    <separator/>"
    "    <toolitem action='LevelSigns'/>"
    "    <separator/>"
    "    <toolitem action='LevelNPCs'/>"
    "    <toolitem action='LevelTilesets'/>"
    "    <separator/>"
#ifdef WIN32
    "    <toolitem action='LevelPlay'/>"
#endif
    "    <toolitem action='EditPreferences'/>"
    "  </toolbar>"
    "</ui>";
}

level_editor::header::header():
  group_level_actions(Gtk::ActionGroup::create()),

  action_file(Gtk::Action::create("FileMenu", "_File")),
  action_file_new(Gtk::Action::create("FileNew", Gtk::Stock::NEW)),
  action_file_open(Gtk::Action::create("FileOpen", Gtk::Stock::OPEN)),
  action_file_save(Gtk::Action::create("FileSave", Gtk::Stock::SAVE)),
  action_file_save_as(Gtk::Action::create("FileSaveAs", Gtk::Stock::SAVE_AS)),
  action_file_quit(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT)),

  action_edit(Gtk::Action::create("EditMenu", "_Edit")),
  action_edit_undo(Gtk::Action::create("EditUndo", Gtk::Stock::UNDO)),
  action_edit_redo(Gtk::Action::create("EditRedo", Gtk::Stock::REDO)),
  action_edit_cut(Gtk::Action::create("EditCut", Gtk::Stock::CUT)),
  action_edit_copy(Gtk::Action::create("EditCopy", Gtk::Stock::COPY)),
  action_edit_paste(Gtk::Action::create("EditPaste", Gtk::Stock::PASTE)),
  action_edit_delete(Gtk::Action::create("EditDelete", Gtk::Stock::DELETE)),
  action_edit_preferences(Gtk::Action::create("EditPreferences",
    Gtk::Stock::PREFERENCES)),

  action_level(Gtk::Action::create("LevelMenu", "_Level")),
  action_level_create_link(
    Gtk::Action::create_with_icon_name(
      "LevelCreateLink", "gonstruct_icon_create_link",
      "Create link", "Create a link from the selected tiles.")),
  action_level_links(
    Gtk::Action::create_with_icon_name(
      "LevelLinks", "gonstruct_icon_links",
      "Links", "Show a list of level links.")),
  action_level_signs(
    Gtk::Action::create_with_icon_name(
      "LevelSigns", "gonstruct_icon_signs",
      "Signs", "Show a list of signs.")),
  action_level_npcs(
    Gtk::Action::create_with_icon_name(
      "LevelNPCs", "gonstruct_icon_npcs",
      "NPCs", "Show a list of NPCs.")),
  action_level_tilesets(
    Gtk::Action::create_with_icon_name(
      "LevelTilesets", "gonstruct_icon_tilesets",
      "Tilesets", "Show a list of tilesets.")),
#ifdef WIN32
  action_level_play(Gtk::Action::create("LevelPlay", Gtk::Stock::EXECUTE)),
#endif
  action_level_screenshot(
    Gtk::Action::create("LevelScreenshot",
                        Gtk::Stock::ZOOM_FIT, "Screenshot",
                        "Take a screenshot of the level.")),
  
  action_help(Gtk::Action::create("HelpMenu", "_Help")),
  action_help_about(Gtk::Action::create("HelpAbout", Gtk::Stock::ABOUT)) 

{
  group_level_actions->add(action_file_save);
  group_level_actions->add(action_file_save_as);

  group_level_actions->add(action_level);
  group_level_actions->add(action_level_create_link);
  group_level_actions->add(action_level_links);
  group_level_actions->add(action_level_signs);
  group_level_actions->add(action_level_npcs);
  group_level_actions->add(action_level_tilesets);
#ifdef WIN32
  group_level_actions->add(action_level_play);
#endif
  group_level_actions->add(action_level_screenshot);
  
  group_level_actions->add(action_edit_undo, Gtk::AccelKey("<control>z"));
  group_level_actions->add(action_edit_redo, Gtk::AccelKey("<control>y"));
  group_level_actions->add(action_edit_cut);
  group_level_actions->add(action_edit_copy);
  group_level_actions->add(action_edit_paste);
  group_level_actions->add(action_edit_delete, Gtk::AccelKey("Delete"));

  Glib::RefPtr<Gtk::ActionGroup> actions = Gtk::ActionGroup::create();
  actions->add(action_file);
  actions->add(action_file_new);
  actions->add(action_file_open);
  actions->add(action_file_quit);
  actions->add(action_help);
  actions->add(action_help_about);
  actions->add(action_edit);
  actions->add(action_edit_preferences);

  m_ui_manager = Gtk::UIManager::create();
  m_ui_manager->add_ui_from_string(ui_xml);
  m_ui_manager->insert_action_group(group_level_actions);
  m_ui_manager->insert_action_group(actions);
  
  m_menubar = static_cast<Gtk::MenuBar*>(m_ui_manager->get_widget("/MenuBar"));
  pack_start(*m_menubar, Gtk::PACK_SHRINK);
  
  m_toolbar = static_cast<Gtk::Toolbar*>(m_ui_manager->get_widget("/ToolBar"));
  m_toolbar->set_toolbar_style(Gtk::TOOLBAR_ICONS);
  pack_start(*m_toolbar, Gtk::PACK_SHRINK);
}

level_editor::header::~header() {}

Gtk::MenuBar& level_editor::header::get_menubar() {
  return *m_menubar;
}

Gtk::Toolbar& level_editor::header::get_toolbar() {
  return *m_toolbar;
}

Glib::RefPtr<Gtk::AccelGroup> level_editor::header::get_accel_group() {
  return m_ui_manager->get_accel_group();
}

const Glib::RefPtr<Gtk::AccelGroup> level_editor::header::get_accel_group() const {
  return m_ui_manager->get_accel_group();
}
