#ifndef GRAAL_LEVEL_EDITOR_UNDO_DIFFS_HPP_
#define GRAAL_LEVEL_EDITOR_UNDO_DIFFS_HPP_

#include "level.hpp"
#include "level_map.hpp"

namespace Graal {
  namespace level_editor {
    class basic_diff {
    public:
      virtual ~basic_diff() {}
      virtual basic_diff* apply(level_map& target) = 0;
    };

    class tile_diff : public basic_diff {
    public:
      tile_diff(int x, int y, tile_buf& tiles, int layer);
      virtual basic_diff* apply(level_map& target);

    private:
      int m_layer;
      int m_x, m_y;
      tile_buf m_tiles;
    };

    /* delete_npc_diff:
     * Undos the deletion of a NPC and requires a npc_ref to know about the
     * former location of the NPC and the NPC to store a copy of */
    class delete_npc_diff : public basic_diff {
    public:
      delete_npc_diff(const level_map::npc_ref& ref, const Graal::npc& npc);

      virtual basic_diff* apply(level_map& target);
    protected:
      level_map::npc_ref m_ref;
      Graal::npc m_npc;
    };

    /* create_npc_diff:
     * Undos the creation of a NPC and requires a reference to the NPC to do that */
    class create_npc_diff : public basic_diff {
    public:
      create_npc_diff(const level_map::npc_ref& ref);

      virtual basic_diff* apply(level_map& target);
    protected:
      level_map::npc_ref m_ref;
    };

    /* npc_diff:
     * Undos changes done to an NPC inside a level, requires a reference and the
     * old NPC to do that */
    class npc_diff : public basic_diff {
    public:
      npc_diff(const level_map::npc_ref& ref, Graal::npc& npc);

      virtual basic_diff* apply(level_map& target);
    protected:
      level_map::npc_ref m_ref;
      Graal::npc m_npc;
    };

    /* move_npc_diff:
     * Undos the movement of a NPC (possibly across levels), requires the
     * reference and old global position for that */
    class move_npc_diff: public basic_diff {
    public:
      move_npc_diff(const  level_map::npc_ref& ref, float old_x, float old_y);

      virtual basic_diff* apply(level_map& target);
    protected:
      level_map::npc_ref m_ref;
      float m_old_x, m_old_y;
    };
  }
}

#endif
