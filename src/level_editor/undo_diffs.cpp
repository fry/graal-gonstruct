#include "undo_diffs.hpp"
#include "level_map.hpp"

using namespace Graal;

level_editor::tile_diff::tile_diff(int x, int y, tile_buf& tiles, int layer)
    : m_x(x), m_y(y), m_layer(layer) {
  // if (tiles.get_width() == 0 || tiles.get_height() == 0)
  //   throw std::logic_error("trying to construct zero-size tile_diff");
  m_tiles.swap(tiles);
}

level_editor::basic_diff* level_editor::tile_diff::apply(
    level_editor::level_map& target) {
  tile_buf buf;
  buf.resize(m_tiles.get_width(), m_tiles.get_height());
  for (int x = 0; x < m_tiles.get_width(); ++x) {
    for (int y = 0; y < m_tiles.get_height(); ++y) {
      const int tx = m_x + x;
      const int ty = m_y + y;

      tile& t = target.get_tile(tx, ty);
      buf.get_tile(x, y) = t;
      t = m_tiles.get_tile(x, y);
    }
  }

  return new tile_diff(m_x, m_y, buf, m_layer);
}

level_editor::delete_npc_diff::delete_npc_diff(const level_map::npc_ref& ref, const Graal::npc& npc):
  m_ref(ref),
  m_npc(npc)
{
}

level_editor::basic_diff* level_editor::delete_npc_diff::apply(level_editor::level_map& target) {
  target.get_level(m_ref.level_x, m_ref.level_y)->npcs.push_back(m_npc);
  return new create_npc_diff(m_ref);
}

level_editor::create_npc_diff::create_npc_diff(const level_map::npc_ref& ref):
  m_ref(ref)
{
}

level_editor::basic_diff* level_editor::create_npc_diff::apply(level_editor::level_map& target) {
  Graal::npc npc = *target.get_npc(m_ref);
  target.delete_npc(m_ref);
  return new delete_npc_diff(m_ref, npc);
}

level_editor::npc_diff::npc_diff(const level_map::npc_ref& ref, Graal::npc& npc):
  m_ref(ref),
  m_npc(npc)
{
}

level_editor::basic_diff* level_editor::npc_diff::apply(level_editor::level_map& target) {
  Graal::npc& npc = *target.get_npc(m_ref);
  Graal::npc old_npc = npc;

  npc = m_npc;
  return new npc_diff(m_ref, old_npc);
}

level_editor::move_npc_diff::move_npc_diff(const level_map::npc_ref& ref, float old_x, float old_y):
  m_ref(ref),
  m_old_x(old_x),
  m_old_y(old_y)
{
}

level_editor::basic_diff* level_editor::move_npc_diff::apply(level_editor::level_map& target) {
  float new_old_x, new_old_y;
  target.get_global_npc_position(m_ref, new_old_x, new_old_y);
  target.move_npc(m_ref, m_old_x, m_old_y);

  return new move_npc_diff(m_ref, new_old_x, new_old_y);
}
