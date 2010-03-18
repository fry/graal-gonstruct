#include "level_display.hpp"
#include "helper.hpp"
#include "edit_npc.hpp"

#include <string>
#include <boost/scoped_ptr.hpp>
#include <sstream>
#include <queue>
#include <iostream>
#include <algorithm>

using namespace Graal;
using namespace Graal::level_editor;

// Convert cursor position to tile. Round to nearest tile.
inline int level_display::to_tiles_x(int x) {
  return (x + m_tile_width / 2) / m_tile_width;
  //return x / m_tile_width;
}

inline int level_display::to_tiles_y(int y) {
  return (y + m_tile_height / 2) / m_tile_height;
  //return y / m_tile_height;
}

level_display::level_display(
  level_editor::preferences& _prefs,
  image_cache& cache,
  int default_tile_index)
    : m_preferences(_prefs),
      m_default_tile_index(default_tile_index),
      m_image_cache(cache),
      m_texture_cache(cache), m_use_vbo(false),
      m_current_level_x(0), m_current_level_y(0) {
  add_events(Gdk::BUTTON_PRESS_MASK
             | Gdk::BUTTON_RELEASE_MASK
             | Gdk::BUTTON_MOTION_MASK
             | Gdk::POINTER_MOTION_MASK
             | Gdk::KEY_PRESS_MASK
             | Gdk::KEY_RELEASE_MASK
             | Gdk::LEAVE_NOTIFY_MASK);

  signal_button_press_event().connect_notify(
    sigc::mem_fun(this, &level_display::on_button_pressed));
  signal_button_release_event().connect_notify(
    sigc::mem_fun(this, &level_display::on_button_released));

  signal_motion_notify_event().connect_notify(
    sigc::mem_fun(this, &level_display::on_button_motion));

  signal_leave_notify_event().connect_notify(
    sigc::mem_fun(this, &level_display::on_mouse_leave));

  /*m_image_cache.signal_cache_update().connect(
    sigc::mem_fun(this, &level_display::update_all));*/

  m_selecting = false;
  m_dragging = false;
  m_new_npc = false;

  m_select_x = 0;
  m_select_y = 0;
  m_select_width = 0;
  m_select_height = 0;

  m_active_layer = 0;
  
  m_position_buffer = 0;
  m_texcoord_buffer = 0;

  m_unsaved = false;
}

void level_display::set_default_tile(int tile_index) {
  m_default_tile_index = tile_index;
}

void level_display::set_active_layer(int layer) {
  if (get_current_level()->tiles_exist(layer)) {
    m_active_layer = layer;
    invalidate();
  }
}

void level_display::load_level(const boost::filesystem::path& file_path) {
  set_level_map(new single_level_map_source(file_path));
}

void level_display::load_gmap(filesystem& fs, const boost::filesystem::path& file_path) {
  set_level_map(new gmap_level_map_source(fs, file_path));
}

// Takes ownership of the pointer
void level_display::set_level_map(level_map_source* level_source) {
  m_level_source.reset(level_source);
  m_level_map.reset(new level_map());
  m_level_map->set_level_source(m_level_source);

  m_level_map->signal_level_changed().connect(
    sigc::mem_fun(*this, &level_display::on_level_changed));

  // TODO: ???
  m_current_level_x = 0;
  m_current_level_y = 0;

  clear_selection();
}

void level_display::new_level(int fill_tile = 0) {
  level* new_level = new Graal::level(fill_tile);
  // The level name needs to be set by the host
  set_level_map(new single_level_map_source(""));
  m_level_map->set_level(new_level);
}

void level_display::save_current_level() {
  m_level_map->get_level_source()->save_level(m_current_level_x, m_current_level_y, get_current_level().get());
  set_unsaved(m_current_level_x, m_current_level_y, false);
}

void level_display::save_current_level(
    const boost::filesystem::path& path) {
  m_level_map->get_level_source()->set_level_name(
    m_current_level_x, m_current_level_y,
    path.string());
  save_current_level();
}

void level_display::save_selection() {
  // ignore empty selections
  if (selection.empty())
    return;

  const int sx = m_select_x / m_tile_width;
  const int sy = m_select_y / m_tile_height;
  const int sw = selection.get_width();
  const int sh = selection.get_height();

  int offset_left   = std::max(0, -sx);
  int offset_top    = std::max(0, -sy);
  int offset_right  = std::max(0, (sx + sw) - m_level_map->get_width_tiles() );
  int offset_bottom = std::max(0, (sy + sh) - m_level_map->get_height_tiles());

  const int actual_width = sw - offset_left - offset_right;
  const int actual_height = sh - offset_top - offset_bottom;

  // this can happen if a selection is dragged wholly outside the level
  if (actual_width < 0 || actual_height < 0)
    return;

  tile_buf buf; // make a bounds-checked copy of the selection while applying
  buf.resize(actual_width, actual_height);

  for (int x = offset_left; x < selection.get_width() - offset_right; ++x) {
    for (int y = offset_top; y < selection.get_height() - offset_bottom; ++y) {
      const int tx = x + sx;
      const int ty = y + sy;

      const tile& t = m_level_map->get_tile(tx, ty, m_active_layer);
      buf.get_tile(x - offset_left, y - offset_top) = t;
      m_level_map->set_tile(selection.get_tile(x, y), tx, ty, m_active_layer);
    }
  }

  // destroy buf
  add_undo_diff(new tile_diff(sx + offset_left, sy + offset_top, buf, m_active_layer));
}

void level_display::delete_selection() {
  if (npc_selected()) {
    add_undo_diff(new delete_npc_diff(selected_npc, *m_level_map->get_npc(selected_npc)));
    m_level_map->delete_npc(selected_npc);
    clear_selection();
  } else {
    // lift selection if we don't have one yet
    if (selection.empty())
      lift_selection();
  }
  clear_selection();
}

void level_display::clear_selection() {
  selection.clear();
  selected_npc = level_map::npc_ref();

  m_selecting = false;
  m_dragging = false;
  m_select_width = m_select_height = 0;
  
  invalidate();
}

bool level_display::in_selection(int x, int y) {
  return x >= m_select_x && x <= (m_select_x + m_select_width) &&
         y >= m_select_y && y <= (m_select_y + m_select_height);
}

void level_display::set_selection(const level_map::npc_ref& ref) {
  npc& npc = *m_level_map->get_npc(ref);
  
  // Set selection origin at the NPC's global position
  float npc_x, npc_y;
  m_level_map->get_global_npc_position(ref, npc_x, npc_y);

  m_select_x = static_cast<int>(npc_x * m_tile_width);
  m_select_y = static_cast<int>(npc_y * m_tile_height);

  // Set selection size to the image's size
  Cairo::RefPtr<Cairo::ImageSurface>& surface =
      m_image_cache.get_image(npc.image);
  m_select_width = surface->get_width();
  m_select_height = surface->get_height();
  
  invalidate();
}

tile_buf& level_display::get_tile_buf() {
  return get_current_level()->get_tiles(m_active_layer);
}

Cairo::RefPtr<Cairo::ImageSurface> level_display::render_level() {
  // Create two buffers, one to read frame buffer content into
  Cairo::RefPtr<Cairo::ImageSurface> surface =
    Cairo::ImageSurface::create(
      Cairo::FORMAT_ARGB32,
      get_width(),
      get_height()
    );
  // And one to y-flip the image into
  Cairo::RefPtr<Cairo::ImageSurface> dest =
    Cairo::ImageSurface::create(
      Cairo::FORMAT_ARGB32,
      get_width(),
      get_height()
    );

  // Read pixel data from OpenGL
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, get_width(), get_height(),
               GL_BGRA, GL_UNSIGNED_BYTE,
               surface->get_data());
  
  // Flip the image along the y-axis, since OpenGL's images are too
  Cairo::RefPtr<Cairo::Context> ct = Cairo::Context::create(dest);
  Cairo::Matrix flipy(1, 0, 0, -1, 0, get_height());
  ct->transform(flipy);
  ct->set_source(surface, 0, 0);
  ct->paint();
  return dest;
}

void level_display::on_button_motion(GdkEventMotion* event) {
  int x, y;
  get_cursor_position(x, y);

  const int tile_x = to_tiles_x(x);
  const int tile_y = to_tiles_y(y);

  const int tx = helper::bound_by(tile_x, 0, m_level_map->get_width_tiles() - 1);
  const int ty = helper::bound_by(tile_y, 0, m_level_map->get_height_tiles() - 1);

  if (m_dragging) {
    // Move selection
    const int delta_x = (to_tiles_x(x - m_drag_mouse_x) - to_tiles_x(m_select_x)) * m_tile_width;
    const int delta_y = (to_tiles_y(y - m_drag_mouse_y) - to_tiles_y(m_select_y)) * m_tile_height;

    const int new_select_x = m_select_x + delta_x;
    const int new_select_y = m_select_y + delta_y;

    // move npc if selected
    if (npc_selected()) {
      const int new_x = new_select_x / m_tile_width;
      const int new_y = new_select_y / m_tile_height;

      // Moves the NPC and updates the reference if the level changes
      m_level_map->move_npc(selected_npc, new_x, new_y);

      // Round select pos for NPCs in case of switching between 0.5 <-> 1 accuracy
      m_select_x = new_x * m_tile_width;
      m_select_y = new_y * m_tile_height;
    } else {
      m_select_x = new_select_x;
      m_select_y = new_select_y;
    }

    invalidate();
  } else if (m_selecting) {
    // extend selection rectangle or
    // if you move the mouse when you have a npc selected, switch to dragging
    if (npc_selected()) {
      m_dragging = true;
      m_drag_mouse_x = x - m_select_x;
      m_drag_mouse_y = y - m_select_y;

      float npc_x, npc_y;
      m_level_map->get_global_npc_position(selected_npc, npc_x, npc_y);
      m_drag_start_x = static_cast<int>(npc_x * m_tile_width);
      m_drag_start_y = static_cast<int>(npc_y * m_tile_height);
    } else {
      // the selection start tile needs to be limited to 0..<size>-1,
      // but in order to get correct width height these need to
      // go till <size>
      m_select_width =
        helper::bound_by(tile_x, 0, m_level_map->get_width_tiles()) * m_tile_width - m_select_x;
      m_select_height =
        helper::bound_by(tile_y, 0, m_level_map->get_height_tiles()) * m_tile_height - m_select_y;

      m_drag_start_x = tx * m_tile_width;
      m_drag_start_y = ty * m_tile_height;
    }
    
    invalidate();
  }

  std::ostringstream str;
  str
    << "Tile (" << tx << ", " << ty << "): "
    << m_level_map->get_tile(tx, ty, m_active_layer).index;
  m_signal_status_update(str.str());

  // change cursor
  if ((in_selection(x, y) || m_dragging) && !m_selecting) {
    Gdk::Cursor new_cursor(Gdk::FLEUR);
    get_window()->set_cursor(new_cursor);
  } else {
    get_window()->set_cursor();
  }
}

void level_display::on_button_pressed(GdkEventButton* event) {
  grab_focus();

  if (event->button == 1 || event->button == 3) {
    int x, y;
    get_cursor_position(x, y);

    const int tile_x = to_tiles_x(x);
    const int tile_y = to_tiles_y(y);

    // Double click
    if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
      // If we double clicked on a NPC, open edit dialog
      if (npc_selected()) {
        edit_npc dialog; // TODO: ???
        npc* npc = m_level_map->get_npc(selected_npc);
        dialog.set(*npc);
        if (dialog.run() == Gtk::RESPONSE_OK) {
          Graal::npc new_npc = dialog.get_npc();
          if (new_npc != *npc) {
            add_undo_diff(new npc_diff(selected_npc, *npc));
          }
          (*npc) = new_npc;
        }
        m_dragging = false;
        m_selecting = false;
        
        set_selection(selected_npc);

        return invalidate();
      }

      // set default tile
      if (!has_selection()) {
        const int tile_index = m_level_map->get_tile(x/m_tile_width, y/m_tile_height, m_active_layer).index;
        m_signal_default_tile_changed.emit(tile_index);
      }
    }


    // abort if we are currently dragging
    if (m_dragging) {
      // or clear the selection if press the right mouse button
      if (event->button == 3 && !npc_selected()) {
        delete_selection();
      }
      return invalidate();
    }

    // Did we click on a selection?
    if (in_selection(x, y)) {
      if (event->button == 3) {
        // write selection if we right click
        if (npc_selected()) {
          // Copy selected NPC
          npc new_npc = *m_level_map->get_npc(selected_npc);
          // Retrieve level of selected NPC
          level* npc_level = m_level_map->get_level(selected_npc.level_x, selected_npc.level_y).get();
          // Create new NPC in that level
          selected_npc.id = npc_level->add_npc(new_npc).id;
          add_undo_diff(new create_npc_diff(selected_npc));
          m_new_npc = true;
        } else {
          if (selection.empty()) {
            grab_selection();
          } else {
            save_selection();
          }
        }
      } else if (!npc_selected() && selection.empty()) {
        lift_selection();
      } 

      // start dragging
      m_dragging = true;
      m_drag_mouse_x = x - m_select_x;
      m_drag_mouse_y = y - m_select_y;

      return;
    } else {
      if (has_selection()) {
        // If not and we already have one, copy selection to map and clear selection
        save_selection();
        clear_selection();
      } else {
       // flood fill if right clicked
        if (event->button == 3) {
          // select the exact tile for flood filling
          const int down_tile_x = x / m_tile_width;
          const int down_tile_y = y / m_tile_height;
          flood_fill(down_tile_x, down_tile_y, m_default_tile_index);
        }
      }

      // prevent selecting out of bounds
      if (x > m_level_map->get_width_tiles() * m_tile_width ||
          y > m_level_map->get_height_tiles() * m_tile_height)
        return invalidate();

      // otherwise select tiles only with left mouse button
      if (!npc_selected() && event->button == 1) {   
        m_select_x = tile_x * m_tile_width;
        m_select_y = tile_y * m_tile_height;

        m_select_width = 0;
        m_select_height = 0;

        m_selecting = true;
      }
    }

    // Check whether we clicked a npc and select it
    if (!m_preferences.hide_npcs) {
      // Determine the level the mouse is in
      const int level_width = m_level_map->get_level_width();
      const int level_height = m_level_map->get_level_height();

      const int mouse_level_x = tile_x / level_width;
      const int mouse_level_y = tile_y / level_height;

      // The position of the mouse inside the level, in pixel
      const int mouse_pixel_level_x = x - mouse_level_x * level_width * m_tile_width;
      const int mouse_pixel_level_y = y - mouse_level_y * level_height * m_tile_height;

      level* mouse_level = m_level_map->get_level(mouse_level_x, mouse_level_y).get();

      if (!mouse_level)
        return;

      // Iterate through NPCs in that level
      level::npc_list_type::iterator iter, end;
      end = mouse_level->npcs.end();
      for (iter = mouse_level->npcs.begin(); iter != end; iter ++) {
        // Retrieve NPC image dimensions
        Cairo::RefPtr<Cairo::ImageSurface> npc_image =
          m_image_cache.get_image(iter->image);
        const int npc_x = static_cast<int>(iter->get_level_x() * m_tile_width);
        const int npc_y = static_cast<int>(iter->get_level_y() * m_tile_height);
        const int npc_width = npc_image->get_width();
        const int npc_height = npc_image->get_height();
        if (mouse_pixel_level_x >= npc_x && mouse_pixel_level_x < npc_x + npc_width
            && mouse_pixel_level_y >= npc_y && mouse_pixel_level_y < npc_y + npc_height) {
          // NPC clicked; deselect previous selection
          if (has_selection()) {
            save_selection();
            clear_selection();
          }

          // Select the clicked on NPC and update selection
          selected_npc.id = iter->id;
          selected_npc.level_x = mouse_level_x;
          selected_npc.level_y = mouse_level_y;
          set_selection(selected_npc);

          m_selecting = true;

          return invalidate();
        }
      }
    }
  }
  
  invalidate();
}


void level_display::on_button_released(GdkEventButton* event) {
  if (event->button == 1 || event->button == 3) {
    int x, y;
    get_cursor_position(x, y);

    // Stop dragging
    if (m_dragging) {
      // if no new npc was dragged, add npc changed diff
      const int tile_x = to_tiles_x(x);
      const int tile_y = to_tiles_y(y);
      if (!m_new_npc && npc_selected()) {
        const float dsx = static_cast<float>(m_drag_start_x) / m_tile_width;
        const float dsy = static_cast<float>(m_drag_start_y) / m_tile_height;
        // Did the position actually change?
        if (dsx != tile_x || dsy != tile_y) {
          // Add npc move diff
          add_undo_diff(new move_npc_diff(selected_npc, dsx, dsy));
        }
      }
      m_new_npc = false;

      m_dragging = false;
    }

    // Stop selecting 
    if (m_selecting) {
      m_selecting = false;

      // select tiles if no npc is selected 
      if (!npc_selected()) {
        // for selections going into a negative direction
        if (m_select_width < 0) {
          m_select_x += m_select_width;
          m_select_width = std::abs(m_select_width);
        }

        if (m_select_height < 0) {
          m_select_y += m_select_height;
          m_select_height = std::abs(m_select_height);
        }
      }
    }
  }
}

void level_display::lift_selection() {
  const int sx = to_tiles_x(m_select_x);
  const int sy = to_tiles_y(m_select_y);
  const int sw = to_tiles_x(m_select_width);
  const int sh = to_tiles_y(m_select_height);

  int offset_left   = std::max(0, -sx);
  int offset_top    = std::max(0, -sy);
  int offset_right  = std::max(0, (sx + sw) - m_level_map->get_width_tiles() );
  int offset_bottom = std::max(0, (sy + sh) - m_level_map->get_height_tiles());

  const int actual_width = sw - offset_left - offset_right;
  const int actual_height = sh - offset_top - offset_bottom;

  // this can happen if a selection is dragged wholly outside the level
  if (actual_width < 0 || actual_height < 0)
    return;

  selection.clear();
  selection.resize(sw, sh);
  tile_buf buf;
  buf.resize(actual_width, actual_height);

  for (int x = offset_left; x < selection.get_width() - offset_right; ++x) {
    for (int y = offset_top; y < selection.get_height() - offset_bottom; ++y) {
      const int tx = x + sx;
      const int ty = y + sy;

      tile t = m_level_map->get_tile(tx, ty, m_active_layer);
      selection.get_tile(x, y)
        = buf.get_tile(x - offset_left, y - offset_top)
        = t;

      // set to default tile
      // TODO: set to transparent tile on layers > 0
      t.index = m_default_tile_index;
      m_level_map->set_tile(t, tx, ty, m_active_layer);
    }
  }

  // destroy buf
  add_undo_diff(new tile_diff(sx + offset_left, sy + offset_right, buf, m_active_layer));
  invalidate();
}

void level_display::grab_selection() {
  lift_selection();
  save_selection();
  for (int i = 0; i < 2; ++i) {
    boost::scoped_ptr<basic_diff> p(undo_buffer.apply(*m_level_map));
    p->apply(*m_level_map);
  }
  // TODO: this is sooo wasteful, so it should probably be the other way around
}

// tiles
void level_display::drag_selection(tile_buf& tiles,
                                       int drag_x, int drag_y) {
  int x, y;
  get_cursor_position(x, y);

  // save a previous selection
  //if (has_selection()) {
    if (!m_dragging) {
      save_selection();
    }
    clear_selection();
  //}

  selection.swap(tiles);

  const int tw = m_tile_width;
  const int th = m_tile_height;

  m_drag_mouse_x = drag_x * tw;
  m_drag_mouse_y = drag_y * th;

  m_select_x = (x/tw)*tw - m_drag_mouse_x;
  m_select_y = (y/tw)*tw - m_drag_mouse_y;
  m_select_width = selection.get_width() * m_tile_width;
  m_select_height = selection.get_height() * m_tile_height;

  m_dragging = true;
  
  invalidate();
}

// npc
void level_display::drag_selection(const level_map::npc_ref& ref) {
  int x, y;
  get_cursor_position(x, y);

  // Deselect and save a previous selection
  if (!m_dragging) {
    save_selection();
  }
  clear_selection();

  // Set the NPC as selected and updated selection
  m_new_npc = true;
  selected_npc = ref;
  set_selection(ref);

  // Position NPC at mouse
  const int tx = to_tiles_x(x);
  const int ty = to_tiles_y(y);

  const int tw = m_tile_width;
  const int th = m_tile_height;

  // Set drag offset
  m_drag_mouse_x = 3 * tw;
  m_drag_mouse_y = 3 * th;

  m_select_x = tx * tw - m_drag_mouse_x;
  m_select_y = ty * th - m_drag_mouse_y;
  m_select_height = 3 * tw;
  m_select_width = 3 * th;

  m_level_map->move_npc(selected_npc, to_tiles_x(m_select_x), to_tiles_y(m_select_y));

  m_dragging = true;
  
  invalidate();
}

void level_display::drag_selection(const npc& _npc) {
  npc& new_npc = get_current_level()->add_npc(_npc);
  level_map::npc_ref ref;
  ref.level_x = m_current_level_x;
  ref.level_y = m_current_level_y;
  ref.id = new_npc.id;
  drag_selection(ref);
}

void level_display::undo() {
  if (has_selection()) {
    save_selection();
  }

  if (undo_buffer.empty())
    return;

  redo_buffer.push(undo_buffer.apply(*m_level_map));
  //set_unsaved(true); // TODO!!
  invalidate();
}

void level_display::redo() {
  if (redo_buffer.empty())
    return;

  undo_buffer.push(redo_buffer.apply(*m_level_map));
  //set_unsaved(true); // TODO!!
  invalidate();
}

void level_display::add_undo_diff(basic_diff* diff) {
  undo_buffer.push(diff);
  redo_buffer.clear();

  //set_unsaved(true); // TODO!!
}

level_display::signal_default_tile_changed_type&
level_display::signal_default_tile_changed() {
  return m_signal_default_tile_changed;
}

level_display::signal_title_changed_type&
level_display::signal_title_changed() {
  return m_signal_title_changed;
}

level_display::signal_unsaved_status_changed_type&
level_display::signal_unsaved_status_changed() {
  return m_signal_unsaved_status_changed;
}

level_display::signal_status_update_type&
level_display::signal_status_update() {
  return m_signal_status_update;
}

Graal::npc& level_display::drag_new_npc() {
  Graal::npc& npc = get_current_level()->add_npc();
  level_map::npc_ref ref;
  ref.id = npc.id;
  ref.level_x = m_current_level_x;
  ref.level_y = m_current_level_y;
  drag_selection(ref);
  add_undo_diff(new create_npc_diff(ref));
  return npc;
}

void level_display::flood_fill(int tx, int ty, int fill_with_index) {
  static int vec_x[] = {-1, 0, 1, 0};
  static int vec_y[] = { 0, -1, 0, 1};

  // the index of the tiles to fill
  int fill_index = m_level_map->get_tile(tx, ty, m_active_layer).index;
  // Abort if the start tile's index is the same as the fill tile
  if (fill_with_index == fill_index)
    return;
  m_level_map->set_tile(tile(fill_with_index), tx, ty, m_active_layer);


  std::queue<std::pair<int, int> > queue;
  queue.push(std::pair<int, int>(tx, ty));

  std::list<std::pair<int, int> > changed_tiles;
  int start_x = m_level_map->get_width_tiles();
  int start_y = m_level_map->get_height_tiles();
  int end_x = -1;
  int end_y = -1;
  
  while (!queue.empty()) {
    std::pair<int, int> current_node = queue.front(); queue.pop();
    changed_tiles.push_back(current_node);

    int cx = current_node.first;
    int cy = current_node.second;

    // Determine bounding box
    if (cx < start_x) start_x = cx;
    if (cy < start_y) start_y = cy;
    if (cx > end_x) end_x = cx;
    if (cy > end_y) end_y = cy;

    //std::cout << "fill " << cx << ", " << cy << ": " << fill_index << " = " << fill_with_index << std::endl;
    for (int i = 0; i < 4; i ++) {
      // continue for all adjacent tiles that have the same index as the current tile
      int current_tx = cx + vec_x[i];
      int current_ty = cy + vec_y[i];

      if (current_tx >= 0 && current_tx < m_level_map->get_width_tiles() &&
          current_ty >= 0 && current_ty < m_level_map->get_height_tiles()) {
        int adjacent_tile_index = m_level_map->get_tile(current_tx, current_ty, m_active_layer).index;
        if (adjacent_tile_index == fill_index) {
          queue.push(std::pair<int, int>(current_tx, current_ty));
          m_level_map->set_tile(tile(fill_with_index), current_tx, current_ty, m_active_layer);
        }
      }
    }
  }

  const int width  = end_x - start_x + 1;
  const int height = end_y - start_y + 1;

  // create diff
  tile_buf buffer;
  buffer.resize(width, height);

  // copy old level part
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      const int cx = start_x + x;
      const int cy = start_y + y;
      tile& current_tile = buffer.get_tile(x, y);
      current_tile = m_level_map->get_tile(cx, cy, m_active_layer);
    }
  }

  // and lay over changed tiles
  std::list<std::pair<int, int> >::iterator it, end = changed_tiles.end();
  for (it = changed_tiles.begin(); it != end; ++it) {
    const int cx = it->first - start_x;
    const int cy = it->second - start_y;

    buffer.get_tile(cx, cy).index = fill_index;
  }
  add_undo_diff(new tile_diff(start_x, start_y, buffer, m_active_layer));
  
  invalidate();
}

void level_display::on_mouse_leave(GdkEventCrossing* event) {
  m_signal_status_update("");
}

void level_display::set_layer_visibility(std::size_t layer, bool visible) {
  if (layer >= m_layer_visibility.size())
    m_layer_visibility.resize(layer + 1, true);
  m_layer_visibility[layer] = visible;
  invalidate();
}

bool level_display::get_layer_visibility(std::size_t layer) {
  if (layer < m_layer_visibility.size())
    return m_layer_visibility[layer];
  return true;
}

void level_display::draw_rectangle(float x, float y, float width, float height, float r, float g, float b, float a, bool fill) {
  glDisable(GL_TEXTURE_2D);

  if (fill) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
      glVertex2f(x, y);
      glVertex2f(x + width, y);
      glVertex2f(x + width, y + height);
      glVertex2f(x, y + height);
    glEnd();
  }

  glColor3f(r, g, b);
  glLineWidth(2);
  glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
  glEnd();
  glEnable(GL_TEXTURE_2D);
}

void level_display::draw_tiles(level* current_level) {
  /* Set up the level vertices if we don't have a buffer and are using VBOS
   * or if we're using vertex arrays and don't have vertices generated */
  if (((!m_position_buffer || !m_texcoord_buffer) && m_use_vbo) ||
      (!m_use_vbo && m_positions.empty()))
    setup_buffers();
 
  // TODO: handle different tilesets per level
  // Each tile needs 4 vertices and 4 texcoords
  const std::size_t size = current_level->get_width() * current_level->get_height() * 4;
  std::vector<vertex_texcoord> tcoords; tcoords.resize(size);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, m_tileset);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  if (m_use_vbo) {
    // Bind VBOs
    glBindBuffer(GL_ARRAY_BUFFER, m_position_buffer);
    glVertexPointer(2, GL_INT, sizeof(vertex_position), 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_texcoord_buffer);
  } else {
    // Link vertex array
    glVertexPointer(2, GL_INT, sizeof(vertex_position), &m_positions.front());
  }

  // Draw each layer
  int layer_count = current_level->get_layer_count();
  for (int i = 0; i < layer_count; i ++) {
    // If it's visible
    if (get_layer_visibility(i)) {
      // With its own set of tiles
      tile_buf& tiles = current_level->get_tiles(i);
      const int width = tiles.get_width();
      const int height = tiles.get_height();

      // Draw layers below the current darker, above transparent
      glColor3f(1, 1, 1);
      if (m_preferences.fade_layers) {
        if (i > m_active_layer) {
          int level_diff = std::abs(m_active_layer - i);
          glColor4f(1, 1, 1, std::pow(0.5, level_diff));
        } else if (i < m_active_layer) {
          glColor4f(0.5, 0.5, 0.5, 1);
        }
      }

      /* We need to draw this in chunks so we can skip transparent tiles */
      int current_start = 0;
      int current_length = 0;
      std::list<std::pair<int, int> > chunks;
      // Build texture coordinates
      for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
          const tile& _tile = tiles.get_tile(x, y);
          // Add a new chunk once a transparent tile is reached
          if (_tile.transparent()) {
            if (current_length > 0) {
              chunks.push_back(std::pair<int, int>(current_start, current_length));
              current_start += current_length;
              current_length = 0;
            }

            // Skip this tile
            current_start ++;
            continue;
          }

          // The position of the actual tile inside the tileset
          const int tx = helper::get_tile_x(_tile.index);
          const int ty = helper::get_tile_y(_tile.index);

          // Build texture coordinates
          float x1 = (float)(tx * m_tile_width)/m_tileset_width;
          float x2 = (float)((tx+1)*m_tile_width)/m_tileset_width;
          float y1 = (float)(ty*m_tile_height)/m_tileset_height;
          float y2 = (float)((ty+1)*m_tile_height)/m_tileset_height;
          
          // Fill texcoord array at the current vertex position
          int index = (x * height + y) * 4;

          tcoords[index++] = vertex_texcoord(x1, y1);
          tcoords[index++] = vertex_texcoord(x2, y1);
          tcoords[index++] = vertex_texcoord(x2, y2);
          tcoords[index  ] = vertex_texcoord(x1, y2);

          current_length ++;
        }
      }
      // And one last chunk
      if (current_length > 0) {
        chunks.push_back(std::pair<int, int>(current_start, current_length));
      }

      if (m_use_vbo) {
        // Upload texture coordinates and draw buffer
        glBufferData(GL_ARRAY_BUFFER,
                     tcoords.size() * sizeof(vertex_texcoord),
                     &tcoords.front(),
                     GL_STREAM_DRAW);
        glTexCoordPointer(2, GL_FLOAT, 0, 0);
      } else {
        // Link texcoord array
        glTexCoordPointer(2, GL_FLOAT, 0, &tcoords.front());
      }
      // Draw all collected chunks
      std::list<std::pair<int, int> >::iterator iter, end = chunks.end();
      for (iter = chunks.begin(); iter != end; ++iter) {
        // Indices are per tile, but we draw 4 vertices per tile
        glDrawArrays(GL_QUADS, iter->first * 4, iter->second * 4);
      }
    }
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glBindTexture(GL_TEXTURE_2D, 0);
}


void level_display::draw_selection() {
  if (!selection.empty()) {
    // Draw at selection position
    glPushMatrix();
    glTranslatef(m_select_x, m_select_y, 0);
    glColor4f(1, 1, 1, 1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_tileset);
    // Draw selection from its tile_buf
    const int width = selection.get_width();
    const int height = selection.get_height();
    glBegin(GL_QUADS);
    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        draw_tile(selection.get_tile(x, y), x, y);
      }
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
  }

  // Show selection rectangle when we are selecting or there's a selection and
  // we are not currently dragging except when the relevant preference is set
  bool show_border = !m_dragging || m_preferences.selection_border_while_dragging;
  if (show_border && (m_selecting || m_select_width*m_select_height != 0 || npc_selected())) {
    // reload selection width/height for npcs
    // TODO: can we somehow get rid of this?
    if (npc_selected()) {
      npc* npc = m_level_map->get_npc(selected_npc);
      Cairo::RefPtr<Cairo::ImageSurface> img =
          m_image_cache.get_image(npc->image);
      m_select_width = img->get_width();
      m_select_height = img->get_height();
    }

    draw_rectangle(
      m_select_x, m_select_y,
      m_select_width, m_select_height,
      0.7, 1, 1, 0.2,
      m_preferences.selection_background);
  }
}

void level_display::draw_misc(level* current_level) {
  // NPCs
  if (!m_preferences.hide_npcs) {
    glColor3f(1, 1, 1);
    Graal::level::npc_list_type::iterator npc_iter, npc_end = current_level->npcs.end();
    for (npc_iter = current_level->npcs.begin(); npc_iter != npc_end; npc_iter ++) {
      const int x = static_cast<int>(npc_iter->get_level_x() * m_tile_width);
      const int y = static_cast<int>(npc_iter->get_level_y() * m_tile_height);
      const std::string& npc_image_file = npc_iter->image;
      Cairo::RefPtr<Cairo::ImageSurface>& npc_img = m_image_cache.get_image(npc_image_file);
      const int width = npc_img->get_width();
      const int height = npc_img->get_height();
      
      glBindTexture(GL_TEXTURE_2D, m_texture_cache.get_texture(npc_image_file));
      glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(x, y);
        glTexCoord2f(1, 0);
        glVertex2f(x+width, y);
        glTexCoord2f(1, 1);
        glVertex2f(x+width, y+height);
        glTexCoord2f(0, 1);
        glVertex2f(x, y+height);
      glEnd();
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }

  // Signs
  if (!m_preferences.hide_signs) {
    Graal::level::sign_list_type::iterator sign_iter, sign_end = current_level->signs.end();
    for (sign_iter = current_level->signs.begin(); sign_iter != sign_end; sign_iter ++) {
      draw_rectangle(
        sign_iter->x * m_tile_width, sign_iter->y * m_tile_height,
        2 * m_tile_width, 1 * m_tile_height, // Signs are by default 2x1 tiles big
        1, 0, 0, 0.2,
        true);
    }
  }

  // Links
  if (!m_preferences.hide_links) {
    Graal::level::link_list_type::iterator link_iter, link_end = current_level->links.end();
    for (link_iter = current_level->links.begin(); link_iter != link_end; link_iter ++) {
      draw_rectangle(
        link_iter->x * m_tile_width, link_iter->y * m_tile_height,
        link_iter->width * m_tile_width, link_iter->height * m_tile_height,
        1, 1, 0.4, 0.2,
        true);
    }
  }
}

void level_display::draw_all() { 
  // Apply scroll offset
  int offset_x, offset_y;
  get_scroll_offset(offset_x, offset_y);
  glPushMatrix();
  glTranslatef(-offset_x, -offset_y, 0);

  const int map_width = m_level_map->get_width();
  const int map_height = m_level_map->get_height();

  const int level_width = m_level_map->get_level_width();
  const int level_height = m_level_map->get_level_height();

  m_current_level_x = helper::bound_by((offset_x + get_width()/2)/m_tile_width/level_width, 0, map_width);
  m_current_level_y = helper::bound_by((offset_y + get_height()/2)/m_tile_height/level_height, 0, map_height);

  // Send level name changed and unsaved changed signals
  m_signal_title_changed(get_current_level_path().leaf());
  m_signal_unsaved_status_changed(m_unsaved_levels[std::pair<int, int>(m_current_level_x, m_current_level_y)]);

  // Draw surrounding levels
  const int start_x = std::max(0, m_current_level_x - 1);
  const int start_y = std::max(0, m_current_level_y - 1);
  const int end_x = std::min(map_width - 1, m_current_level_x + 1) + 1;
  const int end_y = std::min(map_height - 1, m_current_level_y + 1) + 1;

  // Do this in two loops so the tiles get drawn below everything
  // TODO: enable z buffer again?
  // TODO: do bounds checking with viewport to only draw those levels that are actually visible
  for (int x = start_x; x < end_x; x++) {
    for (int y = start_y; y < end_y; y++) {
      const int screen_level_x = x * level_width * m_tile_width,;
      const int screen_level_y = y * level_height * m_tile_height;  
      level* current_level = m_level_map->get_level(x, y).get();
      // Draw level at the correct position
      if (current_level) {
        glPushMatrix();
        glTranslatef(screen_level_x, screen_level_y, 0);
        draw_tiles(current_level);
        glPopMatrix();
      }
    }
  }

  for (int x = start_x; x < end_x; x++) {
    for (int y = start_y; y < end_y; y++) {
      const int screen_level_x = x * level_width * m_tile_width,;
      const int screen_level_y = y * level_height * m_tile_height;  
      level* current_level = m_level_map->get_level(x, y).get();
      if (current_level) {
        // Draw level at the correct position
        glPushMatrix();
        glTranslatef(screen_level_x, screen_level_y, 0);
        draw_misc(current_level);
        glPopMatrix();
      }
    }
  }

  draw_selection();

  glPopMatrix();
}

void level_display::setup_buffers() {
  m_use_vbo = glewIsSupported("GL_ARB_vertex_buffer_object");

  if (!m_use_vbo) {
    std::cerr << "level_display::setup_buffers: no ARB_vertex_buffer_object support, using vertex arrays" << std::endl;
  }
  
  if (m_use_vbo) {
    if (!m_position_buffer)
      glGenBuffers(1, &m_position_buffer);
    if (!m_texcoord_buffer)
      glGenBuffers(1, &m_texcoord_buffer);
  }

  // Each tile needs 4 vertices and 4 texcoords
  const int width = m_level_map->get_level_width();
  const int height = m_level_map->get_level_height();
  const std::size_t size = width * height * 4;

  m_positions.reserve(size);

  // fill with vertex positions
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      const int tx = x * m_tile_width;
      const int ty = y * m_tile_height;
      m_positions.push_back(vertex_position(tx, ty));
      m_positions.push_back(vertex_position(tx + m_tile_width, ty));
      m_positions.push_back(vertex_position(tx + m_tile_width, ty + m_tile_height));
      m_positions.push_back(vertex_position(tx, ty + m_tile_height));
    }
  }


  if (m_use_vbo) {
    // Load the data into the VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_position_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 size * sizeof(vertex_position),
                 &m_positions.front(),
                 GL_STATIC_DRAW);
    m_positions.clear();
  }
}

link level_editor::level_display::create_link() {
  link new_link;
  new_link.x = m_select_x / m_tile_width;
  new_link.y = m_select_y / m_tile_height;
  new_link.width = std::abs(m_select_width / m_tile_width);
  new_link.height = std::abs(m_select_height / m_tile_height);
  return new_link;
}

const boost::shared_ptr<level>& level_editor::level_display::get_current_level() {
  return m_level_map->get_level(
      m_current_level_x,
      m_current_level_y);
}

void level_editor::level_display::set_surface_size() {
  level_editor::ogl_tiles_display::set_surface_size();
  set_scroll_size(
    m_level_map->get_width_tiles() * m_tile_width,
    m_level_map->get_height_tiles() * m_tile_height);
}

const boost::filesystem::path level_editor::level_display::get_current_level_path() const {
  return m_level_source->get_level_name(m_current_level_x, m_current_level_y);
}

void level_editor::level_display::set_unsaved(int level_x, int level_y, bool new_unsaved) {
  std::pair<int, int> level_key(level_x, level_y);
  m_unsaved_levels[level_key] = new_unsaved;
}

void level_editor::level_display::focus_level(int level_x, int level_y) {
  set_scroll_offset(
    level_x * m_level_map->get_level_width() * m_tile_width,
    level_y * m_level_map->get_level_height() * m_tile_height);
}

void level_display::on_level_changed(int x, int y) {
  set_unsaved(x, y, true);
}

bool level_display::on_key_press_event(GdkEventKey* event) {
  int ox, oy;
  get_scroll_offset(ox, oy);

  /* Scroll a max of 10 tiles */
  int step_x = std::min(m_tile_width * 10, static_cast<int>(m_hadjustment->get_page_increment()));
  int step_y = std::min(m_tile_height * 10, static_cast<int>(m_vadjustment->get_page_increment()));
  switch (event->keyval) {
  case GDK_Left:
    ox -= step_x;
    break;
  case GDK_Right:
    ox += step_x;
    break;
  case GDK_Up:
    oy -= step_y;
    break;
  case GDK_Down:
    oy += step_y;
    break;
  }
  set_scroll_offset(ox, oy);

  return true;
}
