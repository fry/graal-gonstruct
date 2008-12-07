#ifndef GRAAL_LEVEL_EDITOR_UNDO_BUFFER_HPP_
#define GRAAL_LEVEL_EDITOR_UNDO_BUFFER_HPP_

#include <boost/ptr_container/ptr_list.hpp>
#include "undo_diffs.hpp"

namespace Graal {
  namespace level_editor {
    class undo_buffer {
    public:
      basic_diff* apply(Graal::level_editor::level_display& target);
      void clear();
      bool empty();

      void push(basic_diff* diff);
    private:
      typedef boost::ptr_list<basic_diff> undos_type;
      undos_type m_undos;
    };
  }
}

#endif
