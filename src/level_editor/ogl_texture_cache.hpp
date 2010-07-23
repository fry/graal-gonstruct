#pragma once

#include "image_cache.hpp"

#include <string>
#include <map>

namespace Graal {
namespace level_editor {

/* Contains everything required to draw a texture:
 * width, height: the correct u, v corrdinates to use [0, 1]
 * image_width, image_height: the actual size of the original image in pixel
 */
struct texture_info {
  unsigned int index;
  double width;
  double height;
  
  unsigned int image_width, image_height;
};

class ogl_texture_cache {
public:
  typedef std::map<std::string, texture_info> texture_map_type;

  ogl_texture_cache(image_cache& cache);

  const texture_info& get_texture(const std::string& file_name);
protected:
  void on_cache_updated();

  image_cache& m_image_cache;
  texture_map_type m_textures;
};

}
}
