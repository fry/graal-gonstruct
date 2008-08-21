#include "_precompiled.hpp"
#include "basic_tiles_display.hpp"
#include "helper.hpp"

using namespace Graal;

level_editor::basic_tiles_display::basic_tiles_display(int tile_width, int tile_height) {
  set_tile_size(tile_width, tile_height);
}

void level_editor::basic_tiles_display::set_tile_size(int tile_width, int tile_height) {
  m_tile_width = tile_width;
  m_tile_height = tile_height;
}

void level_editor::basic_tiles_display::set_tileset_surface(
    const Cairo::RefPtr<Cairo::Surface>& surface) {
  m_tileset_surface = surface;
  update_all();
}

void level_editor::basic_tiles_display::update_all() {
  if (!m_surface)
    return;

  Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(m_surface);
  for (int x = 0; x < get_tile_buf().get_width(); ++x) {
    for (int y = 0; y < get_tile_buf().get_height(); ++y) {
      update_tile(context, x, y);
    }
  }

  queue_draw();
}

void level_editor::basic_tiles_display::update_tiles(int x, int y,
                                                    int width, int height) {
  if (!m_surface)
    return;

  Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(m_surface);

  for (int offset_x = 0; offset_x < width; ++offset_x) {
    for (int offset_y = 0; offset_y < height; ++offset_y) {
      const int tx = x + offset_x;
      const int ty = y + offset_y;
      update_tile(context, tx, ty);
    }
  }
}

void level_editor::basic_tiles_display::update_tile(Cairo::RefPtr<Cairo::Context>& ct, int x, int y) {
  update_tile(ct, get_tile_buf().get_tile(x, y), x, y);
}

void level_editor::basic_tiles_display::update_tile(Cairo::RefPtr<Cairo::Context>& ct,
                                                    const tile& _tile, int x, int y) {
  if (!m_tileset_surface)
    return;

  ct->save();
    ct->translate(x * m_tile_width, y * m_tile_height);
    ct->rectangle(0, 0, m_tile_width, m_tile_height);
    ct->clip();

    int tile_x = -(m_tile_width * helper::get_tile_x(_tile.index));
    int tile_y = -(m_tile_height * helper::get_tile_y(_tile.index));
    ct->set_source(
      m_tileset_surface,
      tile_x,
      tile_y
    );
    ct->paint();

  ct->restore();
}

void level_editor::basic_tiles_display::set_surface_buffers() {
  const tile_buf& buf = get_tile_buf();
  // Create an empty backbuffer surface for the level
  m_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
    buf.get_width() * m_tile_width,
    buf.get_height() * m_tile_height
  );

  // Resize to fit the level
  set_size_request(
    buf.get_width() * m_tile_width,
    buf.get_height() * m_tile_height
  );
}

/*bool level_editor::basic_tiles_display::on_expose_event(GdkEventExpose* event) {
}*/
