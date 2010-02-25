#include "ogl_texture_cache.hpp"

#include <GL/glew.h>
#include <gtkmm.h>
#include <gtkglmm.h>

using namespace Graal::level_editor;

unsigned int load_texture_from_surface(Cairo::RefPtr<Cairo::ImageSurface>& surface, unsigned int id = 0) {
  glEnable(GL_TEXTURE_2D);

  if (!id) {
    glGenTextures(1, &id);

    if (!id)
      throw std::runtime_error("Failed to allocate OpenGL texture");
  }

  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glTexImage2D(GL_TEXTURE_2D,
    0, GL_RGBA,
    surface->get_width(), surface->get_height(),
    0, GL_BGRA, GL_UNSIGNED_BYTE,
    surface->get_data());

  return id;
}

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
