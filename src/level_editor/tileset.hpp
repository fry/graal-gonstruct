#ifndef GRAAL_LEVEL_EDITOR_TILESET_
#define GRAAL_LEVEL_EDITOR_TILESET_

#include <gtkmm.h>

namespace Graal {
  //namespace level_editor {
    class tileset {
    public:
      tileset();

      //Cairo::RefPtr<Cairo::ImageSurface> image;
      std::string name;
      std::string prefix;
      int x, y;

      bool main;
    };
  //}
}

#endif
