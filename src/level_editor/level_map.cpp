#include "level_map.hpp"
#include "filesystem.hpp"
#include "helper.hpp"
#include "core/helper.h"

#include <fstream>

using namespace Graal;
using namespace Graal::level_editor;
using namespace Graal::helper;

level_map_source::level_map_source(filesystem& _filesystem):
  m_filesystem(_filesystem) {
}

std::string level_map_source::get_level_name(int x, int y) {
  // Return an empty name if the index is out of bounds
  if (m_level_names.shape()[0] >= x || m_level_names.shape()[1] >= y)
    return "";
  return m_level_names[x][y];
}

Graal::level* level_map_source::load_level(int x, int y) {
  std::string level_name = get_level_name(x, y);
  if (!level_name.empty()) {
    boost::filesystem::path level_path;
    if (m_filesystem.get_path(level_name, level_path)) {
      return load_nw_level(level_path);
    }
  }
  return 0;
}

int level_map_source::get_width() {
  return m_level_names.shape()[0];
}

int level_map_source::get_height() {
  return m_level_names.shape()[1];
}

gmap_level_map_source::gmap_level_map_source(filesystem& _filesystem, const boost::filesystem::path& gmap_file_name):
  level_map_source(_filesystem)
{
  static const std::string GMAP_VERSION = "GRMAP001";

  std::ifstream file(gmap_file_name.string().c_str());

  if (!file.good()) {
    throw std::runtime_error("Loading GMAP failed: Could not open file " + gmap_file_name.string());
  }

  std::string version = read_line(file);

  if (version.find(GMAP_VERSION) != 0) {
    throw std::runtime_error("Loading GMAP failed: Version mismatch (" + version + " != " + GMAP_VERSION + ")");
  }

  level_names_list_type::extent_gen extend;
  while (!file.eof()) {
    std::string type = read<std::string>(file);

    if (type == "WIDTH") {
      int new_width = read<int>(file);
      int old_height = m_level_names.shape()[1];
      m_level_names.resize(extend[new_width][old_height]);
    } else if (type == "HEIGHT") {
      int new_height = read<int>(file);
      int old_width = m_level_names.shape()[0];
      m_level_names.resize(extend[old_width][new_height]);
    } else if (type == "LEVELNAMES") {
      // Skip the rest of this line
      read_line(file);

      int y = 0;
      std::vector<std::string>::iterator iter, end;
      // Read lines of levels
      while (!file.eof()) {
        std::string line = read_line(file);
        if (line == "LEVELNAMESEND")
          break;
        
        int x = 0;
        std::vector<std::string> level_names(Graal::csv_to_array(line));
        end = level_names.end();
        for (iter = level_names.begin(); iter != end; ++iter) {
          m_level_names[x][y] = *iter;
          x ++;
        }
        y ++;
      }
    }
  }
}

level_map::level_map():
  m_level_width(0),
  m_level_height(0)
{
  /* TODO: not sure where else to set this, it's Graal specific and not
   * mentioned in any level file or GMap. For what it's worth, levels could
   * be of arbitrary size, but that'd make things a lot harder */
  set_level_size(64, 64); 
}

void level_map::set_level_source(const boost::shared_ptr<level_map_source>& source) {
  m_level_source = source;
}

level* level_map::load_level(const boost::filesystem::path& _file_name, int x, int y) {
  level* new_level = load_nw_level(_file_name);

  if (new_level == 0)
    return 0;

  set_level(new_level, x, y);
}

void level_map::set_level(level* _level, int x, int y) {
  // Increase map size to fit the new level
  if (x <= get_width() || y <= get_width()) {
    set_size(
      std::max(x, get_width()),
      std::max(y, get_height()));
  }

  // Set level size to biggest level
  // TODO: uh, should probably throw an error if it gets set more than once?
  /*if (_level->get_width() > get_level_width() ||
      _level->get_height() > get_level_height()) {
    set_level_size(
      std::max(_level->get_width(), get_level_width()),
      std::max(_level->get_height(), get_level_height()));
  }*/

  /* Store and take ownership of the passed level, overwriting any possibly
   * already loaded levels */
  m_level_list[x][y].reset(_level);
}

level* level_map::get_level(int x, int y) {
  boost::shared_ptr<level>& level_ptr = m_level_list[x][y];
  // Load the level if it is not loaded and we have a source to look up into
  if (!level_ptr && m_level_source) {
    level* new_level = m_level_source->load_level(x, y);
    if (new_level) {
      level_ptr.reset(new_level);
    }
  } else {
    return 0;
  }

  return level_ptr.get();
}

level_map::level_list_type& level_map::get_levels() {
  return m_level_list;
}

tile& level_map::get_tile(int x, int y, int layer) {
  const int level_width = get_level_width();
  const int level_height = get_level_height();

  // The particular level this tile falls in
  const int level_x = x / level_width;
  const int level_y = y / level_height;

  // The actual tile position inside the level
  const int tile_x = x % level_width;
  const int tile_y = x % level_height;

  level* tile_level = get_level(level_x, level_y);
  if (tile_level) {
    // Ensure that the layer exists
    return tile_level->create_tiles(layer).get_tile(tile_x, tile_y);
  }

  throw new std::runtime_error("Attempted to edit a tile outside the map");
}

int level_map::get_width() {
  return m_level_list.shape()[0];
}

int level_map::get_height() {
  return m_level_list.shape()[1];
}

void level_map::set_size(int width, int height) {
  level_list_type::extent_gen extents;
  m_level_list.resize(extents[width][height]);
}

int level_map::get_level_width() {
  return m_level_width;
}

int level_map::get_level_height() {
  return m_level_height;
}

void level_map::set_level_size(int width, int height) {
  m_level_width = width;
  m_level_height = height;
}

boost::shared_ptr<level_map> level_map::load_from_gmap(filesystem& _filesystem, const boost::filesystem::path& _file_name) {
  boost::shared_ptr<gmap_level_map_source> source(new gmap_level_map_source(_filesystem, _file_name));
  boost::shared_ptr<level_map> map(new level_map());

  map->set_level_source(source);
  map->set_size(source->get_width(), source->get_height());

  return map;
}
