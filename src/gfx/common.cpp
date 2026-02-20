#include "common.hpp"
#include "window.hpp"

// OpenGL includes
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace porpoise::gfx {

void set_viewport(float left, float top, float width, float height, float nearZ, float farZ) {
  // Set OpenGL viewport
  // Note: OpenGL uses bottom-left origin, but GX uses top-left
  // We need to flip the Y coordinate to convert from GX (top-left) to OpenGL (bottom-left)
  const auto windowSize = porpoise::window::get_window_size();
  const GLint x = static_cast<GLint>(left);
  const GLint y = static_cast<GLint>(windowSize.fb_height - top - height); // Flip Y coordinate
  const GLsizei w = static_cast<GLsizei>(width);
  const GLsizei h = static_cast<GLsizei>(height);
  
  glViewport(x, y, w, h);
  
  // Set depth range (OpenGL uses [0, 1] by default, but we support custom ranges)
  glDepthRange(static_cast<GLclampd>(nearZ), static_cast<GLclampd>(farZ));
}

void set_scissor(u32 left, u32 top, u32 wd, u32 ht) {
  if (wd == 0 || ht == 0) {
    glDisable(GL_SCISSOR_TEST);
    return;
  }
  const auto windowSize = porpoise::window::get_window_size();
  const GLint x = static_cast<GLint>(left);
  const GLint y = static_cast<GLint>(windowSize.fb_height - top - ht);
  glScissor(x, y, static_cast<GLsizei>(wd), static_cast<GLsizei>(ht));
  glEnable(GL_SCISSOR_TEST);
}

} // namespace porpoise::gfx

