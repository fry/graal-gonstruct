#include "ogl_tiles_display.hpp"
#include "helper.hpp"

using namespace Graal::level_editor;

#ifdef DEBUG
  Glib::Timer g_timer;
  unsigned int g_frames = 0;
#endif

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

  int attrlist[] = {
    GDK_GL_RGBA,
    GDK_GL_DOUBLEBUFFER,
    //GDK_GL_DEPTH_SIZE, 1,
    GDK_GL_NONE
  };

  create_gl_area(attrlist);

  add_events(Gdk::VISIBILITY_NOTIFY_MASK);
}

bool ogl_tiles_display::on_configure_event(GdkEventConfigure* event) {
  if (!make_current())
    return false;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, get_width(), get_height(), 0, -1, 1);

  glViewport(0, 0, get_width(), get_height());

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
  
  set_surface_buffers();

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

void ogl_tiles_display::draw_tile(tile& _tile, int x, int y) {
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

void ogl_tiles_display::set_surface_buffers() {
  const tile_buf& buf = get_tile_buf();
  // Resize to fit the level
  set_size_request(
    buf.get_width() * m_tile_width,
    buf.get_height() * m_tile_height
  );

  invalidate();
}

void ogl_tiles_display::clear() {
  get_tile_buf().clear();
  set_surface_buffers();
}

void ogl_tiles_display::set_tile_buf(tile_buf& buf) {
  m_tile_buf.swap(buf);
  set_surface_buffers();
}
