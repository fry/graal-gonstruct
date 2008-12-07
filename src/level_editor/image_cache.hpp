#ifndef GRAAL_LEVEL_EDITOR_IMAGE_CACHE_HPP_
#define GRAAL_LEVEL_EDITOR_IMAGE_CACHE_HPP_

#include <map>
#include <string>

#include <boost/noncopyable.hpp>
#include <sigc++/signal.h>
#include <cairomm/surface.h>

namespace Graal {
  namespace level_editor {
    class filesystem;

    class image_cache : boost::noncopyable {
    public:
      typedef Cairo::RefPtr<Cairo::ImageSurface> image_ptr;
      typedef std::map<std::string, image_ptr> image_map_type;
      typedef sigc::signal<void> signal_cache_update_type;

      image_cache(filesystem& fs);

      image_ptr& get_image(const std::string& file_name);

      void clear_cache();

      signal_cache_update_type& signal_cache_update();

    private:
      void load_internal_images();

      filesystem& m_fs;
      image_map_type m_cache;
      image_ptr m_default_image;
      image_ptr m_npc_image;

      signal_cache_update_type m_signal_cache_update;
    };
  }
}

#endif
