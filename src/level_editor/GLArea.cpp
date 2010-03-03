#include "GLArea.hpp"

GLArea::GLArea(int* attrList) {
  create_gl_area(attrList);
}

void GLArea::create_gl_area(int* attrList) {
  GtkWidget* gtkglarea = gtk_gl_area_new(attrList);
  m_glarea = Glib::wrap(GTK_DRAWING_AREA(gtkglarea));

  m_glarea->add_events(
    Gdk::ALL_EVENTS_MASK);

  m_glarea->signal_realize().connect(
    sigc::mem_fun(this, &GLArea::glarea_on_realize));
  m_glarea->signal_expose_event().connect(
    sigc::mem_fun(this, &GLArea::glarea_on_expose_event));
  m_glarea->signal_configure_event().connect(
    sigc::mem_fun(this, &GLArea::glarea_on_configure_event));

  add(*m_glarea);
  m_glarea->show();
}

bool GLArea::make_current() {
  return gtk_gl_area_make_current(GTK_GL_AREA(m_glarea->gobj()));
}

void GLArea::swap_buffers() {
  gtk_gl_area_swapbuffers(GTK_GL_AREA(m_glarea->gobj()));
}

void GLArea::glarea_on_realize() {
  on_gl_realize();
}

bool GLArea::glarea_on_expose_event(GdkEventExpose* event) {
  return on_gl_expose_event(event);
};

bool GLArea::glarea_on_configure_event(GdkEventConfigure* event) {
  return on_gl_configure_event(event);
}
