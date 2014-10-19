#include "undo_buffer.hpp"

using namespace Graal;

void level_editor::undo_buffer::push(basic_diff* diff) {
  m_undos.push_back(diff);
}

level_editor::basic_diff* level_editor::undo_buffer::apply(
    level_editor::level_map& target) {
  undos_type::auto_type op = m_undos.pop_back();
  return op->apply(target);
}

void level_editor::undo_buffer::clear() {
  m_undos.clear();
}

bool level_editor::undo_buffer::empty() {
  return m_undos.empty();
}
