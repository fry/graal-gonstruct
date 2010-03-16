#include "edit_commands.hpp"
#include "window.hpp"
#include "level_display.hpp"

using namespace Graal;
using namespace Graal::level_editor;

edit_commands::edit_commands(window& _window, header& _header):
  m_window(_window)
{
  _header.action_edit_undo->signal_activate().connect(
    sigc::mem_fun(*this, &edit_commands::on_action_undo));
  _header.action_edit_redo->signal_activate().connect(
    sigc::mem_fun(*this, &edit_commands::on_action_redo));
  _header.action_edit_cut->signal_activate().connect(
    sigc::mem_fun(*this, &edit_commands::on_action_cut));
  _header.action_edit_copy->signal_activate().connect(
    sigc::mem_fun(*this, &edit_commands::on_action_copy));
  _header.action_edit_paste->signal_activate().connect(
    sigc::mem_fun(*this, &edit_commands::on_action_paste));
  _header.action_edit_delete->signal_activate().connect(
    sigc::mem_fun(*this, &edit_commands::on_action_delete));
  _header.action_edit_preferences->signal_activate().connect(
    sigc::mem_fun(*this, &edit_commands::on_action_prefs));
}


void edit_commands::on_action_undo() {
  level_display* current = m_window.get_current_level_display();
  g_assert(current != 0);
  current->undo();
  current->clear_selection();
  current->queue_draw();
}

void edit_commands::on_action_redo() {
  level_display* current = m_window.get_current_level_display();
  g_assert(current != 0);
  current->redo();
  current->clear_selection();
  current->queue_draw();
}

void edit_commands::on_action_cut() {
  level_display& display = *m_window.get_current_level_display();
  if (!display.npc_selected()) {
    tile_buf buf(display.selection);
    if (buf.empty()) {
      display.lift_selection();
      buf = display.selection;
    }
    m_window.copy_cache.reset(new tiles_cache(buf));
  } else {
    npc& selected_npc = *display.get_level_map()->get_npc(display.selected_npc);
    m_window.copy_cache.reset(new npc_cache(selected_npc));
  }

  display.delete_selection();
}

void edit_commands::on_action_copy() {
  level_display& display = *m_window.get_current_level_display();
  if (!display.npc_selected()) {
    tile_buf buf(display.selection);
    if (buf.empty()) {
      display.lift_selection();
      buf = display.selection;
      display.undo();
    }
    m_window.copy_cache.reset(new tiles_cache(buf));
  } else {
    npc& selected_npc = *display.get_level_map()->get_npc(display.selected_npc);
    m_window.copy_cache.reset(new npc_cache(selected_npc));
  }
}

void edit_commands::on_action_paste() {
  level_display& display = *m_window.get_current_level_display();
  m_window.copy_cache->paste(display);
}

void edit_commands::on_action_prefs() {
  m_window.prefs_display.update_controls();
  m_window.prefs_display.present();
}

void edit_commands::on_action_delete() {
  level_display* current = m_window.get_current_level_display();
  g_assert(current != 0);
  current->delete_selection();
}
