#pragma once

// Common graphics functions for libPorpoise
// Shared OpenGL/rendering functionality

namespace porpoise::gfx {

// Viewport management
// Sets the viewport for rendering using OpenGL
void set_viewport(float left, float top, float width, float height, float nearZ, float farZ);

} // namespace porpoise::gfx
