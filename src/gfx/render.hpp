#pragma once

// Deferred rendering system for libPorpoise
// Matches Aurora's deferred rendering approach but uses OpenGL

#include "internal.hpp"
#include <dolphin/gx.h>
#include <vector>
#include <cstdint>

namespace porpoise::gfx {

// Range represents an offset and size in a buffer
// Matches porpoise::Range from internal.hpp
using Range = porpoise::Range;

// Draw command data (simplified version of Aurora's DrawData)
struct DrawData {
  Range vertRange;
  Range idxRange;
  uint32_t indexCount = 0;
  GXPrimitive primitive = GX_TRIANGLES;
  GXVtxFmt vtxFmt = GX_VTXFMT0;
  uint32_t indexedStride = 0;
  uint32_t posIndexOffset = UINT32_MAX;
  uint32_t colorIndexOffset = UINT32_MAX;
  uint8_t posIndexSize = 0;
  uint8_t colorIndexSize = 0;
  uint8_t posArrayStride = 0;
  uint8_t colorArrayStride = 0;
  
  // Indexed attribute arrays (for GX_INDEX8/GX_INDEX16)
  std::array<Range, GX_VA_MAX_ATTR> arrayRanges{};
  
  // GX state at time of draw
  u32 dstAlpha = UINT32_MAX;
  float modelView[16]{};
};

// Push vertex data to staging buffer, returns range
Range push_verts(const uint8_t* data, size_t length);

// Push index data to staging buffer, returns range  
Range push_indices(const uint8_t* data, size_t length);

// Queue a draw command for later execution
void push_draw_command(const DrawData& data);

// Execute all queued draw commands (called from DEMODoneRender)
void render();
bool did_render_with_bridge();

// Clear all queued commands (called at start of frame)
void begin_frame();

} // namespace porpoise::gfx

