#include "ogl_tiles_display.hpp"
#include "helper.hpp"

using namespace Graal::level_editor;

Glib::Timer g_timer;
unsigned int g_frames;

unsigned int Graal::level_editor::load_texture_from_surface(Cairo::RefPtr<Cairo::ImageSurface>& surface, unsigned int id) {
  glEnable(GL_TEXTURE_2D);

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

  return id;
}

ogl_tiles_display::ogl_tiles_display():
  m_tileset(0), m_tile_width(16), m_tile_height(16) {
  Glib::RefPtr<Gdk::GL::Config> glconfig =
    Gdk::GL::Config::create(Gdk::GL::MODE_RGB    |
                            Gdk::GL::MODE_DEPTH  |
                            Gdk::GL::MODE_DOUBLE);
  set_gl_capability(glconfig);

  add_events(Gdk::VISIBILITY_NOTIFY_MASK);

  g_frames = 0;
}

bool ogl_tiles_display::on_configure_event(GdkEventConfigure* event) {
  Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();

  if (!glwindow->gl_begin(get_gl_context()))
    return false;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, get_width(), get_height(), 0, -1, 1);
  glViewport(0, 0, get_width(), get_height());

  glwindow->gl_end();

  return true;
}

void ogl_tiles_display::on_realize() {
  Gtk::GL::DrawingArea::on_realize();
  Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();

  if (!glwindow->gl_begin(get_gl_context()))
    return;

  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  glDisable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 1.0, 1.0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  set_surface_buffers();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, get_width(), get_height(), 0, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glewInit();

  glwindow->gl_end();

  g_timer.start();
}

void ogl_tiles_display::draw_all() {
  tile_buf& buf = get_tile_buf();
  const int width = buf.get_width();
  const int height = buf.get_height();

  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      draw_tile(buf.get_tile(x, y), x, y);
    }
  }
}

void ogl_tiles_display::draw_tile(tile& _tile, int x, int y) {
  // The position of the actual tile inside the tileset
  const int tx = helper::get_tile_x(_tile.index);
  const int ty = helper::get_tile_y(_tile.index);

  // Build texture coordinates
  float x1 = (float)(tx * m_tile_width)/m_tileset_width;
  float x2 = (float)((tx+1)*m_tile_width)/m_tileset_width;
  float y1 = (float)(ty*m_tile_height)/m_tileset_height;
  float y2 = (float)((ty+1)*m_tile_height)/m_tileset_height;

  glPushMatrix();
  glTranslatef(x * m_tile_width, y * m_tile_height, 0);
  glBegin(GL_QUADS);
    // Top left
    glTexCoord2f(x1, y1);
    glVertex2f(0, 0);
    // Top right
    glTexCoord2f(x2, y1);
    glVertex2f(m_tile_width, 0);
    // Bottom right
    glTexCoord2f(x2, y2);
    glVertex2f(m_tile_width, m_tile_height);
    // Bottom left
    glTexCoord2f(x1, y2);
    glVertex2f(0, m_tile_height);
  glEnd();
  glPopMatrix();
}

void ogl_tiles_display::load_tileset(Cairo::RefPtr<Cairo::ImageSurface>& surface) {
  glEnable(GL_TEXTURE_2D);

  m_tileset = load_texture_from_surface(surface, m_tileset);

  m_tileset_width = surface->get_width();
  m_tileset_height = surface->get_height();
}

bool ogl_tiles_display::on_expose_event(GdkEventExpose* event) {
  Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();

  if (m_new_tileset) {
    load_tileset(m_new_tileset);
    m_new_tileset.clear();
  }

  if (!m_tileset)
    return false;

  if (!glwindow->gl_begin(get_gl_context()))
    return false;

  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, m_tileset);

  // Draw all tiles
  draw_all();

  if (glwindow->is_double_buffered())
    glwindow->swap_buffers();
  else
    glFlush();

  glwindow->gl_end();

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
  return true;
}

bool ogl_tiles_display::on_map_event(GdkEventAny* event) {
  if (!m_connection_idle.connected()) {
    m_connection_idle = Glib::signal_idle().connect(sigc::mem_fun(*this, &ogl_tiles_display::on_idle), GDK_PRIORITY_REDRAW);
  }

  return true;
}

bool ogl_tiles_display::on_unmap_event(GdkEventAny* event) {
  if (m_connection_idle.connected()) {
    m_connection_idle.disconnect();
  }

  return true;
}

bool ogl_tiles_display::on_visibility_notify_event(GdkEventVisibility* event) {
  if (event->state == GDK_VISIBILITY_FULLY_OBSCURED) {
    if (m_connection_idle.connected())
      m_connection_idle.disconnect();
  } else {
    if (!m_connection_idle.connected()) {
      m_connection_idle = Glib::signal_idle().connect(sigc::mem_fun(*this, &ogl_tiles_display::on_idle), GDK_PRIORITY_REDRAW);
    }
  }

  return true;
}

bool ogl_tiles_display::on_idle() {
  if (get_window()->is_visible()) {
    get_window()->invalidate_rect(get_allocation(), false);
    get_window()->process_updates(false);
  }

  return true;
}

void ogl_tiles_display::set_tile_size(int tile_width, int tile_height) {
  m_tile_width = tile_width;
  m_tile_height = tile_height;
}

void ogl_tiles_display::set_tileset_surface(
    const Cairo::RefPtr<Cairo::ImageSurface>& surface) {
  m_new_tileset = surface;
}

void ogl_tiles_display::set_surface_buffers() {
  const tile_buf& buf = get_tile_buf();
  // Resize to fit the level
  set_size_request(
    buf.get_width() * m_tile_width,
    buf.get_height() * m_tile_height
  );

  glViewport(0, 0, get_width(), get_height());
}

void ogl_tiles_display::clear() {
  get_tile_buf().clear();
  set_surface_buffers();
}

void ogl_tiles_display::set_tile_buf(tile_buf& buf) {
  m_tile_buf.swap(buf);
  set_surface_buffers();
}
