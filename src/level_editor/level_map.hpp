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
  level_map_source(filesystem& _filesystem);

  /* Returns the name of the level at the passed positon, returning an empty
   * string if no level is found */
  std::string get_level_name(int x, int y);
  /* Reads the level name at the passed position and loads it */
  level* load_level(int x, int y);

  int get_width();
  int get_height();
protected:
  level_names_list_type m_level_names;
  filesystem& m_filesystem;
};

/* A map source retrieving its level names from a GMap */
class gmap_level_map_source: public level_map_source {
public:
  gmap_level_map_source(filesystem& _filesystem, const boost::filesystem::path& gmap_file_name);
};

/* Contains multiple levels and provides helpful functions for accessing them.
 * Dynamically loads requested levels from an input level name list
 */
class level_map: boost::noncopyable {
public:
  typedef boost::multi_array<boost::shared_ptr<level>, 2> level_list_type;
  static level_map* load_from_gmap(filesystem& _filesystem, const boost::filesystem::path& _file_name);

  level_map();

  // Sets a level source to use in case get_level can't find a level
  void set_level_source(const boost::shared_ptr<level_map_source>& source);
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
  tile& get_tile(int x, int y, int layer = 0);

  // get/set the size of the map in levels
  int get_width();
  int get_height();
  void set_size(int width, int height);
  // get the size of the map in tiles
  int get_width_tiles();
  int get_height_tiles();

  // get/set the size of an individual level
  int get_level_width();
  int get_level_height();
  void set_level_size(int width, int height);
protected:
  // Size of one level in tiles
  int m_level_width, m_level_height;

  // Contains the loaded levels
  level_list_type m_level_list;

  boost::shared_ptr<level_map_source> m_level_source;
};

}

}
