#include "image_cache.hpp"

#include <string>
#include <map>

namespace Graal {
namespace level_editor {

class ogl_texture_cache {
public:
  typedef std::map<std::string, unsigned int> texture_map_type;

  ogl_texture_cache(image_cache& cache);

  unsigned int get_texture(const std::string& file_name);
protected:
  void on_cache_updated();

  image_cache& m_image_cache;
  texture_map_type m_textures;
};

}
}
