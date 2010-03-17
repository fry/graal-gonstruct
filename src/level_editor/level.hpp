#ifndef GRAAL_LEVEL_HPP_
#define GRAAL_LEVEL_HPP_

#include "tileset.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <deque>
#include <list>
#include <string>

namespace Graal {
  static const std::string NW_LEVEL_VERSION = "GLEVNW01";
  class tile {
  public:
    static const int transparent_index = -1;

    int index;

    tile(): index(0) {}
    explicit tile(int index_): index(index_) {}
    bool transparent() const { return index == transparent_index; }
  };

  class tile_buf {
  public:
    typedef std::vector<tile> tiles_list_type;

    tile_buf() : width(0), height(0) {}

    int get_width() const { return width; }
    int get_height() const { return height; }

          tile& get_tile(int x, int y)       { return tiles[x + y * width]; }
    const tile& get_tile(int x, int y) const { return tiles[x + y * width]; }

    void swap(tile_buf& other) {
      tiles.swap(other.tiles);
      std::swap(width, other.width);
      std::swap(height, other.height);
    }

    void resize(int w, int h) {
      tiles.resize(w*h);
      width = w;
      height = h;
    }

    void clear() {
      tiles.clear();
      height = width = 0;
    }

    bool empty() {
      return tiles.empty();
    }

    tiles_list_type tiles;
  private:
    int width;
    int height;

  };

  class link {
  public:
    int x, y;
    int width, height;
    // can contain playerx, playery for example
    std::string new_x, new_y;

    std::string destination;

    link(): x(0), y(0), width(0), height(0) {}
  };

  class sign {
  public:
    int x, y;
    std::string text;
  };

  class npc {
  public:
    int id;
    std::string image;
    std::string script;

    bool operator==(const Graal::npc& o) const {
      return id == o.id && x == o.x && y == o.y && image == o.image && script == o.script;
    }
    
    bool operator!=(const Graal::npc& o) const {
      return !operator==(o);
    }

    // These accessors return the correct position in floats
    float get_level_x() const {
      return static_cast<float>(x) / 2;
    }

    float get_level_y() const {
      return static_cast<float>(y) / 2;
    }

    // Round down to 0.5
    void set_level_x(float _x) {
      x = static_cast<int>(_x * 2);
    }

    void set_level_y(float _y) {
      y = static_cast<int>(_y * 2);
    }

    // Stored as position * 2 to allow for an accuracy of 0.5
    int x, y;
  };

  class level {
  public:
    level(int fill_tile = 0);
    typedef std::list<link> link_list_type;
    typedef std::list<sign> sign_list_type;
    typedef std::list<npc> npc_list_type;
    typedef std::vector<tile_buf> layers_list_type;

    int get_width() const;
    int get_height() const;

    Graal::npc& add_npc();
    Graal::npc& add_npc(Graal::npc npc);
    level::npc_list_type::iterator get_npc(int id);
    void delete_npc(int id);

    tile_buf& create_tiles(int layer = 0, int fill_tile = tile::transparent_index, bool overwrite = false);
    tile_buf& get_tiles(int layer = 0);
    const tile_buf& get_tiles(int layer = 0) const;
    bool tiles_exist(int layer = 0);

    void insert_layer(int index, int fill_tile = tile::transparent_index);
    void delete_layer(int index);

    int get_layer_count() const;

    layers_list_type layers;
    link_list_type links;
    sign_list_type signs;
    npc_list_type npcs;
  protected:
    int m_unique_npc_id_counter;
    int m_fill_tile;
  };

  level* load_nw_level(const boost::filesystem::path& path);
  void save_nw_level(const level* _level, const boost::filesystem::path& path);
}

#endif

