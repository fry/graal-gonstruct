#include "ogl_tiles_display.hpp"
#include "helper.hpp"
#include <iostream>

#include <boost/format.hpp>

// Needs this for _gtk_VOID__OBJECT_OBJECT
#include "gtkmarshalers.h"

using namespace Graal;
using namespace Graal::level_editor;

#ifdef DEBUG
  Glib::Timer g_timer;
  unsigned int g_frames = 0;
#endif

unsigned int Graal::level_editor::load_texture_from_surface(Cairo::RefPtr<Cairo::ImageSurface>& surface, unsigned int id) {
  glEnable(GL_TEXTURE_2D);

  if (!GLEW_ARB_texture_non_power_of_two) {
    std::cout << "No ARB_texture_non_power_of_two support" << std::endl;
  }

  if (!id) {
    glGenTextures(1, &id);

    if (!id)
      throw std::runtime_error("Failed to allocate OpenGL texture");
  }

  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glTexImage2D(GL_TEXTURE_2D,
    0, GL_RGBA,
    surface->get_width(), surface->get_height(),
    0, GL_BGRA, GL_UNSIGNED_BYTE,
    surface->get_data());
  
  GLenum err = glGetError();
  if (err)
    throw std::runtime_error((boost::format("Loading texture failed: %1%") % err).str());

  return id;
}

namespace {
  // A little wrapper for the GTK C signal callback from set-scroll-adjustments
  static void ogl_tiles_display_set_adjustments(void* display_gobj, GtkAdjustment* hadj, GtkAdjustment* vadj, ogl_tiles_display* display) {
    display->set_adjustments(
      Glib::wrap(hadj),
      Glib::wrap(vadj));
  }
}

void ogl_tiles_display::set_adjustments(Gtk::Adjustment* hadjustment, Gtk::Adjustment* vadjustment) {
  m_hadjustment = hadjustment;
  m_vadjustment = vadjustment;

  // Invalidate screen when scrolling
  if (m_hadjustment) {
    m_hadjustment->signal_value_changed().connect_notify(
      sigc::mem_fun(*this, &ogl_tiles_display::invalidate));
  }

  if (m_vadjustment) {
    m_vadjustment->signal_value_changed().connect_notify(
      sigc::mem_fun(*this, &ogl_tiles_display::invalidate));
  }
}


ogl_tiles_display::ogl_tiles_display():
  m_tileset(0),
  m_tile_width(16), // TODO: take a parameter for this?
  m_tile_height(16),
  m_hadjustment(0),
  m_vadjustment(0)
{

  /* Set up for custom scrolling handling. GTK does this in a pretty terrible
   * way by providing a signal slot to fill that gets emitted when the parent
   * sets GtkAdjustments.
   * So we create the empty signal here, set it in the class object, and
   * connect to it right again because the function you can provide in
   * g_signal_new is only a gobject class offset, which we can't provide.
   */
  GtkWidget* widget = (GtkWidget*)gobj();
  GtkWidgetClass* klass = GTK_WIDGET_GET_CLASS(widget);

  if (!klass->set_scroll_adjustments_signal) {
    guint adjust_signal = g_signal_new(
      "set-scroll-adjustments",
      G_OBJECT_CLASS_TYPE(klass),
      (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
      0,
      NULL, NULL,
      _gtk_VOID__OBJECT_OBJECT,
      GTK_TYPE_NONE, 2,
      GTK_TYPE_ADJUSTMENT,
      GTK_TYPE_ADJUSTMENT);

    klass->set_scroll_adjustments_signal = adjust_signal;
  }

  g_signal_connect(widget, "set-scroll-adjustments",
  G_CALLBACK(ogl_tiles_display_set_adjustments), this);

  // Set up GLArea
  int attrlist[] = {
    GDK_GL_RGBA,
    GDK_GL_DOUBLEBUFFER,
    //GDK_GL_DEPTH_SIZE, 1,
    GDK_GL_NONE
  };

  create_gl_area(attrlist);

  add_events(Gdk::VISIBILITY_NOTIFY_MASK);

  // Make it possible to grab focus
  property_can_focus().set_value(true);
}

bool ogl_tiles_display::on_gl_configure_event(GdkEventConfigure* event) {
  if (!make_current())
    return false;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, get_width(), get_height(), 0, -1, 1);

  glViewport(0, 0, get_width(), get_height());

  // Update adjustments and reclamp
  if (m_hadjustment) {
    m_hadjustment->set_page_size(get_width());
    m_hadjustment->set_value(
      helper::bound_by(
        static_cast<int>(m_hadjustment->get_value()),
        0, static_cast<int>(m_hadjustment->get_upper())));
  }
  if (m_vadjustment) {
    m_vadjustment->set_page_size(get_height());
    m_vadjustment->set_value(
      helper::bound_by(
        static_cast<int>(m_vadjustment->get_value()),
        0, static_cast<int>(m_vadjustment->get_upper())));
  }
  invalidate();

  return true;
}

void ogl_tiles_display::on_gl_realize() {
  if (!make_current())
    throw std::runtime_error("Failed to initialize device context");

  GLenum err = glewInit();

  if (err != GLEW_OK) {
    std::string err_msg = "Failed to initialize glew: ";
    err_msg += (const char*)glewGetErrorString(err);
    throw std::runtime_error(err_msg);
  }

  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  glDisable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 1.0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  set_surface_size();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, get_width(), get_height(), 0, -1, 1);
  glViewport(0, 0, get_width(), get_height());

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  #ifdef DEBUG
    g_timer.start();
  #endif
}

void ogl_tiles_display::draw_all() {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, m_tileset);
  tile_buf& buf = get_tile_buf();
  const int width = buf.get_width();
  const int height = buf.get_height();

  glBegin(GL_QUADS);
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      draw_tile(buf.get_tile(x, y), x, y);
    }
  }
  glEnd();
}

void ogl_tiles_display::draw_tile(const tile& _tile, int x, int y) {
  // The position of the actual tile inside the tileset
  const int tx = helper::get_tile_x(_tile.index);
  const int ty = helper::get_tile_y(_tile.index);

  // Build texture coordinates
  int dx = x * m_tile_width;
  int dy = y * m_tile_height;
  float x1 = (float)(tx * m_tile_width)/m_tileset_width;
  float x2 = (float)((tx+1)*m_tile_width)/m_tileset_width;
  float y1 = (float)(ty*m_tile_height)/m_tileset_height;
  float y2 = (float)((ty+1)*m_tile_height)/m_tileset_height;

  //glBegin(GL_QUADS);
    // Top left
    glTexCoord2f(x1, y1);
    glVertex2f(dx, dy);
    // Top right
    glTexCoord2f(x2, y1);
    glVertex2f(dx + m_tile_width, dy);
    // Bottom right
    glTexCoord2f(x2, y2);
    glVertex2f(dx + m_tile_width, dy + m_tile_height);
    // Bottom left
    glTexCoord2f(x1, y2);
    glVertex2f(dx, dy + m_tile_height);
  //glEnd();
}

void ogl_tiles_display::load_tileset(Cairo::RefPtr<Cairo::ImageSurface>& surface) {
  glEnable(GL_TEXTURE_2D);

  m_tileset = load_texture_from_surface(surface, m_tileset);

  m_tileset_width = surface->get_width();
  m_tileset_height = surface->get_height();
  
  invalidate();
}

bool ogl_tiles_display::on_gl_expose_event(GdkEventExpose* event) {
  if (!make_current()) {
    return false;
  }

  if (m_new_tileset) {
    load_tileset(m_new_tileset);
    m_new_tileset.clear();
  }

  if (!m_tileset) {
    return false;
  }

#ifdef DEBUG
  float vp[4];
  glGetFloatv(GL_VIEWPORT, vp);
  std::cout << "ogl_tiles_display::on_gl_expose_event" << std::endl;
  std::cout << "viewport: " << vp[0] << "," << vp[1] << "," << vp[2] << "," << vp[3] << std::endl;
#endif

  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Draw everything
  draw_all();

  swap_buffers();

  #ifdef DEBUG
    // Display FPS
    // TODO: temporary
    g_frames ++;

    double seconds = g_timer.elapsed();
    if (seconds > 2) {
      std::cout.setf(std::ios::fixed, std::ios::floatfield);
      std::cout.precision(3);
      std::cout << this << "| FPS: " << g_frames/seconds << std::endl;
      g_timer.reset();
      g_frames = 0;
    }
  #endif

  return true;
}

void ogl_tiles_display::invalidate() {
  queue_draw();
  //get_window()->invalidate_rect(get_allocation(), false);
}

void ogl_tiles_display::set_tile_size(int tile_width, int tile_height) {
  m_tile_width = tile_width;
  m_tile_height = tile_height;
  
  invalidate();
}

void ogl_tiles_display::set_tileset_surface(
    const Cairo::RefPtr<Cairo::ImageSurface>& surface) {
  m_new_tileset = surface;
  invalidate();
}

void ogl_tiles_display::set_surface_size() {
  // Resize to fit one level
  // TODO
  /*set_size_request(
    64 * m_tile_width,
    32 * m_tile_height
  );*/

  invalidate();
}

void ogl_tiles_display::clear() {
  get_tile_buf().clear();
  set_surface_size();
}

void ogl_tiles_display::set_tile_buf(tile_buf& buf) {
  m_tile_buf.swap(buf);
  set_surface_size();
}

void ogl_tiles_display::set_scroll_size(int width, int height) {
  if (m_hadjustment && m_vadjustment) {
    m_hadjustment->set_lower(0);
    m_hadjustment->set_upper(width);

    m_vadjustment->set_lower(0);
    m_vadjustment->set_upper(height);

    m_hadjustment->set_step_increment(width * 0.1);
    m_hadjustment->set_page_increment(width * 0.1);

    m_vadjustment->set_step_increment(height * 0.1);
    m_vadjustment->set_page_increment(height * 0.1);
  }
}

void ogl_tiles_display::get_scroll_offset(int& x, int& y) {
  x = 0;
  y = 0;

  if (m_hadjustment)
    x = static_cast<int>(m_hadjustment->get_value());

  if (m_vadjustment)
    y = static_cast<int>(m_vadjustment->get_value());
}

void ogl_tiles_display::set_scroll_offset(int x, int y) {
  if (m_hadjustment)
    m_hadjustment->set_value(x);
  if (m_vadjustment)
    m_vadjustment->set_value(y);
}

void ogl_tiles_display::get_cursor_position(int& x, int& y) {
  get_pointer(x, y);
  int ox, oy;
  get_scroll_offset(ox, oy);

  x += ox;
  y += oy;
}

void ogl_tiles_display::get_cursor_tiles_position(int& x, int& y) {
  get_cursor_position(x, y);
  x /= m_tile_width;
  y /= m_tile_height;
}
