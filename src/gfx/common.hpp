#pragma once

#include <dolphin/types.h>

// Common graphics functions for libPorpoise
// Shared OpenGL/rendering functionality

namespace porpoise::gfx {

// Viewport management
// Sets the viewport for rendering using OpenGL
void set_viewport(float left, float top, float width, float height, float nearZ, float farZ);

// Scissor (GX top-left origin); pass 0,0,0,0 to disable
void set_scissor(u32 left, u32 top, u32 wd, u32 ht);

} // namespace porpoise::gfx
