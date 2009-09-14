#ifndef GRAAL_LEVEL_EDITOR_UNDO_DIFFS_HPP_
#define GRAAL_LEVEL_EDITOR_UNDO_DIFFS_HPP_

#include "level.hpp"

namespace Graal {
  namespace level_editor {
    class level_display;
    class basic_diff {
    public:
      virtual ~basic_diff() {}
      virtual basic_diff* apply(level_display& target) = 0;
    };

    class tile_diff : public basic_diff {
    public:
      tile_diff(int x, int y, tile_buf& tiles);
      virtual basic_diff* apply(Graal::level_editor::level_display& target);

    private:
      int m_x, m_y;
      tile_buf m_tiles;
    };

    class delete_npc_diff : public basic_diff {
    public:
      delete_npc_diff(Graal::npc& npc): m_npc(npc) {}

      virtual basic_diff* apply(Graal::level_editor::level_display& target);
    protected:
      Graal::npc m_npc;
    };

    class create_npc_diff : public basic_diff {
    public:
      create_npc_diff(int id): m_id(id) {}

      virtual basic_diff* apply(Graal::level_editor::level_display& target);
    protected:
      int m_id;
    };

    class npc_diff : public basic_diff {
    public:
      npc_diff(Graal::npc& npc): m_npc(npc) {}

      virtual basic_diff* apply(Graal::level_editor::level_display& target);
    protected:
      Graal::npc m_npc;
    };
  }
}

#endif
