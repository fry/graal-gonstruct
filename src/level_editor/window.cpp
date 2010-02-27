#include "window.hpp"
#include "preferences_display.hpp"
#include "config.hpp"
#include "toolbar_tools_display.hpp"
#include "layers_control.hpp"

#include <iostream>
#include <memory>

// So we can use Gtk::Stock::DELETE. gtkglext seems to define it (?)
#undef DELETE

using namespace Graal;

namespace {
  bool confirm_changes(Graal::level_editor::window& parent,
                      level_editor::level_display& display) {
    parent.set_current_page(display);
    const boost::filesystem::path& path = display.get_level_path();

    Glib::ustring basename;
    if (path.empty())
      basename = "new";
    else
      basename = path.leaf();
    Gtk::MessageDialog security_dialog(
      parent, "The file '" + basename + "' is not saved.", false,
      Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, true);
    security_dialog.set_secondary_text(
      "Do you want to save it before closing?");
    security_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    security_dialog.add_button(Gtk::Stock::DISCARD, Gtk::RESPONSE_NO);
    security_dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_YES);

    switch (security_dialog.run()) {
      case Gtk::RESPONSE_CANCEL: 
      case Gtk::RESPONSE_DELETE_EVENT:
        return false;
      case Gtk::RESPONSE_NO:
        return true;
      case Gtk::RESPONSE_YES:
        break;
      default:
        throw std::runtime_error("unexpected dialog response");
        break;
    }

    return parent.save_current_page();
  }

  const Glib::ustring ui_xml(
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='New'/>"
    "      <menuitem action='Open'/>"
    "      <menuitem action='Save'/>"
    "      <menuitem action='SaveAs'/>"
    "      <separator/>"
    "      <menuitem action='Quit'/>"
    "    </menu>"
    "    <menu action='EditMenu'>"
    "      <menuitem action='Undo'/>"
    "      <menuitem action='Redo'/>"
    "      <separator/>"
    "      <menuitem action='Cut'/>"
    "      <menuitem action='Copy'/>"
    "      <menuitem action='Paste'/>"
    "      <menuitem action='Delete'/>"
    "      <separator/>"
    "      <menuitem action='Preferences'/>"
    "    </menu>"
    "    <menu action='LevelMenu'>"
    "      <menuitem action='CreateLink'/>"
    "      <menuitem action='Links'/>"
    "      <separator/>"
    "      <menuitem action='Signs'/>"
    "      <separator/>"
    "      <menuitem action='NPCs'/>"
    "      <menuitem action='Tilesets'/>"
#ifdef WIN32
    "      <menuitem action='Play'/>"
#endif
    "      <menuitem action='Screenshot'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='About'/>"
    "    </menu>"
    "  </menubar>"
    "  <toolbar name='ToolBar'>"
    "    <toolitem action='New'/>"
    "    <toolitem action='Open'/>"
    "    <toolitem action='Save'/>"
    "    <toolitem action='SaveAs'/>"
    "    <separator/>"
    "    <toolitem action='CreateLink'/>"
    "    <toolitem action='Links'/>"
    "    <separator/>"
    "    <toolitem action='Signs'/>"
    "    <separator/>"
    "    <toolitem action='NPCs'/>"
    "    <toolitem action='Tilesets'/>"
    "    <separator/>"
#ifdef WIN32
    "    <toolitem action='Play'/>"
#endif
    "    <toolitem action='Preferences'/>"
    "  </toolbar>"
    "</ui>"
      );
}

level_editor::window::tab_label::tab_label(const Glib::ustring& label)
: m_label(label) {
  Gtk::Image* image = Gtk::manage(new Gtk::Image(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU));
  int width, height;
  Gtk::IconSize::lookup(Gtk::ICON_SIZE_MENU, width, height);

  m_button.set_size_request(width + 4, height + 4);
  m_button.add(*image);
  m_button.set_relief(Gtk::RELIEF_NONE);

  pack_start(m_unsaved);
  pack_start(m_label);
  pack_start(m_button);

  show_all();
}

void level_editor::window::tab_label::set_label(const Glib::ustring& label) {
  m_label.set_text(label);
}

void level_editor::window::tab_label::set_unsaved_status(bool status) {
  m_unsaved.set_text(status ? "* " : "");
}

level_editor::window::tab_label::signal_proxy_close level_editor::window::tab_label::close_event() {
  return m_button.signal_clicked();
}

// window
level_editor::window::window(preferences& _prefs)
: fs(_prefs), m_image_cache(fs), display_tileset(_prefs, m_image_cache),
  m_preferences(_prefs), m_link_list(*this),
  m_sign_list(*this), m_npc_list(*this), m_tileset_list(*this, _prefs),
  m_prefs_display(_prefs), m_tile_objects(_prefs), m_opening_levels(false),
  m_fc_open(*this, "Open Level"), m_fc_save(*this, "Save level as", Gtk::FILE_CHOOSER_ACTION_SAVE) {

  set_title(std::string("Gonstruct ") + config::version_string);

  // UIManager setup
  Glib::RefPtr<Gtk::ActionGroup> actions = Gtk::ActionGroup::create();

  actions->add(Gtk::Action::create("FileMenu", "_File"));
  actions->add(Gtk::Action::create("EditMenu", "_Edit"));
  actions->add(Gtk::Action::create("HelpMenu", "_Help"));
  actions->add(Gtk::Action::create("New", Gtk::Stock::NEW),
      sigc::mem_fun(*this, &window::on_action_new));
  actions->add(Gtk::Action::create("Open", Gtk::Stock::OPEN),
      sigc::mem_fun(*this, &window::on_action_open));
  actions->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT),
      sigc::mem_fun(*this, &window::on_action_quit));
  actions->add(Gtk::Action::create("Preferences",
                                   Gtk::Stock::PREFERENCES),
      sigc::mem_fun(*this, &window::on_action_prefs));
  actions->add(Gtk::Action::create("About",
                                   Gtk::Stock::ABOUT),
      sigc::mem_fun(*this, &window::on_action_about));

  m_level_actions = Gtk::ActionGroup::create();

  m_level_actions->add(Gtk::Action::create("LevelMenu", "_Level"));
  m_level_actions->add(Gtk::Action::create("Save", Gtk::Stock::SAVE),
      sigc::mem_fun(*this, &window::on_action_save));
  m_level_actions->add(Gtk::Action::create("SaveAs", Gtk::Stock::SAVE_AS),
      sigc::mem_fun(*this, &window::on_action_save_as));
  m_level_actions->add(
      Gtk::Action::create("CreateLink",
                          Gtk::Stock::GO_FORWARD, "Create link",
                          "Create a link from the selected tiles."),
      sigc::mem_fun(*this, &window::on_action_create_link));
  m_level_actions->add(
      Gtk::Action::create("Links",
                          Gtk::Stock::FULLSCREEN, "Links",
                          "Show a list of level links."),
      sigc::mem_fun(*this, &window::on_action_links));
  m_level_actions->add(
      Gtk::Action::create("Signs",
                          Gtk::Stock::DND_MULTIPLE, "Signs",
                          "Show a list of signs."),
      sigc::mem_fun(*this, &window::on_action_signs));
  m_level_actions->add(
      Gtk::Action::create("NPCs",
                          Gtk::Stock::SELECT_COLOR, "NPCs",
                          "Show a list of NPCs."),
      sigc::mem_fun(*this, &window::on_action_npcs));
  actions->add(
      Gtk::Action::create("Tilesets",
                          Gtk::Stock::SELECT_COLOR, "Tilesets",
                          "Show a list of tilesets."),
      sigc::mem_fun(*this, &window::on_action_tilesets));
#ifdef WIN32
  m_level_actions->add(Gtk::Action::create("Play", Gtk::Stock::EXECUTE),
      sigc::mem_fun(*this, &window::on_action_play));
#endif
  m_level_actions->add(
      Gtk::Action::create("Screenshot",
                          Gtk::Stock::ZOOM_FIT, "Screenshot",
                          "Take a screenshot of the level."),
      sigc::mem_fun(*this, &window::on_action_screenshot));

  m_level_actions->add(
      Gtk::Action::create("Undo", Gtk::Stock::UNDO),
      Gtk::AccelKey("<control>z"),
      sigc::mem_fun(*this, &window::on_action_undo));
  m_level_actions->add(
      Gtk::Action::create("Redo", Gtk::Stock::REDO),
      Gtk::AccelKey("<control>y"),
      sigc::mem_fun(*this, &window::on_action_redo));
  m_level_actions->add(
      Gtk::Action::create("Cut", Gtk::Stock::CUT),
      sigc::mem_fun(*this, &window::on_action_cut));
  m_level_actions->add(
      Gtk::Action::create("Copy", Gtk::Stock::COPY),
      sigc::mem_fun(*this, &window::on_action_copy));
  m_level_actions->add(
      Gtk::Action::create("Paste", Gtk::Stock::PASTE),
      sigc::mem_fun(*this, &window::on_action_paste));
  m_level_actions->add(
      Gtk::Action::create("Delete", Gtk::Stock::DELETE),
      Gtk::AccelKey("Delete"),
      sigc::mem_fun(*this, &window::on_action_delete));

  m_ui = Gtk::UIManager::create();
  m_ui->add_ui_from_string(ui_xml);
  m_ui->insert_action_group(m_level_actions);
  m_ui->insert_action_group(actions);
  add_accel_group(m_ui->get_accel_group());

  Gtk::HPaned* hpane = Gtk::manage(new Gtk::HPaned());
  Gtk::VBox* vbox_main = Gtk::manage(new Gtk::VBox());

  Gtk::Widget* menu = m_ui->get_widget("/MenuBar");
  vbox_main->pack_start(*menu, Gtk::PACK_SHRINK);

  // toolbar + edit area + layers control
  Gtk::Toolbar* toolbar =
    static_cast<Gtk::Toolbar*>(m_ui->get_widget("/ToolBar"));
  toolbar->set_toolbar_style(Gtk::TOOLBAR_ICONS);
  // TODO: need to create layers_control here because it uses this in the constructor
  m_layers_control = Gtk::manage(new layers_control(*this, m_preferences));
  Gtk::HBox& hbox_toolbar = *Gtk::manage(new Gtk::HBox());
  hbox_toolbar.pack_start(*toolbar, Gtk::PACK_EXPAND_WIDGET);
  hbox_toolbar.pack_start(*m_layers_control, Gtk::PACK_SHRINK);
  vbox_main->pack_start(hbox_toolbar, Gtk::PACK_SHRINK);
  // hpane with level + side panel
  m_nb_levels.signal_switch_page().connect_notify(
      sigc::mem_fun(this, &window::on_switch_page), true); // after is important
  m_nb_levels.set_scrollable(true);
  
  hpane->pack1(m_nb_levels);
  hpane->pack2(m_nb_toolset, Gtk::SHRINK);
  hpane->set_position(550);

  vbox_main->pack_start(*hpane);

  // toolset
  // tileset
  Gtk::ScrolledWindow& tileset_scrolled = *Gtk::manage(new Gtk::ScrolledWindow());
  Gdk::Color color;
  color.set_rgb_p(1, 0.6, 0.9);
  display_tileset.modify_bg(Gtk::STATE_NORMAL, color);
  tileset_scrolled.add(display_tileset);
  tileset_scrolled.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  m_nb_toolset.append_page(tileset_scrolled, "Tileset");
  // tools TODO: need to construct it here because it requires the level_display switch signal
  m_tools = Gtk::manage(new toolbar_tools_display(*this, _prefs));
  m_nb_toolset.append_page(*m_tools, "Tools");
  // tile objects
  m_nb_toolset.append_page(m_tile_objects, "Tile Objects");
  //m_status_bar.pack_start(m_status, Gtk::PACK_SHRINK);
  m_status_bar.push("");
  m_status_bar.pack_start(default_tile, Gtk::PACK_SHRINK);
  vbox_main->pack_start(m_status_bar, Gtk::PACK_SHRINK);

  set_default_size(800, 600);

  add(*vbox_main);
  show_all_children();

  m_prefs_display.signal_preferences_changed().connect(
      sigc::mem_fun(this, &window::on_preferences_changed));

  // connect to tileset_display's signals
  display_tileset.signal_status_update().connect(
      sigc::mem_fun(this, &window::set_status));
  display_tileset.signal_default_tile_changed().connect(
      sigc::mem_fun(this, &window::set_default_tile));
  display_tileset.signal_tiles_selected().connect(
      sigc::mem_fun(this, &window::tiles_selected));
  display_tileset.signal_tileset_updated().connect(
      sigc::mem_fun(this, &window::on_tileset_update));
  m_tile_objects.signal_tiles_selected().connect(
      sigc::mem_fun(this, &window::tiles_selected));
  m_tile_objects.signal_create_tile_object().connect(
      sigc::mem_fun(this, &window::get_current_tile_selection));

  std::cout << "Caching...";
  fs.update_cache();
  std::cout << " done" << std::endl;

  default_tile.set_tile(m_preferences.default_tile);

  // FileChooserDialog open level
  m_fc_open.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  m_fc_open.add_button("Open", Gtk::RESPONSE_OK);
  m_fc_open.set_select_multiple(true);

  Gtk::FileFilter nw_filter;
  nw_filter.add_pattern("*.nw");
  nw_filter.set_name("Graal Level (*.nw)");
  m_fc_open.add_filter(nw_filter);
  m_fc_open.set_filter(nw_filter);

  Gtk::FileFilter all_filter;
  all_filter.add_pattern("*");
  all_filter.set_name("All Files");
  m_fc_open.add_filter(all_filter);

  // FileChooserDialog save as
  m_fc_save.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  m_fc_save.add_button("Save", Gtk::RESPONSE_OK);

  m_fc_save.add_filter(nw_filter);
  m_fc_save.set_filter(nw_filter);
  m_fc_save.set_do_overwrite_confirmation(true);

  set_level_buttons(false);

  // TODO: no
  m_tile_width = 16;
  m_tile_height = 16;

  default_tile.set_tile_size(m_tile_width, m_tile_height);
  display_tileset.set_tile_size(m_tile_width, m_tile_height);
  m_tile_objects.set_tile_size(m_tile_width, m_tile_height);

  m_tile_objects.get();
}

level_editor::window::~window() {
  if (m_preferences.default_tile != -1)
    m_preferences.default_tile = default_tile.get_tile();
}

void level_editor::window::on_tileset_update(const Cairo::RefPtr<Cairo::ImageSurface>& surface) {
  m_tile_objects.set_tileset_surface(surface);
  default_tile.set_tileset_surface(surface);
  if (m_nb_levels.get_n_pages() > 0) {
    get_current_level_display()->set_tileset_surface(surface);
  }
}

void level_editor::window::set_level_buttons(bool enabled) {
  m_level_actions->set_sensitive(enabled);
  m_tile_objects.set_sensitive(enabled);
  m_tools->set_sensitive(enabled);
  m_layers_control->set_sensitive(enabled);
}

void level_editor::window::set_default_tile(int tile_index) {
  default_tile.set_tile(tile_index);

  for (int i = 0; i < m_nb_levels.get_n_pages(); i ++) {
    level_display& disp(*get_nth_level_display(i));
    disp.set_default_tile(tile_index);
  }
  get_current_level_display()->clear_selection();
}

void level_editor::window::display_error(const Glib::ustring& message) {
  Gtk::MessageDialog dialog(message, false,
    Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
  dialog.set_message("Error");
  dialog.set_secondary_text(message);
  dialog.run();
}

// change tileset to the level's one when the page changes
void level_editor::window::on_switch_page(GtkNotebookPage* page, guint page_num) {
  level_display* display = get_nth_level_display(page_num);
  if (!m_opening_levels) {
    display_tileset.update_tileset(display->get_level_path().leaf());
  }

  m_signal_switch_level_display(*display);

  if (m_nb_levels.get_n_pages() > 0) {
    m_link_list.get();
    m_npc_list.get();
    m_sign_list.get();
  }
}

void level_editor::window::on_preferences_changed(
    level_editor::preferences_display::preference_changes c) {
  if (c & preferences_display::GRAAL_DIR_CHANGED) {
    m_image_cache.clear_cache();
    
    fs.update_cache();

    for (int i = 0; i < m_nb_levels.get_n_pages(); i ++) {
      level_display& disp(*get_nth_level_display(i));
      display_tileset.update_tileset(disp.get_level_path().leaf());
    }

    m_tile_objects.get();
  }
}

std::auto_ptr<level_editor::level_display>
level_editor::window::create_level_display() {
  std::auto_ptr<level_display> display(new level_display(
      m_preferences, m_image_cache,
      default_tile.get_tile()));
  display->set_tile_size(m_tile_width, m_tile_height);

  display->signal_default_tile_changed().connect(
      sigc::mem_fun(this, &window::set_default_tile));
  display->signal_status_update().connect(
      sigc::mem_fun(this, &window::set_status));

  return display;
}

void level_editor::window::tiles_selected(tile_buf& selection, int x, int y) {
  get_current_level_display()->drag_selection(selection, x, y);
}

boost::shared_ptr<level>& level_editor::window::get_current_level() {
  return get_current_level_display()->get_level();
}

bool level_editor::window::close_all_levels() {
  for (int i = 0; i < m_nb_levels.get_n_pages(); i ++) {
    level_display& disp(*get_nth_level_display(i));
    if (disp.get_unsaved()) {
      if (confirm_changes(*this, disp)) {
        m_nb_levels.remove_page(i);
        i = i - 1;
      } else {
        return true;
      }
    }
  }
  
  return false;
}

bool level_editor::window::on_delete_event(GdkEventAny* event) {
  return close_all_levels();
}

void level_editor::window::on_action_links() {
  m_link_list.present();
}

void level_editor::window::on_action_npcs() {
  m_npc_list.present();
}

void level_editor::window::on_action_tilesets() {
  m_tileset_list.present();
}

void level_editor::window::on_action_signs() {
  m_sign_list.present();
}

void level_editor::window::on_action_screenshot() {
  Gtk::FileChooserDialog dialog(*this, "Save Level Screenshot", Gtk::FILE_CHOOSER_ACTION_SAVE);
  Gtk::FileFilter filter;
  filter.add_pattern("*.png");
  filter.set_name("PNG Image (*.png)");
  dialog.add_filter(filter);
  dialog.set_do_overwrite_confirmation(true);

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  if (dialog.run() == Gtk::RESPONSE_OK) {
    Cairo::RefPtr<Cairo::Surface> surface = get_current_level_display()->render_level(
        false, false,
        !m_preferences.hide_npcs,
        !m_preferences.hide_links,
        !m_preferences.hide_signs);
    // TODO: do utf8 magic here, too lazy
    surface->write_to_png(dialog.get_filename());
  }
}

#ifdef WIN32
void level_editor::window::on_action_play() {
  if (save_current_page()) {
    std::string path = get_current_level_display()->get_level_path().string();
    g_assert(!path.empty());
    std::string cmd = m_preferences.graal_dir + "/graaleditor.exe";
    char play_arg[] = "-play";
    gchar* argv[] = { &cmd[0], &path[0], play_arg, 0 };
    GError* error = 0;
    gdk_spawn_on_screen(
        get_screen()->gobj(),
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
      display_error(message);
    }
  }
}
#endif // WIN32

void level_editor::window::on_action_prefs() {
  m_prefs_display.update_controls();
  m_prefs_display.present();
}

void level_editor::window::on_action_about() {
  Gtk::AboutDialog dialog;
  dialog.set_name("Gonstruct");
  dialog.set_version(config::version_string);
  dialog.set_comments("An alternative Graal Online level editor");

  dialog.set_copyright("Copyright \302\251 2008 "
                       "Eike Siewertsen, Benjamin Herr\nGraal Online is Copyright \302\251 by Cyberjoueurs"); // TODO

  std::deque<Glib::ustring> authors;
  authors.push_back("Developers:");
  authors.push_back("  Eike Siewertsen <e.siew@londeroth.org>");
  authors.push_back("  Benjamin Herr <ben@0x539.de>");

  std::deque<Glib::ustring> artists;
  artists.push_back("Benjamin Herr <ben@0x539.de>");

  dialog.set_authors(authors);
  dialog.set_artists(artists);

  //dialog.set_logo(m_logo);

  dialog.run();
}

void level_editor::window::on_action_undo() {
  level_display* current = get_current_level_display();
  g_assert(current != 0);
  current->undo();
  current->clear_selection();
  current->queue_draw();
}

void level_editor::window::on_action_redo() {
  level_display* current = get_current_level_display();
  g_assert(current != 0);
  current->redo();
  current->clear_selection();
  current->queue_draw();
}

void level_editor::window::on_action_cut() {
  level_display& display = *get_current_level_display();
  if (!display.npc_selected()) {
    tile_buf buf(display.selection);
    if (buf.empty()) {
      display.lift_selection();
      buf = display.selection;
    }
    copy_cache.reset(new tiles_cache(buf));
  } else {
    copy_cache.reset(new npc_cache(*display.selected_npc));
  }

  display.delete_selection();
}

void level_editor::window::on_action_copy() {
  level_display& display = *get_current_level_display();
  if (!display.npc_selected()) {
    tile_buf buf(display.selection);
    if (buf.empty()) {
      display.lift_selection();
      buf = display.selection;
      display.undo();
    }
    copy_cache.reset(new tiles_cache(buf));
  } else {
    copy_cache.reset(new npc_cache(*display.selected_npc));
  }
}

void level_editor::window::on_action_paste() {
  level_display& display = *get_current_level_display();
  copy_cache->paste(display);
}

void level_editor::window::on_action_delete() {
  level_display* current = get_current_level_display();
  g_assert(current != 0);
  current->delete_selection();
}

// create a new link
void level_editor::window::on_action_create_link() {
  level_display* current = get_current_level_display();
  if (current->has_selection()) {
    link new_link;
    new_link.x = current->select_x() / m_tile_width;
    new_link.y = current->select_y() / m_tile_height;
    new_link.width = std::abs(current->select_width() / m_tile_width);
    new_link.height = std::abs(current->select_height() / m_tile_height);
    edit_link link_window(*this);
    link_window.get(new_link);
    if (link_window.run() == Gtk::RESPONSE_OK) {
      new_link = link_window.get_link();
      get_current_level()->links.push_back(new_link);

      // update link list & level
      m_link_list.get();
      // TODO: also probably shouldn't be here
      current->queue_draw();
    }
  }
}

void level_editor::window::on_action_new() {
  try {
    std::auto_ptr<level_display> display(create_level_display());

    display->new_level(default_tile.get_tile());
    create_new_page(*Gtk::manage((display.release())), "new");
    set_level_buttons(true);
  } catch (std::exception& e) {
    display_error(e.what());
  }
}

void level_editor::window::on_action_open() {
  level_display& current_display = *get_current_level_display();

  if (m_fc_open.run() == Gtk::RESPONSE_OK) {
    std::list<Glib::ustring> files(m_fc_open.get_filenames());
    std::list<Glib::ustring>::const_iterator iter, end = files.end();
    for (iter = files.begin();
         iter != end;) {
      boost::filesystem::path path(*iter);
      ++iter;

      m_opening_levels = iter != end;
      try {
        // Only activate the last page to load
        load_level(path, !m_opening_levels);
      } catch (const std::exception& e) {
        display_error(e.what());
      }
    }
  }
  
  m_fc_open.hide();
}

void level_editor::window::on_action_save() {
  save_current_page();
}

void level_editor::window::on_action_save_as() {
  save_current_page_as();
}

bool level_editor::window::save_current_page_as() {
  level_display* disp = get_current_level_display();
  boost::filesystem::path path = disp->get_level_path();
  if (path.empty())
    m_fc_save.set_current_name("new.nw");
  else
    m_fc_save.set_filename(path.string());

  
  if (m_fc_save.run() == Gtk::RESPONSE_OK) {
    std::string path = m_fc_save.get_filename();
    level_display* disp = get_current_level_display();
    disp->save_level(path);
    m_fc_save.hide();
    return true;
  } else {
    m_fc_save.hide();
    return false;
  }
}

bool level_editor::window::save_current_page() {
  level_display* disp = get_current_level_display();
  boost::filesystem::path path = disp->get_level_path();
  if (path.empty()) {
    return save_current_page_as();
  }

  disp->save_level();
  return true;
}

void level_editor::window::on_action_quit() {
  if (close_all_levels())
    return;
  hide();
}

void level_editor::window::load_level(const boost::filesystem::path& file_path, bool activate) {
  // check whether the level is already loaded
  for (int i = 0; i < m_nb_levels.get_n_pages(); i ++) {
    if (get_nth_level_display(i)->get_level_path() == file_path) {
      if (activate)
        m_nb_levels.set_current_page(i);
      return;
    }
  }

  try {
    std::auto_ptr<level_display> display(create_level_display());
    //display_tileset.update_tileset(file_path.leaf());
    display->load_level(file_path);
    create_new_page(*Gtk::manage(display.release()), file_path.leaf(), activate);
    set_level_buttons(true);
  } catch (const std::exception& e) {
    display_error(e.what());
  }
}

void level_editor::window::create_new_page(level_display& display, const std::string& name, bool activate) {
  
  Gtk::ScrolledWindow* scrolled = Gtk::manage(new Gtk::ScrolledWindow());
  scrolled->add(display);
  
  tab_label* label = Gtk::manage(new tab_label(name));
  label->close_event().connect(sigc::bind(
    sigc::mem_fun(this, &level_editor::window::on_close_level_clicked),
    sigc::ref(*scrolled), sigc::ref(display)
  ));
  display.signal_title_changed().connect(
      sigc::mem_fun(*label, &tab_label::set_label));
  display.signal_unsaved_status_changed().connect(
      sigc::mem_fun(*label, &tab_label::set_unsaved_status));

  // TODO: Somehow prevent double on_switch_page here (on append_page and set_current_page)
  m_nb_levels.append_page(*scrolled, *label);
  m_nb_levels.set_tab_reorderable(*scrolled, true);
  m_nb_levels.show_all_children();
  if (activate)
    m_nb_levels.set_current_page(m_nb_levels.get_n_pages() - 1);
}

void level_editor::window::set_current_page(const level_display& display) {
  for (int i = 0; i < m_nb_levels.get_n_pages(); i ++) {
    if (get_nth_level_display(i) == &display) {
      m_nb_levels.set_current_page(i);
      return;
    }
  }

  throw std::runtime_error("set_current_page: no such page currently open");
}

void level_editor::window::on_close_level_clicked(Gtk::ScrolledWindow& scrolled, level_display& display) {
  if (display.get_unsaved()) {
    if (!confirm_changes(*this, display))
      return;
  }

  m_nb_levels.remove(scrolled);

  if (m_nb_levels.get_n_pages() <= 0) {
    set_level_buttons(false);

    m_link_list.hide();
    m_npc_list.hide();
    m_sign_list.hide();
  }
}

level_editor::level_display* level_editor::window::get_nth_level_display(int n) {
  // TODO: oh god
  Gtk::ScrolledWindow* scrolled =
    static_cast<Gtk::ScrolledWindow*>(m_nb_levels.get_nth_page(n));

  if (!scrolled)
    return 0;

  Gtk::Viewport* viewport =
    static_cast<Gtk::Viewport*>(scrolled->get_child());

  if (!viewport)
    return 0;

  return static_cast<level_display*>(viewport->get_child());
}

level_editor::level_display* level_editor::window::get_current_level_display() {
  return get_nth_level_display(m_nb_levels.get_current_page());
}

// Update the tileset of all level displays that start with prefix
void level_editor::window::update_matching_level_displays(const std::string& prefix) {
  level_display* display;
  for (int i = 0; i < m_nb_levels.get_n_pages(); ++i) {
    display = get_nth_level_display(i);
    if (display->get_level_path().leaf().find(prefix) == 0) {
      display_tileset.update_tileset(display->get_level_path().leaf());
    }
  }
}

Cairo::RefPtr<Cairo::ImageSurface> level_editor::window::get_image(const std::string& file_name) {
  Cairo::RefPtr<Cairo::ImageSurface> image;
  //std::cout << "window: loading image " << file_name << std::endl;

  try {
    image = m_image_cache.get_image(file_name);
  } catch (Cairo::logic_error&) {
    display_error("Filesystem error while trying to load " + file_name);
  } catch (std::bad_alloc& e) {
    display_error("Memory allocation error while trying to load `"
                  + file_name + "': " + e.what());
  } catch (std::exception& e) {
    display_error("Some generic error happened while trying to load `"
                  + file_name + "': " + e.what());
  }

  return image;
}


void level_editor::window::set_status(const std::string& text) {
  m_status_bar.pop();
  m_status_bar.push(text);
}

tile_buf level_editor::window::get_current_tile_selection() {
  level_display& disp = *get_current_level_display();
  tile_buf buf(disp.selection);
  if (buf.empty()) {
    disp.lift_selection();
    buf = disp.selection;
    disp.undo();
    return buf;
  }

  return tile_buf();
}

level_editor::window::signal_switch_level_display_type& level_editor::window::signal_switch_level_display() {
  return m_signal_switch_level_display;
}
