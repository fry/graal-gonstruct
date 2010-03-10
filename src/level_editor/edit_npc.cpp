#include "edit_npc.hpp"
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguagemanager.h>

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
  m_edit_x.set_text(boost::lexical_cast<std::string>(_npc.get_level_x()));
  m_edit_y.set_text(boost::lexical_cast<std::string>(_npc.get_level_y()));

  GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view));
  gtk_text_buffer_set_text(buf, _npc.script.c_str(), _npc.script.size());
}

npc level_editor::edit_npc::get_npc() {
  npc new_npc(m_npc);
  new_npc.image = m_edit_image.get_text();
  float new_x, new_y;
  helper::parse<float>(m_edit_x.get_text(), new_x);
  new_npc.set_level_x(new_x);
  helper::parse<float>(m_edit_y.get_text(), new_y);
  new_npc.set_level_y(new_y);

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