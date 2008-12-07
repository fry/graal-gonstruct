#ifndef GRAAL_LEVEL_EDITOR_COPY_CACHE_HPP_
#define GRAAL_LEVEL_EDITOR_COPY_CACHE_HPP_

#include "level.hpp"

namespace Graal {
  namespace level_editor {
    class level_display;
    class basic_cache {
    public:
      virtual ~basic_cache() {}
      virtual void paste(level_display& target) = 0;
    };

    class tiles_cache : public basic_cache {
    public:
      tiles_cache(Graal::tile_buf& tile_buf);
      virtual void paste(level_display& target);
    protected:
      tile_buf m_tile_buf;
    };

   class npc_cache : public basic_cache {
    public:
      npc_cache(const Graal::npc& npc);
      virtual void paste(level_display& target);
    protected:
      Graal::npc m_npc;
    };
  }
}

#endif
