#ifndef GRAAL_LEVEL_EDITOR_PREFERENCES_HPP_
#define GRAAL_LEVEL_EDITOR_PREFERENCES_HPP_

#include <map>
#include <core/preferences.h>
#include "tileset.hpp"

#include "level.hpp"

namespace Graal {
  static const char TILEOBJECTS_VERSION[] = "GOBJSET01";
  namespace level_editor {
    typedef std::list<tileset> tileset_list_type;
    struct preferences: public Graal::preferences {
      typedef std::map<std::string, tile_buf> tile_object_group_type;
      typedef std::map<std::string, tile_object_group_type> tile_objects_type;

      preferences();

      tile_objects_type tile_object_groups;
      std::string graal_dir;
      tileset_list_type tilesets;
      int default_tile;
      bool selection_border_while_dragging;
      bool selection_background;
      bool sticky_tile_selection;
      bool hide_npcs, hide_signs, hide_links;
      bool fade_layers;
      bool remember_default_tile;
      bool use_graal_cache;

      tileset add_tileset(const std::string& name, const std::string& prefix);
      tileset add_tileset(const std::string& name, const std::string& prefix, int x, int y, bool main = false);

      void load_tile_objects();
      void save_tile_objects(const std::string& group);

      virtual ~preferences() {}
    protected:
      virtual void serialize();
      virtual void deserialize();
    };
  }
}

#endif
