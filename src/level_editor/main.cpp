#include "window.hpp"
#include "preferences.hpp"
#include "preferences_display.hpp"
#include <gtkmm.h>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/convenience.hpp>

namespace {
  // Called AFTER loading
  void set_default_preferences(Graal::level_editor::preferences& prefs) {
    // Default tileset
    if (prefs.tilesets.empty()) {
      prefs.add_tileset("pics1.png", "");
      prefs.default_tile = 511; // clear grass tile
    }
  }

  void start_editor(int argc, char* argv[],
                    boost::scoped_ptr<Graal::level_editor::window>& editor,
                    Graal::level_editor::preferences& prefs) {
    if (!boost::filesystem::is_directory(prefs.graal_dir)) {
      Gtk::Main::quit();
      return;
    }

    try {
      editor.reset(new Graal::level_editor::window(prefs));
    } catch (std::runtime_error& e) {
      Gtk::MessageDialog dialog("Error", false,
          Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
      dialog.set_secondary_text(
          Glib::ustring("Could not start editor: ") + e.what());
      dialog.run();
      Gtk::Main::quit();
      return;
    }

    // Load passed level arguments
    if (argc > 1) {
      for (int i = 1; i < argc; i ++) {
        if (argv[i][0] != '-')
          editor->load_level(argv[i]);
      }
    }

    editor->signal_hide().connect(sigc::ptr_fun(&Gtk::Main::quit));
    editor->show_all();
  }
}

int main(int argc, char* argv[]) {
  Gtk::Main kit(argc, argv);

  if (!gdk_gl_query()) {
    std::cerr << "No OpenGL support" << std::endl;
    return -1;
  }

  Graal::level_editor::preferences prefs;

  boost::filesystem::path preferences_path;
  preferences_path = Glib::get_user_config_dir();
  preferences_path = preferences_path / "gonstruct";
  
  // Make sure all directories to this path exist
  boost::filesystem::create_directories(preferences_path);

  prefs.load(preferences_path / "preferences");
  set_default_preferences(prefs);
  { // destroy window before preferences get serialised
    boost::scoped_ptr<Graal::level_editor::preferences_display> prefs_display;
    boost::scoped_ptr<Graal::level_editor::window> editor;

    if (!boost::filesystem::is_directory(prefs.graal_dir)) {
      prefs_display.reset(new Graal::level_editor::preferences_display(prefs));
      prefs_display->signal_hide().connect(
          sigc::bind(&start_editor, argc, argv,
                                    sigc::ref(editor), sigc::ref(prefs)));
      prefs_display->show_all();
    } else {
      Glib::signal_idle().connect(
          sigc::bind_return(
            sigc::bind(&start_editor, argc, argv,
                       sigc::ref(editor), sigc::ref(prefs)),
            false));
    }

    kit.run();
  }
  prefs.save(preferences_path / "preferences");

  return 0;
}
