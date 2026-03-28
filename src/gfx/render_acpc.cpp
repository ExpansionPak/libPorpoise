#include "render.hpp"
#include <dolphin/vi.h>

extern "C" {
void pc_gx_begin_frame(void);
extern int g_pc_window_w;
extern int g_pc_window_h;
}

namespace porpoise::gfx {

Range push_verts(const uint8_t* data, size_t length) {
  (void)data;
  (void)length;
  return {0, 0};
}

Range push_indices(const uint8_t* data, size_t length) {
  (void)data;
  (void)length;
  return {0, 0};
}

void push_draw_command(const DrawData& data) { (void)data; }

void begin_frame() {
  int w = 640;
  int h = 480;
  VIGetWindowSize(&w, &h);
  g_pc_window_w = w;
  g_pc_window_h = h;
  pc_gx_begin_frame();
}

void render() {}

bool did_render_with_bridge() { return false; }

void flush_render_if_pending() {}

void render_before_copy() {}

} // namespace porpoise::gfx
