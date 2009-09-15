#include "undo_diffs.hpp"
#include "level_display.hpp"

using namespace Graal;

level_editor::tile_diff::tile_diff(int x, int y, tile_buf& tiles, int layer)
    : m_x(x), m_y(y), m_layer(layer) {
  // if (tiles.get_width() == 0 || tiles.get_height() == 0)
  //   throw std::logic_error("trying to construct zero-size tile_diff");
  m_tiles.swap(tiles);
}

level_editor::basic_diff* level_editor::tile_diff::apply(
    Graal::level_editor::level_display& target) {
  tile_buf& level_tiles = target.get_level()->get_tiles(m_layer);
  tile_buf buf;
  buf.resize(m_tiles.get_width(), m_tiles.get_height());
  for (int x = 0; x < m_tiles.get_width(); ++x) {
    for (int y = 0; y < m_tiles.get_height(); ++y) {
      const int tx = m_x + x;
      const int ty = m_y + y;

      tile& t = level_tiles.get_tile(tx, ty);
      buf.get_tile(x, y) = t;
      t = m_tiles.get_tile(x, y);
    }
  }
  target.update_tiles(m_x, m_y, m_tiles.get_width(), m_tiles.get_height());

  return new tile_diff(m_x, m_y, buf, m_layer);
}

level_editor::basic_diff* level_editor::delete_npc_diff::apply(Graal::level_editor::level_display& target) {
  target.get_level()->npcs.push_back(m_npc);
  return new create_npc_diff(m_npc.id);
}

level_editor::basic_diff* level_editor::create_npc_diff::apply(Graal::level_editor::level_display& target) {
  Graal::level& level = *target.get_level();
  Graal::npc npc = *level.get_npc(m_id);
  level.delete_npc(m_id);
  return new delete_npc_diff(npc);
}

level_editor::basic_diff* level_editor::npc_diff::apply(Graal::level_editor::level_display& target) {
  Graal::npc& npc = *(target.get_level()->get_npc(m_npc.id));
  Graal::npc old_npc = npc;

  npc = m_npc;
  return new npc_diff(old_npc);
}
