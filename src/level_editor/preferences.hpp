#ifndef GRAAL_LEVEL_EDITOR_PREFERENCES_HPP_
#define GRAAL_LEVEL_EDITOR_PREFERENCES_HPP_

#include <map>
#include <core/preferences.h>
#include "tileset.hpp"

#include "level.hpp"

namespace Graal {
  static const std::string TILEOBJECTS_VERSION = "GOBJSET01";
  namespace level_editor {
    typedef std::list<tileset> tileset_list_type;
    struct preferences: public Graal::preferences {
      typedef std::map<std::string, tile_buf> tile_object_group_type;
      typedef std::map<std::string, tile_object_group_type> tile_objects_type;

      std::string graal_dir;
      tileset_list_type tilesets;
      bool selection_border_while_dragging;
      bool sticky_tile_selection;
      bool hide_npcs, hide_signs, hide_links;
      int default_tile;

      tile_objects_type tile_object_groups;

      tileset add_tileset(const std::string& name, const std::string& prefix) {
        return add_tileset(name, prefix, 0, 0, true);
      }

      tileset add_tileset(const std::string& name, const std::string& prefix, int x, int y, bool main = false) {
        tileset_list_type::iterator iter, end;
        end = tilesets.end();
        for (iter = tilesets.begin(); iter != end; iter++) {
          if (iter->name == name &&
              iter->prefix == prefix &&
              iter->x == x &&
              iter->y == y &&
              iter->main == main) {
            return *iter;
          }
        }

        tileset new_tileset;
        new_tileset.name = name;
        new_tileset.prefix = prefix;
        new_tileset.x = x;
        new_tileset.y = y;
        new_tileset.main = main;
        tilesets.push_back(new_tileset);
        return tilesets.back();
      }

      void load_tile_objects();
      void save_tile_objects();
    protected:
      virtual void serialize();
      virtual void deserialize();
    };
  }
}

#endif
