#include "gx.hpp"

#include <algorithm>

namespace {
inline void notify_pixel_state() {
}
} // namespace

extern "C" {
void GXSetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ, GXColor color) {
  update_gx_state(g_gxState().fog, {type, startZ, endZ, nearZ, farZ, from_gx_color(color)});
  notify_pixel_state();
}

void GXSetFogColor(GXColor color) {
  update_gx_state(g_gxState().fog.color, from_gx_color(color));
  notify_pixel_state();
}

void GXInitFogAdjTable(GXFogAdjTable* table, u16 width, const float projMtx[4][4]) {
  if (!table) {
    return;
  }

  constexpr size_t kEntries = 10;
  if (width == 0) {
    for (size_t i = 0; i < kEntries; ++i) {
      table->r[i] = 0;
    }
    return;
  }

  // Store a simple linear ramp so fog adjustments behave deterministically on PC.
  for (size_t i = 0; i < kEntries; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(kEntries - 1);
    const float value = t * static_cast<float>(width);
    table->r[i] = static_cast<u16>(std::clamp(value, 0.0f, static_cast<float>(width)));
  }
  (void)projMtx;
}

void GXSetFogRangeAdj(GXBool enable, u16 center, GXFogAdjTable* table) {
  g_gxState().fog.rangeAdjustEnabled = enable == GX_TRUE;
  g_gxState().fog.rangeAdjustCenter = center;

  if (table) {
    for (size_t i = 0; i < g_gxState().fog.rangeAdjustTable.size(); ++i) {
      g_gxState().fog.rangeAdjustTable[i] = table->r[i];
    }
  } else {
    g_gxState().fog.rangeAdjustTable.fill(0);
  }
  notify_pixel_state();
}

void GXSetBlendMode(GXBlendMode mode, GXBlendFactor src, GXBlendFactor dst, GXLogicOp op) {
  update_gx_state(g_gxState().blendMode, mode);
  update_gx_state(g_gxState().blendFacSrc, src);
  update_gx_state(g_gxState().blendFacDst, dst);
  update_gx_state(g_gxState().blendOp, op);
  notify_pixel_state();
}

void GXSetColorUpdate(GXBool enabled) {
  update_gx_state(g_gxState().colorUpdate, enabled);
  notify_pixel_state();
}

void GXSetAlphaUpdate(bool enabled) {
  update_gx_state(g_gxState().alphaUpdate, enabled);
  notify_pixel_state();
}

void GXSetZMode(bool compare_enable, GXCompare func, bool update_enable) {
  update_gx_state(g_gxState().depthCompare, compare_enable);
  update_gx_state(g_gxState().depthFunc, func);
  update_gx_state(g_gxState().depthUpdate, update_enable);
  notify_pixel_state();
}

void GXSetZCompLoc(GXBool before_tex) {
  // TODO
}

void GXSetPixelFmt(GXPixelFmt pix_fmt, GXZFmt16 z_fmt) {}

void GXSetDither(GXBool dither) {}

void GXSetDstAlpha(bool enabled, u8 value) {
  if (enabled) {
    update_gx_state<u32>(g_gxState().dstAlpha, value);
  } else {
    update_gx_state(g_gxState().dstAlpha, UINT32_MAX);
  }
  notify_pixel_state();
}

// TODO GXSetFieldMask
// TODO GXSetFieldMode
}
