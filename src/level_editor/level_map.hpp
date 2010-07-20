#pragma once

#include "level.hpp"

#include <boost/multi_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

namespace Graal {

namespace level_editor {

class filesystem;

/* TODO: Needs cleaning up, determine what needs to be in the base class,
 * and what should go into derived classes. Also think about whether the
 * getters for the source's size should be in the base class */
/* Contains the names of levels present in a 2D level map */
class level_map_source {
public:
  typedef boost::multi_array<std::string, 2> level_names_list_type;

  /* Returns the name of the level at the passed positon, returning an empty
   * string if no level is found */
  std::string get_level_name(int x, int y);
  /* Set the name of the level at the specified position */
  void set_level_name(int x, int y, const std::string& name);
  /* Reads the level name at the passed position and loads it */
  virtual level* load_level(int x, int y) = 0;
  /* Saves the level at the specified position */
  virtual void save_level(int x, int y, level* _level) = 0;

  int get_width() const;
  int get_height() const;
protected:
  level_names_list_type m_level_names;
};

/* A map source representing a single level */
class single_level_map_source: public level_map_source {
public:
  single_level_map_source(const boost::filesystem::path& file_name);

  virtual level* load_level(int x, int y);
  virtual void save_level(int x, int y, level* _level);
};

/* A map source retrieving its level names from a GMap */
class gmap_level_map_source: public level_map_source {
public:
  gmap_level_map_source(filesystem& _filesystem, const boost::filesystem::path& gmap_file_name);

  virtual level* load_level(int x, int y);
  virtual void save_level(int x, int y, level* _level);
protected:
  filesystem& m_filesystem;
  boost::filesystem::path m_gmap_file_name;
};

/* Contains multiple levels and provides helpful functions for accessing them.
 * Dynamically loads requested levels from an input level name list
 */
class level_map: boost::noncopyable {
public:
  struct npc_ref {
    int level_x, level_y;
    int id;

    npc_ref(): id(0) {}
    bool operator==(const npc_ref& o) { return level_x == o.level_x && level_y == o.level_y && id == o.id; }
  };

  typedef boost::multi_array<boost::shared_ptr<level>, 2> level_list_type;
  static level_map* load_from_gmap(filesystem& _filesystem, const boost::filesystem::path& _file_name);

  level_map();

  // gets/sets a level source to use in case get_level can't find a level
  void set_level_source(const boost::shared_ptr<level_map_source>& source);
  const boost::shared_ptr<level_map_source>& get_level_source();
  // Loads a level and calls set_level on it
  const boost::shared_ptr<level>& load_level(const boost::filesystem::path& _file_name, int x = 0, int y = 0);
  // Places a level into the specified slot
  void set_level(level* _level, int x = 0, int y = 0);
  // Returns the level at the specified location
  const boost::shared_ptr<level>& get_level(int x, int y);
  // Returns all levels
  /* TODO: is this actually needed? It'd be possibly to iterate over all
   * levels easily that way, but we'd lack the level's position, making it
   * rather useless. */
  level_list_type& get_levels();

  /* Loads the level at the specified GLOBAL position if it is not loaded
   * already and returns the tile from inside that level */
  const tile& get_tile(int x, int y, int layer = 0);
  void set_tile(const tile& tile, int x, int y, int layer = 0);
  bool is_valid_tile(int x, int y);

  /* Return the list of NPCs from the level at the specified tile position.
   * Loads the level if it is not loaded already */
  level::npc_list_type& get_npcs(int x, int y);
  /* Properly updates the position and level of an NPC by translating the
   * NPCs GLOBAL position to the correct level. Returns the (possibly) new
   * NPC */
  npc* update_npc(npc* _npc);

  npc* get_npc(const npc_ref& ref);
  void delete_npc(const npc_ref& ref);
  // in global coords, might update ref if level changes
  npc* move_npc(npc_ref& ref, float new_x, float new_y);
  // Return the global position of the references NPC
  void get_global_npc_position(const npc_ref& ref, float& x, float& y);

  // get/set the size of the map in levels
  int get_width() const;
  int get_height() const;
  void set_size(int width, int height);
  // get the size of the map in tiles
  int get_width_tiles() const;
  int get_height_tiles() const;

  // get/set the size of an individual level
  int get_level_width() const;
  int get_level_height() const;
  void set_level_size(int width, int height);

  // Signal to notify users if a specific level was changed
  typedef sigc::signal<void, int, int> signal_level_changed_type;
  signal_level_changed_type& signal_level_changed();
protected:
  signal_level_changed_type m_signal_level_changed;

  // Keep this protected so we can signal on level changes
  tile& get_tile_editable(int x, int y, int layer = 0);

  // Size of one level in tiles
  int m_level_width, m_level_height;

  // Contains the loaded levels
  level_list_type m_level_list;

  boost::shared_ptr<level_map_source> m_level_source;
};

}

}
