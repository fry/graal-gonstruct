#include "preferences.hpp"
#include "helper.hpp"
#include <fstream>
#include <sstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <core/helper.h>

using namespace Graal::level_editor;

namespace {
  template <typename T>
  inline T read(std::istream& stream) {
    T v;
    stream >> v;
    return v;
  }

  inline std::string read_line(std::istream& stream) {
    std::string line;
    std::getline(stream, line);
    return line;
  }

  void try_load_objects(const boost::filesystem::path& path,
                        preferences::tile_object_group_type group) {
    //std::cout << "Group name: " << group_name << std::endl;

    std::ifstream stream(path.string().c_str());

    std::string version = ::read_line(stream);
    //std::cout << "Version: " << version << std::endl;
    if (version.find(Graal::TILEOBJECTS_VERSION) != 0) {
      std::ostringstream ss;
      ss << "Couldn't load tile objects, version mismatch ("
         << Graal::TILEOBJECTS_VERSION
         <<" != " + version + ")";
      throw std::runtime_error(ss.str());
    }

    while (!stream.eof()) {
      std::string type = read<std::string>(stream);

      if (type == "OBJECT") {
        int width, height; std::string name;
        width = read<int>(stream); height = read<int>(stream);
        name = Graal::helper::strip(read_line(stream));

        //std::cout << "OBJECT: " << name << ", " << width << "/" << height <<std::endl;
        Graal::tile_buf& tiles = group[name];
        tiles.resize(width, height);
        //bool end = false;
        for (int y = 0; y < height; ++y) {
          std::string line = read_line(stream);
          if (line == "OBJECTEND") {
            // skip transparency check if the object ends
            //end = true;
            break;
          } else {
            for (int x = 0; x < width; x++) {
              int tile_index = static_cast<int>(Graal::helper::parse_base64(line.substr(x * 2, 2)));
              tiles.get_tile(x, y).index = tile_index;
            }
          }
        }
      } else {
        read_line(stream);
      }
    }
  }
}

preferences::preferences():
  use_graal_cache(false)
{
}

void preferences::serialize() {
  std::ostringstream str;
  
  m_values["graal_directory"] = graal_dir;

  m_values["selection_border_while_dragging"]
    = selection_border_while_dragging ? "true" : "false";

  m_values["selection_background"]
    = selection_background ? "true" : "false";

  m_values["sticky_tile_selection"]
    = sticky_tile_selection ? "true" : "false";

  m_values["fade_layers"]
    = fade_layers ? "true" : "false";
  
  m_values["use_graal_cache"]
    = use_graal_cache ? "true" : "false";

  if (default_tile == -1) { // TODO: see window.cpp TODO re this
    m_values.erase("default_tile");
  } else {
    m_values["default_tile"] = boost::lexical_cast<std::string>(default_tile);
  }

  // write tilesets
  tileset_list_type::iterator iter, end;
  end = tilesets.end();

  int i = 0;
  for (iter = tilesets.begin(); iter != end; iter ++) {
    str.str("");
    std::string key;
    str << "tileset_" << i;
    key = str.str();
    str.str("");
    str << iter->name << ", " << iter->prefix << ", " << iter->x << ", "
        << iter->y << ", " << iter->main;
    m_values[key] = str.str();
    i ++;
  }
}

void preferences::deserialize() {
  value_map_type::iterator iter, end = m_values.end();

  iter = m_values.find("graal_directory");
  if (iter != m_values.end())
    graal_dir = iter->second;

  iter = m_values.find("selection_border_while_dragging");
  if (iter != m_values.end()) {
    selection_border_while_dragging = (iter->second == "true");
  }

  iter = m_values.find("selection_background");
  if (iter != m_values.end()) {
    selection_background = (iter->second == "true");
  }

  iter = m_values.find("sticky_tile_selection");
  if (iter != m_values.end()) {
    sticky_tile_selection = (iter->second == "true");
  }

  iter = m_values.find("fade_layers");
  if (iter != m_values.end()) {
    fade_layers = (iter->second == "true");
  }
 
  iter = m_values.find("use_graal_cache");
  if (iter != m_values.end()) {
    use_graal_cache = (iter->second == "true");
  }

  hide_npcs = false;
  hide_signs = false;
  hide_links = false;

  default_tile = -1; // TODO: see window.cpp TODO re. this
  iter = m_values.find("default_tile");
  if (iter != m_values.end()) {
    std::istringstream ss(iter->second);
    ss >> default_tile;
  }

  for (iter = m_values.begin(); iter != end; iter ++) {
    if (iter->first.find("tileset_") == 0) {
      std::vector<std::string> tokens(Graal::csv_to_array(iter->second));
      tileset new_tileset;
      new_tileset.name = helper::strip(tokens[0]);
      new_tileset.prefix = helper::strip(tokens[1]);
      // TODO: ugh
      try {
        new_tileset.x = boost::lexical_cast<int>(helper::strip(tokens[2]));
      } catch (boost::bad_lexical_cast&) {
        new_tileset.x = 0;
      }
      try {
        new_tileset.y = boost::lexical_cast<int>(helper::strip(tokens[3]));
      } catch (boost::bad_lexical_cast&) {
        new_tileset.y = 0;
      }

      new_tileset.main = helper::strip(tokens[4]) == "1";

      tilesets.push_back(new_tileset);
    }
  }

  //std::cout << "Loading tile objects" << std::endl;
  load_tile_objects();
}

void preferences::load_tile_objects() try {
  boost::filesystem::path tile_objects_path(
      boost::filesystem::path(graal_dir) / "tileobjects");
  if (graal_dir.empty())
    return; // nothing to do!

  tile_object_groups.clear();
  boost::filesystem::directory_iterator it(tile_objects_path), end;
  const std::size_t prefix_len = sizeof("objects") - 1;
  for (; it != end; ++it) {
    boost::filesystem::path path = it->path();
    std::string file_name = path.filename().string();
    if (it->status().type() != boost::filesystem::regular_file || file_name.find("objects") != 0)
      continue;
    std::string group_name = file_name.substr(prefix_len, file_name.find_last_of(".") - prefix_len);
    tile_object_group_type& group = tile_object_groups[group_name];

    // catch errors in the loop so a single inaccessible
    // file won't ruin everything
    try {
      try_load_objects(path, group);
    } catch (const boost::filesystem::filesystem_error&) {
      // TODO: log error or something
    } catch (const std::runtime_error&) {
      // TODO: log error or something
    }
  }
} catch (boost::filesystem::filesystem_error&) {
  // TODO: log error or something
}

void preferences::save_tile_objects(const std::string& group) {
  boost::filesystem::path tile_objects_path(
      boost::filesystem::path(graal_dir) / "tileobjects");
  if (graal_dir.empty() || !boost::filesystem::is_directory(tile_objects_path))
    return;

  std::string file_name = "objects" + group + ".txt";
  boost::filesystem::path group_file(tile_objects_path / file_name);
  
  std::ofstream stream(group_file.string().c_str());
  stream << "GOBJSET01" << std::endl;

  preferences::tile_object_group_type object_group = tile_object_groups[group];
  preferences::tile_object_group_type::iterator it = object_group.begin(), end = object_group.end();

  for (; it != end; ++it) {
    tile_buf& buf = it->second;
    stream << std::endl;
    stream << "OBJECT " << buf.get_width() << " " << buf.get_height() << " "
           << it->first << std::endl;
    
    for (int y = 0; y < buf.get_height(); ++y) {
      for (int x = 0; x < buf.get_width(); ++x) {
        stream << helper::format_base64(buf.get_tile(x, y).index);
      }
      stream << std::endl;
    }
    stream << "OBJECTEND" << std::endl;
  }
}

Graal::tileset preferences::add_tileset(const std::string& name, const std::string& prefix, int x, int y, bool main) {
  tileset_list_type::iterator iter, end = tilesets.end();
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

Graal::tileset preferences::add_tileset(const std::string& name, const std::string& prefix) {
  return add_tileset(name, prefix, 0, 0, true);
}
