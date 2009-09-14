#include "preferences.hpp"
#include "helper.hpp"
#include <fstream>
#include <sstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <core/helper.h>

using namespace Graal;

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
}

void level_editor::preferences::serialize() {
  std::ostringstream str;
  
  m_values["graal_directory"] = graal_dir;

  m_values["selection_border_while_dragging"]
    = selection_border_while_dragging ? "true" : "false";

  m_values["sticky_tile_selection"]
    = sticky_tile_selection ? "true" : "false";

  if (default_tile == -1) {
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

void level_editor::preferences::deserialize() {
  value_map_type::iterator iter, end = m_values.end();

  iter = m_values.find("graal_directory");
  if (iter != m_values.end())
    graal_dir = iter->second;

  iter = m_values.find("selection_border_while_dragging");
  if (iter != m_values.end()) {
    selection_border_while_dragging = (iter->second == "true");
  }

  iter = m_values.find("sticky_tile_selection");
  if (iter != m_values.end()) {
    sticky_tile_selection = (iter->second == "true");
  }

  hide_npcs = false;
  hide_signs = false;
  hide_links = false;

  default_tile = -1;
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

void level_editor::preferences::load_tile_objects() {
  boost::filesystem::path tile_objects_path(
      boost::filesystem::path(graal_dir) / "tileobjects");
  if (graal_dir.empty() || !boost::filesystem::is_directory(tile_objects_path))
    return;

  tile_object_groups.clear();
  boost::filesystem::directory_iterator it(tile_objects_path), end;
  for (; it != end; ++it) {
    boost::filesystem::path path = it->path();

    std::string file_name = path.leaf();
    if (it->status().type() != boost::filesystem::regular_file || file_name.find("objects") != 0)
      continue;

    std::size_t prefix_len = std::string("objects").length();
    std::string group_name = file_name.substr(prefix_len, file_name.find_last_of(".") - prefix_len);
    //std::cout << "Group name: " << group_name << std::endl;

    std::ifstream stream(path.string().c_str());

    std::string version = ::read_line(stream);
    //std::cout << "Version: " << version << std::endl;
    if (version.find(TILEOBJECTS_VERSION) != 0) {
      throw std::runtime_error("Couldn't load tile objects, version mismatch (" +
          TILEOBJECTS_VERSION + " != " + version + ")"); 
    }


    tile_object_group_type& group = tile_object_groups[group_name];
    while (!stream.eof()) {
      std::string type = read<std::string>(stream);

      if (type == "OBJECT") {
        int width, height; std::string name;
        width = read<int>(stream); height = read<int>(stream);
        name = helper::strip(read_line(stream));

        //std::cout << "OBJECT: " << name << ", " << width << "/" << height <<std::endl;
        tile_buf& tiles = group[name];
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
              int tile_index = helper::parse_base64(line.substr(x * 2, 2));
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

void level_editor::preferences::save_tile_objects(const std::string& group) {
  boost::filesystem::path tile_objects_path(
      boost::filesystem::path(graal_dir) / "tileobjects");
  if (graal_dir.empty() || !boost::filesystem::is_directory(tile_objects_path))
    return;

  std::string file_name = "objects" + group + ".txt";
  boost::filesystem::path group_file(tile_objects_path / file_name);
  
  std::ofstream stream(group_file.string().c_str());
  stream << "GOBJSET01" << std::endl;

  tile_object_group_type object_group = tile_object_groups[group];
  tile_object_group_type::iterator it = object_group.begin(), end = object_group.end();

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
