#include "_precompiled.hpp"
#include "link_list.hpp"
#include "window.hpp"
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

// edit link window
level_editor::edit_link::edit_link(window& _window)
: Gtk::Dialog("Edit link", true, false), m_window(_window) {
  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  // source controls
  Gtk::Frame* frame_source = Gtk::manage(new Gtk::Frame("Source"));
  Gtk::HBox* source_coords = Gtk::manage(new Gtk::HBox());
  Gtk::Label* label_x = Gtk::manage(new Gtk::Label("X:"));
  Gtk::Label* label_y = Gtk::manage(new Gtk::Label("Y:"));
  Gtk::Label* label_width = Gtk::manage(new Gtk::Label("W:"));
  Gtk::Label* label_height = Gtk::manage(new Gtk::Label("H:"));
  source_coords->pack_start(*label_x, Gtk::PACK_SHRINK, 5);
  m_edit_x.set_width_chars(4);
  source_coords->pack_start(m_edit_x);
  source_coords->pack_start(*label_y, Gtk::PACK_SHRINK, 5);
  m_edit_y.set_width_chars(4);
  source_coords->pack_start(m_edit_y);
  source_coords->pack_start(*label_width, Gtk::PACK_SHRINK, 5);
  m_edit_width.set_width_chars(4);
  source_coords->pack_start(m_edit_width);
  source_coords->pack_start(*label_height, Gtk::PACK_SHRINK, 5);
  m_edit_height.set_width_chars(4);
  source_coords->pack_start(m_edit_height);

  source_coords->set_border_width(5);
  frame_source->add(*source_coords);
  
  get_vbox()->pack_start(*frame_source);

  // destination controls
  Gtk::Frame* frame_dest = Gtk::manage(new Gtk::Frame("Destination"));
  Gtk::Label* label_dest_level = Gtk::manage(new Gtk::Label("Level:"));
  Gtk::Label* label_new_x = Gtk::manage(new Gtk::Label("New X:"));
  Gtk::Label* label_new_y = Gtk::manage(new Gtk::Label("New Y:"));
  Gtk::Table* table = Gtk::manage(new Gtk::Table(2, 3));
  table->attach(*label_dest_level, 0, 1, 0, 1);
  table->attach(m_edit_destination, 1, 2, 0, 1);
  table->attach(*label_new_x, 0, 1, 1, 2);
  table->attach(m_edit_new_x, 1, 2, 1, 2);
  table->attach(*label_new_y, 0, 1, 2, 3);
  table->attach(m_edit_new_y, 1, 2, 2, 3);
  table->set_border_width(5);
  frame_dest->add(*table);
  get_vbox()->pack_start(*frame_dest);
  show_all_children();
}

Graal::link level_editor::edit_link::get_link() {
  link new_link;
  parse<int>(m_edit_x.get_text(), new_link.x);
  parse<int>(m_edit_y.get_text(), new_link.y);
  parse<int>(m_edit_width.get_text(), new_link.width);
  parse<int>(m_edit_height.get_text(), new_link.height);

  new_link.destination = m_edit_destination.get_text();
  new_link.new_x = m_edit_new_x.get_text();
  new_link.new_y = m_edit_new_y.get_text();

  return new_link;
}

void level_editor::edit_link::get(const link& _link) {
  m_edit_x.set_text(boost::lexical_cast<std::string>(_link.x));
  m_edit_y.set_text(boost::lexical_cast<std::string>(_link.y));
  m_edit_width.set_text(boost::lexical_cast<std::string>(_link.width));
  m_edit_height.set_text(boost::lexical_cast<std::string>(_link.height));

  m_edit_destination.set_text(_link.destination);
  m_edit_new_x.set_text(_link.new_x);
  m_edit_new_y.set_text(_link.new_y);
}

// link list window
level_editor::link_list::link_list(window& _window)
: Gtk::Dialog("Link list", _window, false, false), m_window(_window) {
  m_list_store = Gtk::ListStore::create(columns);
  m_tree_view.set_model(m_list_store);

  m_tree_view.append_column("Destination", columns.destination);
  m_tree_view.append_column("New X", columns.new_x);
  m_tree_view.append_column("New Y", columns.new_y);

  Gtk::ScrolledWindow* scroll_links = Gtk::manage(new Gtk::ScrolledWindow());
  scroll_links->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scroll_links->set_shadow_type(Gtk::SHADOW_IN);
  scroll_links->add(m_tree_view);

  get_vbox()->pack_start(*scroll_links);

  Gtk::Button* button_edit = Gtk::manage(new Gtk::Button("Edit"));
  button_edit->signal_clicked().connect(sigc::mem_fun(this, &link_list::on_edit_clicked));
  get_action_area()->pack_start(*button_edit);
  Gtk::Button* button_delete = Gtk::manage(new Gtk::Button("Delete"));
  button_delete->signal_clicked().connect(sigc::mem_fun(this, &link_list::on_delete_clicked));
  get_action_area()->pack_start(*button_delete);
  
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  
  set_default_size(400, 300);

  show_all_children();
}

void level_editor::link_list::on_show() {
  get();
  Gtk::Dialog::on_show();
}

void level_editor::link_list::on_response(int response_id) {
  if (response_id == Gtk::RESPONSE_CLOSE) {
    hide();
  }
}

void level_editor::link_list::on_edit_clicked() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    Gtk::TreeRow row = *iter;
    
    // get selected link
    link& _link = *row.get_value(columns.iter);
    edit_link edit_window(m_window);
    edit_window.get(_link);
    if (edit_window.run() == Gtk::RESPONSE_OK) {
      // save link
      _link = edit_window.get_link();
      // TODO: this should probably not be here
      m_window.get_current_level_display()->queue_draw();
    }

    get();
  }
}

void level_editor::link_list::on_delete_clicked() {
  Glib::RefPtr<Gtk::TreeView::Selection> selection = m_tree_view.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (iter) {
    Gtk::TreeRow row = *iter;
    m_window.get_current_level()->links.erase(row.get_value(columns.iter));
    get();
    m_window.get_current_level_display()->queue_draw();
  }
}

// get link data from the level
void level_editor::link_list::get() {
  m_list_store->clear();

  level::link_list_type::iterator iter, end;
  end = m_window.get_current_level()->links.end();

  for (iter = m_window.get_current_level()->links.begin();
       iter != end;
       iter ++) {
    Gtk::TreeModel::iterator row = m_list_store->append();
    (*row)[columns.iter] = iter;
    (*row)[columns.destination] = iter->destination; // TODO: unicode
    (*row)[columns.new_x] = iter->new_x;
    (*row)[columns.new_y] = iter->new_y;
  } 
}

