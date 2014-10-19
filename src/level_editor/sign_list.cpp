#include "sign_list.hpp"
#include "window.hpp"
#include "helper.hpp"
#include <boost/lexical_cast.hpp>

using namespace Graal;

namespace {
  template<typename T>
  bool parse(const std::string& str, T& output) {
    std::istringstream ss(str);
    T value;
    ss >> value;
    if (ss.fail())
      return false;

    output = value;
    return true;
  }
}

// sign list window
level_editor::sign_list::sign_list(window& _window)
: Gtk::Dialog("Sign list", _window, false, false), m_window(_window) {
  m_list_store = Gtk::ListStore::create(columns);
  m_tree_view.set_model(m_list_store);

  m_tree_view.append_column("Index", columns.index);
  m_tree_view.append_column("X", columns.x);
  m_tree_view.append_column("Y", columns.y);

  Gtk::ScrolledWindow* scroll_signs = Gtk::manage(new Gtk::ScrolledWindow());
  scroll_signs->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scroll_signs->set_shadow_type(Gtk::SHADOW_IN);
  scroll_signs->add(m_tree_view);
  get_vbox()->pack_start(*scroll_signs);

  Gtk::HBox* sign_menu = Gtk::manage(new Gtk::HBox());
  Gtk::Button* button_create = Gtk::manage(new Gtk::Button("Create"));
  button_create->signal_clicked().connect(sigc::mem_fun(this, &sign_list::on_create_clicked));
  sign_menu->pack_start(*button_create, Gtk::PACK_EXPAND_WIDGET, 2);
  Gtk::Button* button_delete = Gtk::manage(new Gtk::Button("Delete"));
  button_delete->signal_clicked().connect(sigc::mem_fun(this, &sign_list::on_delete_clicked));
  sign_menu->pack_start(*button_delete, Gtk::PACK_EXPAND_WIDGET, 2);

  Gtk::Label* label_x = Gtk::manage(new Gtk::Label("X:"));
  sign_menu->pack_start(*label_x, Gtk::PACK_EXPAND_WIDGET, 5);
  m_edit_x.set_width_chars(4);
  sign_menu->pack_start(m_edit_x);
  Gtk::Label* label_y = Gtk::manage(new Gtk::Label("Y:"));
  sign_menu->pack_start(*label_y, Gtk::PACK_EXPAND_WIDGET, 5);
  m_edit_y.set_width_chars(4);
  sign_menu->pack_start(m_edit_y);

  get_vbox()->pack_start(*sign_menu, Gtk::PACK_SHRINK, 5);

  Gtk::ScrolledWindow* scroll_text = Gtk::manage(new Gtk::ScrolledWindow());
  scroll_text->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scroll_text->set_shadow_type(Gtk::SHADOW_IN);
  scroll_text->add(m_text_view);
  m_text_view.set_indent(3);
  get_vbox()->pack_start(*scroll_text);
  
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

  m_tree_view.get_selection()->signal_changed().connect_notify(
    sigc::mem_fun(this, &sign_list::on_sign_changed)
  );

  m_text_view.get_buffer()->signal_changed().connect_notify(
    sigc::mem_fun(this, &sign_list::on_text_changed)
  );

  // TODO: ugh
  m_edit_x.signal_changed().connect_notify(
    sigc::mem_fun(this, &sign_list::on_x_changed)
  );

  m_edit_y.signal_changed().connect_notify(
    sigc::mem_fun(this, &sign_list::on_y_changed)
  );

  set_default_size(400, 300);

  show_all_children();
}

void level_editor::sign_list::on_show() {
  get();
  Gtk::Dialog::on_show();
}

void level_editor::sign_list::on_response(int response_id) {
  if (response_id == Gtk::RESPONSE_CLOSE) {
    hide();
  }
}

void level_editor::sign_list::on_create_clicked() {
  sign new_sign;
  parse(m_edit_x.get_text(), new_sign.x);
  parse(m_edit_y.get_text(), new_sign.y);

  Graal::level& level = *m_window.get_current_level();
  new_sign.x = helper::bound_by(new_sign.x, 0, level.get_width());
  new_sign.y = helper::bound_by(new_sign.y, 0, level.get_height());

  m_window.get_current_level()->signs.push_back(new_sign);
  get();
  
  // select the last item and scroll to it
  Gtk::TreeIter iter_last = m_list_store->children()[m_list_store->children().size() - 1];
  m_tree_view.get_selection()->select(iter_last);
  m_tree_view.scroll_to_row(m_list_store->get_path(iter_last));
  m_window.get_current_level_display()->queue_draw();
}

void level_editor::sign_list::on_delete_clicked() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    Gtk::TreeRow row = *iter;
    m_window.get_current_level()->signs.erase(row.get_value(columns.iter));
    get();
    m_window.get_current_level_display()->queue_draw();
  }
}

// get sign data from the level
void level_editor::sign_list::get() {
  m_list_store->clear();

  level_display& display = *m_window.get_current_level_display();
  if (display.has_selection()) {
    // TODO: find another solution for this
    /*m_edit_x.set_text(boost::lexical_cast<std::string>(display.select_tile_x()));
    m_edit_y.set_text(boost::lexical_cast<std::string>(display.select_tile_y()));*/
  }
  level::sign_list_type::iterator iter, end;
  end = m_window.get_current_level()->signs.end();

  int index = 0;
  for (iter = m_window.get_current_level()->signs.begin();
       iter != end;
       iter ++) {
    Gtk::TreeModel::iterator row = m_list_store->append();
    (*row)[columns.iter] = iter;
    (*row)[columns.index] = index;
    (*row)[columns.x] = iter->x;
    (*row)[columns.y] = iter->y;
    (*row)[columns.text] = iter->text; // TODO: unicode
    index ++;
  } 
}

void level_editor::sign_list::set() {
  Gtk::TreeIter iter, end;
  end = m_list_store->children().end();
  for (iter = m_list_store->children().begin();
       iter != end;
       iter ++) {
    sign& _sign = *iter->get_value(columns.iter);
    _sign.x = iter->get_value(columns.x);
    _sign.y = iter->get_value(columns.y);
    _sign.text = iter->get_value(columns.text);
  }

  m_window.get_current_level_display()->queue_draw();
}

void level_editor::sign_list::on_sign_changed() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    m_text_view.get_buffer()->set_text((*iter)[columns.text]);
    m_edit_x.set_text(boost::lexical_cast<std::string>((*iter)[columns.x]));
    m_edit_y.set_text(boost::lexical_cast<std::string>((*iter)[columns.y]));
  }
}

void level_editor::sign_list::on_x_changed() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    int value = 0;
    parse(m_edit_x.get_text(), value);
    (*iter)[columns.x] = value;
  }

  set();
}

void level_editor::sign_list::on_y_changed() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    int value = 0;
    parse(m_edit_y.get_text(), value);
    (*iter)[columns.y] = value;
  }

  set();
}

void level_editor::sign_list::on_text_changed() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    (*iter)[columns.text] = m_text_view.get_buffer()->get_text();
  }

  set();
}
