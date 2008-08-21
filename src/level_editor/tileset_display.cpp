#include "_precompiled.hpp"
#include "tileset_display.hpp"
#include "helper.hpp"
#include <sstream>

using namespace Graal;

// scales real positions to tile positions
inline int level_editor::tileset_display::to_tiles_x(int x) {
  return (x + m_tile_width / 2) / m_tile_width;
}

inline int level_editor::tileset_display::to_tiles_y(int y) {
  return (y + m_tile_height / 2) / m_tile_width;
}

level_editor::tileset_display::tileset_display(level_editor::preferences& _prefs,
    level_editor::image_cache& cache)
: m_preferences(_prefs), m_image_cache(cache) {
  add_events(Gdk::BUTTON_PRESS_MASK
             | Gdk::BUTTON_RELEASE_MASK
             | Gdk::BUTTON_MOTION_MASK
             | Gdk::POINTER_MOTION_MASK
             | Gdk::LEAVE_NOTIFY_MASK);

  signal_button_press_event().connect_notify(
    sigc::mem_fun(this, &tileset_display::on_button_pressed));
  signal_button_release_event().connect_notify(
    sigc::mem_fun(this, &tileset_display::on_button_released));

  signal_motion_notify_event().connect_notify(
    sigc::mem_fun(this, &tileset_display::on_button_motion));

  signal_leave_notify_event().connect_notify(
    sigc::mem_fun(this, &tileset_display::on_mouse_leave));

  m_selecting = false;
  m_select_x = m_select_y = m_select_end_x = m_select_end_y = 0;
}

void level_editor::tileset_display::set_tile_size(int tile_width, int tile_height) {
  m_tile_width = tile_width;
  m_tile_height = tile_height;
}

void level_editor::tileset_display::update_tileset(const std::string& level_name) {
  // find the main tileset and draw it first, then the others
  level_editor::tileset_list_type::iterator main_iter;
  level_editor::tileset_list_type::iterator iter, end;
  end = m_preferences.tilesets.end();
  main_iter = m_preferences.tilesets.end();
  for (iter = m_preferences.tilesets.begin();
       iter != end;
       iter ++) {
    // prefix matches level name
    if (level_name.find(iter->prefix) != std::string::npos) {
      if (iter->main) {
        main_iter = iter;
      }
    }
  }
  
  Cairo::RefPtr<Cairo::ImageSurface> main;
  if (main_iter == m_preferences.tilesets.end()) {
    // TODO: throw something more specific
    throw std::runtime_error(
      "No valid tileset found, please add atleast one main tileset to the "
      "tileset list that matches the current level."
    );
  } else {
    //std::cout << "loading tileset " << main_iter->name << std::endl;
    main = m_image_cache.get_image(main_iter->name);
  }
  //std::cout << "creating surface" << std::endl;
  m_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
    main->get_width(),
    main->get_height()
  );

  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface);
  cr->set_source(main, 0, 0);
  cr->paint();

  // now draw the rest
  end = m_preferences.tilesets.end();
  for (iter = m_preferences.tilesets.begin();
       iter != end;
       iter ++) {
    // prefix matches level name
    if (level_name.find(iter->prefix) != std::string::npos && iter != main_iter) {
      Cairo::RefPtr<Cairo::ImageSurface> ts = m_image_cache.get_image(iter->name);
      // Ignore other matching main tilesets
      if (!iter->main) {
        cr->set_source(ts, iter->x, iter->y);
      }
      cr->paint();
    }
  }

  set_size_request(
    main->get_width(),
    main->get_height()
  );

  queue_draw();

  // Fire signal to users
  m_signal_tileset_updated(m_surface);
}

bool level_editor::tileset_display::on_expose_event(GdkEventExpose* event) {
  Glib::RefPtr<Gdk::Window> window = get_window();

  if (!window || !m_surface)
    return true;

  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
  cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
  cr->clip();

  cr->set_source(m_surface, 0, 0);
  cr->paint();

  // Show selection rectangle when we are selecting
  if (m_selecting
      || ((m_select_x != m_select_end_x || m_select_y != m_select_end_y)
          && m_preferences.sticky_tile_selection)) {
    const int x = std::min(m_select_x, m_select_end_x) * m_tile_width;
    const int y = std::min(m_select_y, m_select_end_y) * m_tile_height;

    const int w = (std::abs(m_select_x - m_select_end_x))
                    * m_tile_width;
    const int h = (std::abs(m_select_y - m_select_end_y))
                    * m_tile_height;

    cr->save();
      cr->rectangle(x, y, w, h);
      // TODO: move selection color somewhere else (preferences?)
      cr->set_source_rgb(0.7, 1, 1);
      cr->stroke_preserve();
      cr->set_source_rgba(0.7, 1, 0, 0.2);
      cr->fill();
    cr->restore();
  }

  return true;
}

void level_editor::tileset_display::on_button_pressed(GdkEventButton* event) {
  int x, y;
  get_pointer(x, y);

  if (!m_surface)
    return;

  if (event->button == 1) {
    int tx = x / m_tile_width;
    int ty = y / m_tile_height;
    const int max_x = m_surface->get_width() / m_tile_width;
    const int max_y = m_surface->get_height() / m_tile_height;

    if (tx >= 0 && ty >= 0 &&
        tx < max_x && ty < max_y) {
      if (event->type == GDK_2BUTTON_PRESS) {
        // set default tile
        const int tile_index = helper::get_tile_index(tx, ty);
        m_signal_default_tile_changed.emit(tile_index);
        m_selecting = false;
        return;
      }

      m_selecting = true;

      tx = helper::bound_by(to_tiles_x(x), 0, max_x - 1);
      ty = helper::bound_by(to_tiles_y(y), 0, max_y - 1);
      m_select_end_x = m_select_x = tx;
      m_select_end_y = m_select_y = ty;
    }
  }

  queue_draw();
}

void level_editor::tileset_display::on_button_motion(GdkEventMotion* event) {
  if (!m_surface)
    return;

  int x, y;
  get_pointer(x, y);

  int tx = to_tiles_x(x);
  int ty = to_tiles_y(y);

  // make sure the selection does not extend beyond the bounds of the tileset
  const int max_x = to_tiles_x(m_surface->get_width() - 1);
  const int max_y = to_tiles_y(m_surface->get_height() - 1);

  tx = helper::bound_by(tx, 0, max_x);
  ty = helper::bound_by(ty, 0, max_y);

  std::ostringstream str;
  str
    << "Tile (" << tx << ", " << ty << "): "
    << helper::get_tile_index(tx, ty);
  m_signal_status_update(str.str());

  if (!m_selecting)
    return;

  m_select_end_x = tx;
  m_select_end_y = ty;

  queue_draw();
}

void level_editor::tileset_display::on_button_released(GdkEventButton* event) {
  if (event->button == 1) {
    if (m_selecting) {
      m_selecting = false;

      copy_selection();
    }

    queue_draw();
  }
}

void level_editor::tileset_display::reset_selection() {
  m_select_x = m_select_y = m_select_end_x = m_select_end_y = 0;
  m_selecting = false;

  queue_draw();
}

void level_editor::tileset_display::copy_selection() {
  int x, y;
  get_pointer(x, y);

  int tx = std::min(m_select_x, m_select_end_x);
  int ty = std::min(m_select_y, m_select_end_y);

  int w = std::abs(m_select_x - m_select_end_x);
  int h = std::abs(m_select_y - m_select_end_y);

  int try_drag_x = to_tiles_x(x) - m_select_x;
  int try_drag_y = to_tiles_y(y) - m_select_y;

  // TODO: kinda ugly
  // no rectangle drawn, we want to select a single tile
  if (w == 0 || h == 0) {
    w = 1; h = 1;
    tx = x / m_tile_width;
    ty = y / m_tile_height;
    m_select_x = tx;
    m_select_y = ty;
    m_select_end_x = m_select_x + 1;
    m_select_end_y = m_select_y + 1;
  }
  // TODO: hackish, makes manually selecting one tile more like single tile
  // selection (drag pos in lower corner)
  if (w == 1 && h == 1) {
    try_drag_x = m_tile_width;
    try_drag_y = m_tile_height;
  }

  tile_buf selection;
  // Add tiles to selection
  selection.resize(w, h);
  for (int cx = 0; cx < w; ++cx) {
    for (int cy = 0; cy < h; ++cy) {
      selection.get_tile(cx, cy).index =
        helper::get_tile_index(tx + cx, ty + cy);
    }
  }

  const int drag_x = helper::bound_by(try_drag_x, 0, w); 
  const int drag_y = helper::bound_by(try_drag_y, 0, h);

  m_signal_tiles_selected(selection, drag_x, drag_y);
}

void level_editor::tileset_display::on_mouse_leave(GdkEventCrossing* event) {
  /*if (!m_selecting)
    reset_selection();*/

  m_signal_status_update("");
}

level_editor::tileset_display::signal_status_update_type&
level_editor::tileset_display::signal_status_update() {
  return m_signal_status_update;
}

level_editor::tileset_display::signal_default_tile_changed_type&
level_editor::tileset_display::signal_default_tile_changed() {
  return m_signal_default_tile_changed;
}

level_editor::tileset_display::signal_tileset_updated_type&
level_editor::tileset_display::signal_tileset_updated() {
  return m_signal_tileset_updated;
}

level_editor::tileset_display::signal_tiles_selected_type&
level_editor::tileset_display::signal_tiles_selected() {
  return m_signal_tiles_selected;
}
