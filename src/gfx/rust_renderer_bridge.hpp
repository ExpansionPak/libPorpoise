#pragma once

#include "gx_state.hpp"
#include "render.hpp"

#include <dolphin/types.h>

#include <vector>

namespace porpoise::gfx::bridge {

enum class Action : u32 {
  StateGeneric = 0,
  Pixel = 1,
  Tev = 2,
  Texture = 3,
  Transform = 4,
  Lighting = 5,
  Draw = 6,
  Copy = 7,
};

void notify_state(Action action);
bool begin_frame(const gx::GXState& state);
void push_draw_command(const DrawData& data);

bool render(const std::vector<u8>& vertex_buffer,
            const std::vector<u8>& index_buffer,
            const gx::GXState& state);

bool copy_disp(void* dest, u16 left, u16 top, u16 width, u16 height, GXBool clear, const gx::GXState& state);
bool copy_tex(void* dest, const ClipRect& rect, GXTexFmt fmt, GXBool clear, const gx::GXState& state);
const char* last_status();

} // namespace porpoise::gfx::bridge
