#include "ogl_texture_cache.hpp"
#include "ogl_tiles_display.hpp"

#include <GL/glew.h>
#include <gtkmm.h>
#include <gtkglmm.h>

using namespace Graal::level_editor;

ogl_texture_cache::ogl_texture_cache(image_cache& cache): m_image_cache(cache) {
  cache.signal_cache_update().connect(
      sigc::mem_fun(this, &ogl_texture_cache::on_cache_updated));
}

void ogl_texture_cache::on_cache_updated() {
  texture_map_type::iterator iter, end = m_textures.end();

  for (iter = m_textures.begin(); iter != end; ++iter) {
    glDeleteTextures(1, &iter->second);
  }

  m_textures.clear();
}

unsigned int ogl_texture_cache::get_texture(const std::string& file_name) {
  texture_map_type::iterator iter = m_textures.find(file_name);
  if (iter != m_textures.end())
    return iter->second;

  // Texture not loaded yet, load it
  Cairo::RefPtr<Cairo::ImageSurface>& surface = m_image_cache.get_image(file_name);
  unsigned int id = load_texture_from_surface(surface);
  m_textures[file_name] = id;

  return id;
}
