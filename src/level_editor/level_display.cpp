#include "level_display.hpp"
#include "npc_list.hpp"
#include "helper.hpp"
#include <string>
#include <boost/scoped_ptr.hpp>
#include <sstream>
#include <queue>

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
      m_image_cache(cache) {
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

  m_unsaved = false;
}

void level_display::set_default_tile(int tile_index) {
  m_default_tile_index = tile_index;
}

void level_display::set_active_layer(int layer) {
  if (m_level->tiles_exist(layer)) {
    m_active_layer = layer;
  }
}

void level_display::load_level(const boost::filesystem::path& file_path) {
  m_level_path = file_path;
  set_level(Graal::load_nw_level(file_path.string()));
}

// Takes ownership of the pointer
void level_display::set_level(Graal::level* _level) {
  m_level.reset(_level);
  selected_npc = m_level->npcs.end();

  // Show all layers initially
  int layer_count = m_level->get_layer_count();
  for (int i = 0; i < layer_count; i ++) {
    set_layer_visibility(i, true);
  }

  set_surface_buffers();
}

void level_display::new_level(int fill_tile = 0) {
  set_level(new Graal::level(fill_tile));
}

void level_display::set_level_path(
    const boost::filesystem::path& new_path) {
  m_level_path = new_path;
  m_signal_title_changed.emit(new_path.leaf());
}

void level_display::save_level() {
  save_nw_level(m_level.get(), m_level_path);
  set_unsaved(false);
}

void level_display::save_level(
    const boost::filesystem::path& path) {
  set_level_path(path);
  save_level();
}

void level_display::update_selection() {
  /*m_selection_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
    m_select_width,
    m_select_height
  );

  Cairo::RefPtr<Cairo::Context> cr =
    Cairo::Context::create(m_selection_surface);

  for (int x = 0; x < selection.get_width(); ++x) {
    for (int y = 0; y < selection.get_height(); ++y) {
      basic_tiles_display::update_tile(cr, selection.get_tile(x, y), x, y);
    }
  }*/
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
  int offset_right  = std::max(0, (sx + sw) - m_level->get_width() );
  int offset_bottom = std::max(0, (sy + sh) - m_level->get_height());

  const int actual_width = sw - offset_left - offset_right;
  const int actual_height = sh - offset_top - offset_bottom;

  // this can happen if a selection is dragged wholly outside the level
  if (actual_width < 0 || actual_height < 0)
    return;

  tile_buf buf; // make a bounds-checked copy of the selection while applying
  buf.resize(actual_width, actual_height);
  tile_buf& tiles = get_tile_buf();
  for (int x = offset_left; x < selection.get_width() - offset_right; ++x) {
    for (int y = offset_top; y < selection.get_height() - offset_bottom; ++y) {
      const int tx = x + sx;
      const int ty = y + sy;

      tile& t = tiles.get_tile(tx, ty);
      buf.get_tile(x - offset_left, y - offset_top) = t;
      t = selection.get_tile(x, y);
    }
  }

  // destroy buf
  add_undo_diff(new tile_diff(sx + offset_left, sy + offset_top, buf, m_active_layer));
}

void level_display::delete_selection() {
  if (npc_selected()) {
    add_undo_diff(new delete_npc_diff(*selected_npc));
    m_level->npcs.erase(selected_npc);
  } else {
    // lift selection if we don't have one yet
    if (selection.empty())
      lift_selection();
  }
  clear_selection();
}

void level_display::clear_selection() {
  selection.clear();
  selected_npc = m_level->npcs.end();

  m_selecting = false;
  m_dragging = false;
  m_select_width = m_select_height = 0;
}

bool level_display::in_selection(int x, int y) {
  return x >= m_select_x && x <= (m_select_x + m_select_width) &&
         y >= m_select_y && y <= (m_select_y + m_select_height);
}

void level_display::set_selection(const Graal::npc& npc) {
  Cairo::RefPtr<Cairo::ImageSurface>& surface =
      m_image_cache.get_image(npc.image);
  m_select_x = npc.x * m_tile_width;
  m_select_y = npc.y * m_tile_height;
  m_select_width = surface->get_width();
  m_select_height = surface->get_height();
}

tile_buf& level_display::get_tile_buf() {
  return m_level->get_tiles(m_active_layer);
}

Cairo::RefPtr<Cairo::Surface> level_display::render_level(
    bool show_selection_border, bool show_selection,
    bool show_npcs, bool show_links, bool show_signs) {
  Cairo::RefPtr<Cairo::Surface> surface =
    Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
      get_width(),
      get_height()
    );


  return surface;
}

void level_display::on_button_motion(GdkEventMotion* event) {
  int x, y;
  get_pointer(x, y);

  const int tile_x = to_tiles_x(x);
  const int tile_y = to_tiles_y(y);

  const int tx = helper::bound_by(tile_x, 0, m_level->get_width() - 1);
  const int ty = helper::bound_by(tile_y, 0, m_level->get_height() - 1);

  if (m_dragging) {
    const int delta_x = (to_tiles_x(x - m_drag_mouse_x) - to_tiles_x(m_select_x)) * m_tile_width;
    const int delta_y = (to_tiles_y(y - m_drag_mouse_y) - to_tiles_y(m_select_y)) * m_tile_height;

    const int new_select_x = m_select_x + delta_x;
    const int new_select_y = m_select_y + delta_y;

    // move npc if selected
    if (npc_selected()) {
      selected_npc->x = new_select_x / m_tile_width;
      selected_npc->y = new_select_y / m_tile_height;
    }

    m_select_x = new_select_x;
    m_select_y = new_select_y;

  } else if (m_selecting) {
    // if you move the mouse when you have a npc selected, switch to dragging
    if (npc_selected()) {
      m_dragging = true;
      m_drag_mouse_x = x - m_select_x;
      m_drag_mouse_y = y - m_select_y;

      m_drag_start_x = selected_npc->x;
      m_drag_start_y = selected_npc->y;
    } else {

      // the start tile needs to be limited to 0..63,
      // but in order to get correct width height these need to
      // go till 64
      m_select_width =
        helper::bound_by(tile_x, 0, m_level->get_width()) * m_tile_width - m_select_x;
      m_select_height =
        helper::bound_by(tile_y, 0, m_level->get_height()) * m_tile_height - m_select_y;

      m_drag_start_x = tx;
      m_drag_start_y = ty;
    }
  }

  std::ostringstream str;
  str
    << "Tile (" << tx << ", " << ty << "): "
    << get_tile_buf().get_tile(tx, ty).index;
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
  if (event->button == 1 || event->button == 3) {
    int x, y;
    get_pointer(x, y);

    const int tile_x = to_tiles_x(x);
    const int tile_y = to_tiles_y(y);

    if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
      if (npc_selected()) {
        edit_npc dialog; // TODO: ???
        dialog.set(*selected_npc);
        if (dialog.run() == Gtk::RESPONSE_OK) {
          Graal::npc new_npc = dialog.get_npc();
          if (new_npc != *selected_npc) {
            add_undo_diff(new npc_diff(*selected_npc));
          }
          (*selected_npc) = new_npc;
        }
        m_dragging = false;
        m_selecting = false;
        
        set_selection(*selected_npc);

        return;
      }

      // set default tile
      if (!has_selection()) {
        const int tile_index = get_tile_buf().get_tile(x/m_tile_width, y/m_tile_height).index;
        m_signal_default_tile_changed.emit(tile_index);
      }
    }


    // abort if we are currently dragging
    if (m_dragging) {
      // or clear the selection if press the right mouse button
      if (event->button == 3 && !npc_selected()) {
        delete_selection();
      }
      return;
    }

    if (!m_preferences.hide_npcs) {
      // check whether we clicked a npc and select it
      level::npc_list_type::iterator iter, end;
      end = m_level->npcs.end();
      for (iter = m_level->npcs.begin(); iter != end; iter ++) {
        Cairo::RefPtr<Cairo::ImageSurface> npc_image =
          m_image_cache.get_image(iter->image);
        const int npc_x = iter->x * m_tile_width;
        const int npc_y = iter->y * m_tile_height;
        const int npc_width = npc_image->get_width();
        const int npc_height = npc_image->get_height();
        if (x >= npc_x && x < npc_x + npc_width
            && y >= npc_y && y < npc_y + npc_height) {
          // NPC clicked; deselect previous selection
          if (has_selection()) {
            save_selection();
            clear_selection();
          }
          selected_npc = iter;
          set_selection(*selected_npc);
        }
      }
    }
    

    // Did we click on a selection?
    if (in_selection(x, y)) {
      if (event->button == 3) {
        // write selection if we right click
        if (npc_selected()) {
          Graal::npc new_npc = *selected_npc;
          add_undo_diff(new create_npc_diff(m_level->add_npc(new_npc).id));
          selected_npc = (--m_level->npcs.end());
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
      if (x > m_level->get_width() * m_tile_width ||
          y > m_level->get_height() * m_tile_height)
        return;

      // otherwise select tiles only with left mouse button
      if (!npc_selected() && event->button == 1) {   
        m_select_x = tile_x * m_tile_width;
        m_select_y = tile_y * m_tile_height;

        m_select_width = 0;
        m_select_height = 0;

        m_selecting = true;
      }
    }
  }
}


void level_display::on_button_released(GdkEventButton* event) {
  if (event->button == 1 || event->button == 3) {
    int x, y;
    get_pointer(x, y);

    // Stop dragging
    if (m_dragging) {
      // if no new npc was dragged, added npc changed diff
      const int tile_x = to_tiles_x(x);
      const int tile_y = to_tiles_y(y);
      if (!m_new_npc && npc_selected() && (m_drag_start_x != tile_x || m_drag_start_y != tile_y)) {
        Graal::npc old_npc = *selected_npc;
        old_npc.x = m_drag_start_x;
        old_npc.y = m_drag_start_y;
        add_undo_diff(new npc_diff(old_npc));
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
  int offset_right  = std::max(0, (sx + sw) - m_level->get_width() );
  int offset_bottom = std::max(0, (sy + sh) - m_level->get_height());

  const int actual_width = sw - offset_left - offset_right;
  const int actual_height = sh - offset_top - offset_bottom;

  // this can happen if a selection is dragged wholly outside the level
  if (actual_width < 0 || actual_height < 0)
    return;

  selection.clear();
  selection.resize(sw, sh);
  tile_buf buf;
  buf.resize(actual_width, actual_height);
  tile_buf& tiles = get_tile_buf();
  for (int x = offset_left; x < selection.get_width() - offset_right; ++x) {
    for (int y = offset_top; y < selection.get_height() - offset_bottom; ++y) {
      const int tx = x + sx;
      const int ty = y + sy;

      tile& t = tiles.get_tile(tx, ty);
      selection.get_tile(x, y)
        = buf.get_tile(x - offset_left, y - offset_top)
        = t;

      // set to default tile
      t.index = m_default_tile_index;
    }
  }

  // destroy buf
  add_undo_diff(new tile_diff(sx + offset_left, sy + offset_right, buf, m_active_layer));

  update_selection();
}

void level_display::grab_selection() {
  lift_selection();
  save_selection();
  for (int i = 0; i < 2; ++i) {
    boost::scoped_ptr<basic_diff> p(undo_buffer.apply(*this));
    p->apply(*this);
  }
  // TODO: this is sooo wasteful, so it should probably be the other way around
}

// tiles
void level_display::drag_selection(tile_buf& tiles,
                                       int drag_x, int drag_y) {
  int x, y;
  get_pointer(x, y);

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
  update_selection();
}

// npc
void level_display::drag_selection(level::npc_list_type::iterator npc_iter) {
  int x, y;
  get_pointer(x, y);

  // save a previous selection
  //if (has_selection()) {
    if (!m_dragging) {
      save_selection();
    }
    clear_selection();
  //}

  m_new_npc = true;
  selected_npc = npc_iter;

  set_selection(*selected_npc);

  const int tx = to_tiles_x(x);
  const int ty = to_tiles_y(y);

  const int tw = m_tile_width;
  const int th = m_tile_height;

  m_drag_mouse_x = 3 * tw;
  m_drag_mouse_y = 3 * th;

  m_select_x = tx * tw - m_drag_mouse_x;
  m_select_y = ty * th - m_drag_mouse_y;
  m_select_height = 3 * tw;
  m_select_width = 3 * th;

  selected_npc->x = to_tiles_x(m_select_x);
  selected_npc->y = to_tiles_y(m_select_y);

  m_dragging = true;

  update_selection();
}

void level_display::undo() {
  if (has_selection()) {
    save_selection();
  }

  if (undo_buffer.empty())
    return;

  redo_buffer.push(undo_buffer.apply(*this));
  set_unsaved(true);
}

void level_display::redo() {
  if (redo_buffer.empty())
    return;

  undo_buffer.push(redo_buffer.apply(*this));
  set_unsaved(true);
}

void level_display::add_undo_diff(basic_diff* diff) {
  undo_buffer.push(diff);
  redo_buffer.clear();

  set_unsaved(true);
}

void level_display::set_unsaved(bool new_unsaved) {
  if (m_unsaved != new_unsaved) {
    m_unsaved = new_unsaved;
    signal_unsaved_status_changed().emit(new_unsaved);
  } else {
    m_unsaved = new_unsaved;
  }
}

bool level_display::get_unsaved() {
  return m_unsaved;
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
  Graal::npc& npc = m_level->add_npc();
  drag_selection(--m_level->npcs.end());
  add_undo_diff(new create_npc_diff(npc.id));
  return npc;
}

void level_display::flood_fill(int tx, int ty, int fill_with_index) {
  static int vec_x[] = {-1, 0, 1, 0};
  static int vec_y[] = { 0, -1, 0, 1};

  // the index of the tiles to fill
  tile& start_tile = get_tile_buf().get_tile(tx, ty);
  int fill_index = start_tile.index;
  if (fill_with_index == fill_index)
    return;
  start_tile.index = fill_with_index;

  std::queue<std::pair<int, int> > queue;
  queue.push(std::pair<int, int>(tx, ty));

  std::list<std::pair<int, int> > changed_tiles;
  int start_x = m_level->get_width();
  int start_y = m_level->get_height();
  int end_x = -1;
  int end_y = -1;
  
  tile_buf& tiles = get_tile_buf();
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

      if (current_tx >= 0 && current_tx < m_level->get_width() &&
          current_ty >= 0 && current_ty < m_level->get_height()) {
        tile& adjacent_tile = tiles.get_tile(current_tx, current_ty);
        if (adjacent_tile.index == fill_index) {
          queue.push(std::pair<int, int>(current_tx, current_ty));
          adjacent_tile.index = fill_with_index;
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
      current_tile = tiles.get_tile(cx, cy);
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
  queue_draw();
}

void level_display::on_mouse_leave(GdkEventCrossing* event) {
  m_signal_status_update("");
}

void level_display::set_layer_visibility(std::size_t layer, bool visible) {
  if (layer >= m_layer_visibility.size())
    m_layer_visibility.resize(layer + 1, false);
  m_layer_visibility[layer] = visible;
}

bool level_display::get_layer_visibility(std::size_t layer) {
  if (layer < m_layer_visibility.size())
    return m_layer_visibility[layer];
  return false;
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

void level_display::draw_tiles() {
// Draw each layer
  int layer_count = m_level->get_layer_count();
  for (int i = 0; i < layer_count; i ++) {
    // If it's visible
    if (m_layer_visibility[i]) {
      // With its own set of tiles
      tile_buf& tiles = m_level->get_tiles(i);
      const int width = tiles.get_width();
      const int height = tiles.get_height();

      // Draw layers below the current darker, above transparent
      if (i > m_active_layer) {
        int level_diff = std::abs(m_active_layer - i);
        glColor4f(1, 1, 1, std::pow(0.5, level_diff));
      } else if (i < m_active_layer) {
        glColor4f(0.5, 0.5, 0.5, 1);
      } else {
        glColor4f(1, 1, 1, 1);
      }

      for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
          draw_tile(tiles.get_tile(x, y), x, y);
        }
      }
    }
  }
}


void level_display::draw_selection() {
  if (!selection.empty()) {
    // Draw at selection position
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(m_select_x, m_select_y, 0);
    glColor4f(1, 1, 1, 1);
    // Draw selection from its tile_buf
    const int width = selection.get_width();
    const int height = selection.get_height();
    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        draw_tile(selection.get_tile(x, y), x, y);
      }
    }
    glPopMatrix();
  }

  // Show selection rectangle when we are selecting or there's a selection and
  // we are not currently dragging except when the relevant preference is set
  bool show_border = !m_dragging || m_preferences.selection_border_while_dragging;
  if (show_border && (m_selecting || m_select_width*m_select_height != 0 || npc_selected())) {
    // reload selection width/height for npcs
    if (npc_selected()) {
      Cairo::RefPtr<Cairo::ImageSurface> img =
          m_image_cache.get_image(selected_npc->image);
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

void level_display::draw_misc() {
  // Signs
  if (!m_preferences.hide_signs) {
    Graal::level::sign_list_type::iterator sign_iter, sign_end = m_level->signs.end();
    for (sign_iter = m_level->signs.begin(); sign_iter != sign_end; sign_iter ++) {
      draw_rectangle(
        sign_iter->x * m_tile_width, sign_iter->y * m_tile_height,
        2 * m_tile_width, 1 * m_tile_height, // Signs are by default 2x1 tiles big
        1, 0, 0, 0.2,
        true);
    }
  }

  // Links
  if (!m_preferences.hide_links) {
    Graal::level::link_list_type::iterator link_iter, link_end = m_level->links.end();
    for (link_iter = m_level->links.begin(); link_iter != link_end; link_iter ++) {
      draw_rectangle(
        link_iter->x * m_tile_width, link_iter->y * m_tile_height,
        link_iter->width * m_tile_width, link_iter->height * m_tile_height,
        1, 1, 0.4, 0.2,
        true);
    }
  }
}

void level_display::draw_all() {
  draw_tiles();
  draw_misc();

  draw_selection();
}
