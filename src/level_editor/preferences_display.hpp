#ifndef GRAAL_LEVEL_EDITOR_PREFERENCES_EDITOR_HPP_
#define GRAAL_LEVEL_EDITOR_PREFERENCES_EDITOR_HPP_

#include <gtkmm.h>
#include "preferences.hpp"

namespace Graal {
  namespace level_editor {
    class preferences_display : public Gtk::Dialog {
    public:
      static const int NOTHING_CHANGED                       = 0;
      static const int GRAAL_DIR_CHANGED                     = 1 << 0;
      static const int SHOW_SELECTION_WHILE_DRAGGING_CHANGED = 1 << 1;
      static const int REMEMBER_DEFAULT_TILE_CHANGED         = 1 << 2;
      static const int STICKY_TILE_SELECTION                 = 1 << 3;

      typedef int preference_changes;

      typedef sigc::signal<void, preference_changes>
          signal_preferences_changed_type;
      preferences_display(preferences& _prefs);

      void update_controls();

      signal_preferences_changed_type& signal_preferences_changed();

    private:
      virtual void on_response(int response_id);

      void apply();

      preferences& m_prefs;
      signal_preferences_changed_type m_signal_preferences_changed;

      Gtk::FileChooserButton m_pref_graaldir;
      Gtk::CheckButton       m_pref_selection_border_while_dragging;
      Gtk::CheckButton       m_pref_remember_default_tile;
      Gtk::CheckButton       m_pref_sticky_tile_selection;
    };
  }
}

#endif
