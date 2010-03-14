#include "copy_cache.hpp"
#include "level_display.hpp"

using namespace Graal;

level_editor::tiles_cache::tiles_cache(Graal::tile_buf& tile_buf) {
  m_tile_buf.swap(tile_buf);
}

void level_editor::tiles_cache::paste(level_display& target) {
  int x, y;
  target.get_pointer(x, y);

  Graal::tile_buf buf(m_tile_buf);
  target.drag_selection(buf, 0, 0);
}

level_editor::npc_cache::npc_cache(const Graal::npc& npc): m_npc(npc) {}

void level_editor::npc_cache::paste(level_display& target) {
  int x, y;
  target.get_pointer(x, y);

  Graal::npc npc(m_npc);
  target.get_level()->add_npc(npc);
  //target.drag_selection(--target.get_level()->npcs.end()); // TODO
}
