#include "_precompiled.hpp"
#include "window.hpp"
#include "npc_list.hpp"
#include "helper.hpp"
#include "undo_diffs.hpp"
#include "level.hpp"
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <boost/lexical_cast.hpp>
#include <string>

using namespace Graal;

// edit npc window
level_editor::edit_npc::edit_npc()
: Gtk::Dialog("Edit NPC", true, false) {
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

  Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
  Gtk::Label* label_image = Gtk::manage(new Gtk::Label("Image:"));
  Gtk::Label* label_x = Gtk::manage(new Gtk::Label("X:"));
  Gtk::Label* label_y = Gtk::manage(new Gtk::Label("Y:"));

  hbox->pack_start(*label_image, Gtk::PACK_EXPAND_WIDGET, 5);
  hbox->pack_start(m_edit_image);
  hbox->pack_start(*label_x, Gtk::PACK_EXPAND_WIDGET, 5);
  m_edit_x.set_width_chars(4);
  hbox->pack_start(m_edit_x);
  hbox->pack_start(*label_y, Gtk::PACK_EXPAND_WIDGET, 5);
  m_edit_y.set_width_chars(4);
  hbox->pack_start(m_edit_y);

  get_vbox()->pack_start(*hbox, Gtk::PACK_SHRINK, 3);

  Gtk::ScrolledWindow* scrolled = Gtk::manage(new Gtk::ScrolledWindow());
  scrolled->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrolled->set_shadow_type(Gtk::SHADOW_IN);

  m_view = gtk_source_view_new();
  gtk_container_add(GTK_CONTAINER(scrolled->gobj()), GTK_WIDGET(m_view));
  GtkSourceView* view = GTK_SOURCE_VIEW(m_view);
  gtk_source_view_set_auto_indent(view, TRUE);
  gtk_source_view_set_indent_on_tab(view, TRUE);
  gtk_source_view_set_indent_width(view, 2);
  gtk_source_view_set_insert_spaces_instead_of_tabs(view, TRUE);
  gtk_source_view_set_highlight_current_line(view, TRUE);
  gtk_source_view_set_show_line_numbers(view, TRUE);
  gtk_source_view_set_show_right_margin(view, TRUE);
  gtk_source_view_set_tab_width(view, 2);

  PangoFontDescription* font =
    pango_font_description_from_string("Monospace 10");
  gtk_widget_modify_font(m_view, font);
  pango_font_description_free(font);

  GtkTextView* tview = GTK_TEXT_VIEW(m_view);
  gtk_text_view_set_indent(tview, 3);

  GtkSourceBuffer* buf = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(tview));
  GtkSourceLanguageManager* lm = gtk_source_language_manager_get_default();
  GtkSourceLanguage* lang =
    gtk_source_language_manager_get_language(lm, "graal");
  if (!lang)
    lang = gtk_source_language_manager_get_language(lm, "js");
  gtk_source_buffer_set_language(buf, lang);
  gtk_source_buffer_set_highlight_syntax(buf, TRUE);

  get_vbox()->pack_start(*scrolled);

  set_default_size(500, 500);
  show_all_children();
}

void level_editor::edit_npc::set(const npc& _npc) {
  m_npc = _npc;
  m_edit_image.set_text(_npc.image);
  m_edit_x.set_text(boost::lexical_cast<std::string>(_npc.x));
  m_edit_y.set_text(boost::lexical_cast<std::string>(_npc.y));

  GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view));
  gtk_text_buffer_set_text(buf, _npc.script.c_str(), _npc.script.size());
}

npc level_editor::edit_npc::get_npc() {
  npc new_npc(m_npc);
  new_npc.image = m_edit_image.get_text();
  helper::parse<int>(m_edit_x.get_text(), new_npc.x);
  helper::parse<int>(m_edit_y.get_text(), new_npc.y);

  GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view));
  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter(buf, &start);
  gtk_text_buffer_get_end_iter(buf, &end);
  gchar* text = gtk_text_buffer_get_text(buf, &start, &end, TRUE);
  try {
    new_npc.script.assign(text);
  } catch (...) {
    g_free(text);
    throw;
  }
  g_free(text);

  return new_npc;
}


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
      // TODO: urgh
      if (new_npc != current_npc) {
        display->add_undo_diff(new npc_diff(current_npc));
      }
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
    Gtk::TreeRow row = *iter;
    level_display* display = m_window.get_current_level_display();
    Graal::level::npc_list_type::iterator npc_iter = row.get_value(columns.iter);
    display->add_undo_diff(new delete_npc_diff(*npc_iter));
    display->get_level()->npcs.erase(npc_iter);
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
    (*row)[columns.x] = iter->x;
    (*row)[columns.y] = iter->y;
  } 
}
