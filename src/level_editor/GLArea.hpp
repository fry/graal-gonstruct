#pragma once

#include <gtkmm.h>
#include <gtkgl/gtkglarea.h>

class GLArea: public Gtk::EventBox  {
public:
  GLArea() {};
  GLArea(int* attrList);
  virtual ~GLArea() {};

  // Signal proxies
  virtual void on_gl_realize() {};
  virtual bool on_gl_expose_event(GdkEventExpose* event) { return false; }
  virtual bool on_gl_configure_event(GdkEventConfigure* event) { return true; }

  void create_gl_area(int* attrList);

  bool make_current();
  void swap_buffers();
private:
  // The Glib-wrapped GtkGLArea
  Gtk::DrawingArea* m_glarea;

  // Pass this to on_gl_init
  void glarea_on_realize();
  bool glarea_on_expose_event(GdkEventExpose* event);
  bool glarea_on_configure_event(GdkEventConfigure* event);
};
