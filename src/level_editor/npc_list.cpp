#include "window.hpp"
#include "npc_list.hpp"
#include "helper.hpp"
#include "undo_diffs.hpp"
#include "level.hpp"
#include <string>
#include "edit_npc.hpp"

using namespace Graal;

// npc list window
level_editor::npc_list::npc_list(window& _window)
: Gtk::Dialog("NPC list", _window, false, false), m_window(_window) {
  m_list_store = Gtk::ListStore::create(columns);
  m_tree_view.set_model(m_list_store);

  m_tree_view.append_column("X", columns.x);
  m_tree_view.append_column("Y", columns.y);
  m_tree_view.append_column("Image", columns.image);

  Gtk::ScrolledWindow* scroll_links = Gtk::manage(new Gtk::ScrolledWindow());
  scroll_links->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scroll_links->set_shadow_type(Gtk::SHADOW_IN);
  scroll_links->add(m_tree_view);

  get_vbox()->pack_start(*scroll_links);

  Gtk::Button* button_edit = Gtk::manage(new Gtk::Button("Edit"));
  button_edit->signal_clicked().connect(sigc::mem_fun(this, &npc_list::on_edit_clicked));
  get_action_area()->pack_start(*button_edit);
  Gtk::Button* button_delete = Gtk::manage(new Gtk::Button("Delete"));
  button_delete->signal_clicked().connect(sigc::mem_fun(this, &npc_list::on_delete_clicked));
  get_action_area()->pack_start(*button_delete);
  
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  
  set_default_size(400, 300);

  show_all_children();
}

void level_editor::npc_list::on_show() {
  get();
  Gtk::Dialog::on_show();
}

void level_editor::npc_list::on_response(int response_id) {
  if (response_id == Gtk::RESPONSE_CLOSE) {
    hide();
  }
}

void level_editor::npc_list::on_edit_clicked() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    Gtk::TreeRow row = *iter;
    
    // get selected npc
    npc& current_npc = *row.get_value(columns.iter);
    edit_npc dialog;
    dialog.set(current_npc);
    if (dialog.run() == Gtk::RESPONSE_OK) {
      Graal::npc new_npc = dialog.get_npc();
      level_display* display = m_window.get_current_level_display();
      // TODO: urgh -- move this into level_display (edit_npc() or something)
      /*if (new_npc != current_npc) {
        display->add_undo_diff(new npc_diff(current_npc));
      }*/
      // save npc
      current_npc = new_npc;
      // TODO: this should probably not be here
      display->clear_selection();
      display->queue_draw();
    }

    get();
  }
}

void level_editor::npc_list::on_delete_clicked() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    level_display* display = m_window.get_current_level_display();
    // TODO: move this into level_display, too
    // Gtk::TreeRow row = *iter;
    // Graal::level::npc_list_type::iterator npc_iter = row.get_value(columns.iter);
    // display->add_undo_diff(new delete_npc_diff(*npc_iter));
    // display->get_current_level()->npcs.erase(npc_iter);
    get();

    display->clear_selection();
    display->queue_draw();
  }
}

// get npc data from the level
void level_editor::npc_list::get() {
  m_list_store->clear();

  level::npc_list_type::iterator iter, end;
  end = m_window.get_current_level()->npcs.end();

  for (iter = m_window.get_current_level()->npcs.begin();
       iter != end;
       iter ++) {
    Gtk::TreeModel::iterator row = m_list_store->append();
    (*row)[columns.iter] = iter;
    (*row)[columns.image] = iter->image; // TODO: unicode
    (*row)[columns.x] = iter->get_level_x();
    (*row)[columns.y] = iter->get_level_y();
  } 
}
