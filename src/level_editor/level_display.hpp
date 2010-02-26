#pragma once

#include <gtkmm.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include "level.hpp"
#include "tileset.hpp"
#include "preferences.hpp"
#include "undo_buffer.hpp"
#include "image_cache.hpp"
#include "ogl_tiles_display.hpp"
#include "ogl_texture_cache.hpp"

namespace Graal {
namespace level_editor {

class level_display: public ogl_tiles_display {
public:
  typedef std::vector<bool> layer_visibility_list_type;
  level_display(preferences& _prefs, image_cache& cache, int default_tile_index = 0);
  virtual ~level_display() {}

  void set_default_tile(int tile_index);

  void update_selection();

  // writes the selection to the level
  void save_selection();
  tile_buf selection;
  level::npc_list_type::iterator selected_npc;
  void clear_selection();
  void delete_selection();
  void lift_selection();
  void grab_selection();

  void load_level(const boost::filesystem::path& file_path);
  void set_level(Graal::level* _level);
  void new_level(int fill_tile);
  const boost::filesystem::path& get_level_path() const {
    return m_level_path;
  }
  void set_level_path(const boost::filesystem::path& new_path);
  void save_level();
  void save_level(const boost::filesystem::path& path);

  void set_selection(const Graal::npc& npc);
  bool in_selection(int x, int y);
  boost::shared_ptr<level>& get_level() { return m_level; }

  // Create new npc at mouse
  Graal::npc& drag_new_npc();
  // TODO: use c++0x move semantics, tiles will be swapped with the
  // current selection and then hopefully discarded
  void drag_selection(tile_buf& tiles, int drag_x, int drag_y);
  void drag_selection(level::npc_list_type::iterator npc_iter);

  inline int to_tiles_x(int x);
  inline int to_tiles_y(int y);

  // TODO: no
  bool npc_selected() { return (selected_npc != m_level->npcs.end()); }
  bool is_selecting() { return m_selecting; }
  bool has_selection() { return !selection.empty() || m_select_width > 0 || npc_selected(); }
  int select_x() { return m_select_x; }
  int select_y() { return m_select_y; }
  int select_tile_x() { return m_select_x / m_tile_width; }
  int select_tile_y() { return m_select_y / m_tile_height; }
  int select_width() { return m_select_width; }
  int select_height() { return m_select_height; }

  level_editor::undo_buffer undo_buffer;
  level_editor::undo_buffer redo_buffer;

  void undo();
  void redo();

  typedef sigc::signal<void, int> signal_default_tile_changed_type;
  signal_default_tile_changed_type& signal_default_tile_changed();

  typedef sigc::signal<void, const Glib::ustring&>
    signal_title_changed_type;
  signal_title_changed_type& signal_title_changed();

  typedef sigc::signal<void, bool> signal_unsaved_status_changed_type;
  signal_unsaved_status_changed_type& signal_unsaved_status_changed();

  typedef sigc::signal<void, const std::string&> signal_status_update_type;
  signal_status_update_type& signal_status_update();

  void flood_fill(int tx, int ty, int fill_with_index);

  void add_undo_diff(basic_diff* diff);

  void set_unsaved(bool new_unsaved);
  bool get_unsaved();

  void set_active_layer(int layer);
  int get_active_layer() { return m_active_layer; };

  virtual tile_buf& get_tile_buf();
  Cairo::RefPtr<Cairo::Surface> render_level(
      bool show_selection_border = true, bool show_selection = true,
      bool show_npcs = true, bool show_links = true, bool show_signs = true);

  void set_layer_visibility(std::size_t layer, bool visible);
  bool get_layer_visibility(std::size_t layer);
protected:
  void draw_tiles();
  void draw_selection();
  void draw_misc();
  virtual void draw_all();

  void draw_rectangle(float x, float y, float width, float height, float r, float g, float b, float a = 1.0, bool fill = false);
  //virtual bool on_expose_event(GdkEventExpose* event);
  void on_button_pressed(GdkEventButton* event);
  void on_button_released(GdkEventButton* event);
  void on_button_motion(GdkEventMotion* event);
  void on_mouse_leave(GdkEventCrossing* event);

  boost::shared_ptr<level> m_level;
  boost::filesystem::path m_level_path;

  preferences& m_preferences;

  // selection stuff
  bool m_selecting;
  int m_select_x, m_select_y, m_select_width, m_select_height;
  
  // currently creating a new npc
  bool m_new_npc;
  // drag stuff
  bool m_dragging;
  int m_drag_start_x, m_drag_start_y;
  // mouse offset from the selection origin
  int m_drag_mouse_x, m_drag_mouse_y;

  layer_visibility_list_type m_layer_visibility;
private:
  int m_active_layer;
  bool m_unsaved;

  int m_default_tile_index;

  image_cache& m_image_cache;
  ogl_texture_cache m_texture_cache;

  signal_default_tile_changed_type m_signal_default_tile_changed;
  signal_title_changed_type m_signal_title_changed;
  signal_unsaved_status_changed_type m_signal_unsaved_status_changed;
  signal_status_update_type m_signal_status_update;

};

}
}
