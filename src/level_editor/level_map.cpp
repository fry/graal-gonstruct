#include "level_map.hpp"
#include "filesystem.hpp"
#include "helper.hpp"
#include "core/helper.h"

#include <fstream>
#include <iostream>

using namespace Graal;
using namespace Graal::level_editor;
using namespace Graal::helper;

std::string level_map_source::get_level_name(int x, int y) {
  // Return an empty name if the index is out of bounds
  if (x >= get_width() || y >= get_height())
    return "";
  
  return m_level_names[x][y];
}

void level_map_source::set_level_name(int x, int y, const std::string& name) {
  if (x >= get_width() || y >= get_height())
    return;
  m_level_names[x][y] = name;
}

int level_map_source::get_width() const {
  return m_level_names.shape()[0];
}

int level_map_source::get_height() const {
  return m_level_names.shape()[1];
}

/* GMap level source */
gmap_level_map_source::gmap_level_map_source(filesystem& _filesystem, const boost::filesystem::path& gmap_file_name):
  m_filesystem(_filesystem),
  m_gmap_file_name(gmap_file_name)
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

  int level_count = 0;
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

        // Protect against malformed level list
        std::cout << line << std::endl;
        g_assert(!file.eof());
        if (line == "LEVELNAMESEND" || file.eof())
          break;
        
        int x = 0;
        std::vector<std::string> level_names(Graal::csv_to_array(line));
        end = level_names.end();
        for (iter = level_names.begin(); iter != end; ++iter) {
          /* Check the path of the GMap for the level for the level,
           * otherwise let the filesystem deal with finding it */
          boost::filesystem::path level_path;
          if (m_filesystem.get_path((m_gmap_file_name.parent_path() / *iter).string(), level_path)) {
            set_level_name(x, y, level_path.string());
          } else {
            set_level_name(x, y, *iter);
          }
          level_count ++;
          x ++;
        }
        y ++;
      }
    }
    // Skip the line if it's unknown
    else {
      read_line(file);
    }
  }

  if (get_width() == 0 || get_height() == 0 || level_count == 0) {
    throw std::runtime_error("No levels present in GMap (Terrain level? No support for that, yet)");
  }
}

Graal::level* gmap_level_map_source::load_level(int x, int y) {
  std::string level_name = get_level_name(x, y);

  if (!level_name.empty()) {
    boost::filesystem::path level_path;
    if (m_filesystem.get_path(level_name, level_path)) {
      return load_nw_level(level_path);
    }
  }
  return 0;
}

void gmap_level_map_source::save_level(int x, int y, level* _level) {
  std::string level_name = get_level_name(x, y);

  if (!level_name.empty()) {
    boost::filesystem::path level_path;
    if (m_filesystem.get_path(level_name, level_path)) {
      save_nw_level(_level, level_path);
    }
  }
}

/* Single level source */
single_level_map_source::single_level_map_source(const boost::filesystem::path& file_name) {
  // Resize level names array to fit the single level
  level_names_list_type::extent_gen extend;
  m_level_names.resize(extend[1][1]);
  m_level_names[0][0] = file_name.string();
}

Graal::level* single_level_map_source::load_level(int x, int y) {
  std::string level_name = get_level_name(x, y);

  if (level_name.empty())
    return 0;

  return load_nw_level(level_name);
}

void single_level_map_source::save_level(int x, int y, level* _level) {
  std::string level_name = get_level_name(x, y);
  if (!level_name.empty()) {
    save_nw_level(_level, level_name);
  }
}

/* level map */
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
  set_size(source->get_width(), source->get_height());
}

const boost::shared_ptr<level_map_source>& level_map::get_level_source() {
  return m_level_source;
}

const boost::shared_ptr<level>& level_map::load_level(const boost::filesystem::path& _file_name, int x, int y) {
  level* new_level = load_nw_level(_file_name);

  set_level(new_level, x, y);

  return get_level(x, y);
}

void level_map::set_level(level* _level, int x, int y) {
  // Increase map size to fit the new level
  if (x <= get_width() || y <= get_width()) {
    set_size(
      std::max(x + 1, get_width()),
      std::max(y + 1, get_height()));
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

const boost::shared_ptr<level>& level_map::get_level(int x, int y) {
  g_assert(x >= 0 && x < get_width() && y >= 0 && y < get_height());
  if (x >= get_width() || y >= get_height())
    throw std::runtime_error("Level position out of bounds");

  boost::shared_ptr<level>& level_ptr = m_level_list[x][y];
  // Load the level if it is not loaded and we have a source to look up into
  if (m_level_source && !level_ptr) {
    level* new_level = m_level_source->load_level(x, y);
    if (new_level) {
      level_ptr.reset(new_level);
    }
  }


  return level_ptr;
}

level_map::level_list_type& level_map::get_levels() {
  return m_level_list;
}

tile& level_map::get_tile_editable(int x, int y, int layer) {
  const int level_width = get_level_width();
  const int level_height = get_level_height();

  // The particular level this tile falls in
  const int level_x = x / level_width;
  const int level_y = y / level_height;

  // The actual tile position inside the level
  const int tile_x = x % level_width;
  const int tile_y = y % level_height;

  level* tile_level = get_level(level_x, level_y).get();
  if (tile_level) {
    // Ensure that the layer exists
    return tile_level->create_tiles(layer).get_tile(tile_x, tile_y);
  }

  throw std::runtime_error("Attempted to edit a tile outside the map");
}

const tile& level_map::get_tile(int x, int y, int layer) {
  return get_tile_editable(x, y, layer);
}

void level_map::set_tile(const tile& tile, int x, int y, int layer) {
  get_tile_editable(x, y, layer) = tile;

  const int level_width = get_level_width();
  const int level_height = get_level_height();

  // The particular level this tile falls in
  const int level_x = x / level_width;
  const int level_y = y / level_height;

  m_signal_level_changed(level_x, level_y);
}

level::npc_list_type& level_map::get_npcs(int x, int y) {
  const int level_width = get_level_width();
  const int level_height = get_level_height();

  // The particular level this tile falls in
  const int level_x = x / level_width;
  const int level_y = y / level_height;

  level* tile_level = get_level(level_x, level_y).get();
  if (tile_level) {
    return tile_level->npcs;
  }

  throw std::runtime_error("Attempted to retrieve NPCs from outside the map");
}

npc* level_map::get_npc(const level_map::npc_ref& ref) {
  level* npc_level = get_level(ref.level_x, ref.level_y).get();
  if (npc_level)
    return &(*npc_level->get_npc(ref.id));
  return 0;
}

void level_map::delete_npc(const level_map::npc_ref& ref) {
  level* npc_level = get_level(ref.level_x, ref.level_y).get();
  if (npc_level)
    npc_level->delete_npc(ref.id);
}

npc* level_map::move_npc(level_map::npc_ref& ref, float new_x, float new_y) {
  const int level_width = get_level_width();
  const int level_height = get_level_height();

  const int new_level_x = static_cast<int>(new_x) / level_width;
  const int new_level_y = static_cast<int>(new_y) / level_height;

  // Determine the position of the NPC inside the level
  const float new_tiles_x = new_x - new_level_x * get_level_width();
  const float new_tiles_y = new_y - new_level_y * get_level_height();

  npc* _npc = get_npc(ref);
  npc* new_npc = _npc;

  // The level changed, take care of moving the NPC into the new level
  if (new_level_x != ref.level_x || new_level_y != ref.level_y) {
    level* old_level = get_level(ref.level_x, ref.level_y).get();
    level* new_level = get_level(new_level_x, new_level_y).get();

    // Add a copy of the old NPC
    new_npc = &new_level->add_npc(*_npc);
    old_level->delete_npc(_npc->id);

    // Fix reference
    ref.id = new_npc->id;
    ref.level_x = new_level_x;
    ref.level_y = new_level_y;

    m_signal_level_changed(ref.level_x, ref.level_y);
  }
  
  m_signal_level_changed(new_level_x, new_level_y);

  // Set the correct position inside the level
  new_npc->set_level_x(new_tiles_x);
  new_npc->set_level_y(new_tiles_y);

  
  return new_npc;
}

void level_map::get_global_npc_position(const level_map::npc_ref& ref, float& x, float& y) {
  npc* _npc = get_npc(ref);
  x = ref.level_x * get_level_width() + _npc->get_level_x();
  y = ref.level_y * get_level_height() + _npc->get_level_y();
}

int level_map::get_width() const {
  return m_level_list.shape()[0];
}

int level_map::get_height() const {
  return m_level_list.shape()[1];
}

int level_map::get_width_tiles() const {
  return get_width() * get_level_width();
}

int level_map::get_height_tiles() const {
  return get_height() * get_level_height();
}

void level_map::set_size(int width, int height) {
  level_list_type::extent_gen extents;
  m_level_list.resize(extents[width][height]);
}

int level_map::get_level_width() const {
  return m_level_width;
}

int level_map::get_level_height() const {
  return m_level_height;
}

void level_map::set_level_size(int width, int height) {
  m_level_width = width;
  m_level_height = height;
}

level_map* level_map::load_from_gmap(filesystem& _filesystem, const boost::filesystem::path& _file_name) {
  boost::shared_ptr<gmap_level_map_source> source(new gmap_level_map_source(_filesystem, _file_name));
  level_map* map(new level_map());

  std::cout << "load_from_gmap(): " << source->get_width() << "," << source->get_height() << std::endl;
  map->set_level_source(source);
  map->set_size(source->get_width(), source->get_height());

  return map;
}

level_map::signal_level_changed_type& level_map::signal_level_changed() {
  return m_signal_level_changed;
}
