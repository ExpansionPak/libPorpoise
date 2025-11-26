#include "gx.hpp"

#include "../gfx/texture.hpp"
#include "../gfx/gx_state.hpp"
#include "../gfx/window.hpp"
#include <dolphin/vi.h>

namespace {

constexpr u32 kMaxXfbLines = 1024;
constexpr u32 kYScaleMask = 0x1FF;
constexpr f32 kYScaleRegisterScale = 256.0f;

u16 s_dispCopyWidth = 0;
u16 s_dispCopyHeight = 0;

u32 __GXGetNumXfbLines(u32 efbHeight, u32 yScaleReg) {
  if (yScaleReg == 0) {
    return efbHeight;
  }
  const u32 numerator = efbHeight * static_cast<u32>(kYScaleRegisterScale);
  return (numerator + yScaleReg - 1u) / yScaleReg;
}

u32 packYScaleRegister(f32 yScale) {
  if (yScale <= 0.0f) {
    return kYScaleMask;
  }
  const auto raw = static_cast<u32>(kYScaleRegisterScale / yScale);
  u32 reg = raw & kYScaleMask;
  if (reg == 0) {
    reg = 1;
  }
  return reg;
}

} // namespace

extern "C" {
GXRenderModeObj GXNtsc480IntDf = {
    VI_TVMODE_NTSC_INT, 640, 480, 480, 40, 0, 640, 480, VI_XFBMODE_DF, 0, 0,
};
GXRenderModeObj GXPal528IntDf = {
    VI_TVMODE_PAL_INT, 704, 528, 480, 40, 0, 640, 480, VI_XFBMODE_DF, 0, 0,
};
GXRenderModeObj GXMpal480IntDf = {
    VI_TVMODE_PAL_INT, 640, 480, 480, 40, 0, 640, 480, VI_XFBMODE_DF, 0, 0,
};


void GXAdjustForOverscan(GXRenderModeObj* rmin, GXRenderModeObj* rmout, u16 hor, u16 ver) {
  *rmout = *rmin;
  const auto size = aurora::window::get_window_size();
  rmout->fbWidth = size.fb_width;
  rmout->efbHeight = size.fb_height;
  rmout->xfbHeight = size.fb_height;
}

void GXSetDispCopySrc(u16 left, u16 top, u16 wd, u16 ht) {
  (void)left;
  (void)top;
  s_dispCopyWidth = wd;
  s_dispCopyHeight = ht;
}

void GXSetTexCopySrc(u16 left, u16 top, u16 wd, u16 ht) { g_gxState.texCopySrc = {left, top, wd, ht}; }

void GXSetDispCopyDst(u16 wd, u16 ht) {}

void GXSetTexCopyDst(u16 wd, u16 ht, GXTexFmt fmt, GXBool mipmap) {
  // TODO texture copy scaling (mipmap)
  g_gxState.texCopyFmt = fmt;
}

// TODO GXSetDispCopyFrame2Field
// TODO GXSetCopyClamp

u32 GXSetDispCopyYScale(f32 vscale) {
  if (vscale <= 0.0f || s_dispCopyHeight == 0) {
    return 0;
  }
  const u32 reg = packYScaleRegister(vscale);
  return __GXGetNumXfbLines(s_dispCopyHeight, reg);
}

void GXSetCopyClear(GXColor color, u32 depth) { update_gx_state(g_gxState.clearColor, from_gx_color(color)); }

void GXSetCopyFilter(GXBool aa, u8 sample_pattern[12][2], GXBool vf, u8 vfilter[7]) {}

void GXSetDispCopyGamma(GXGamma gamma) {}

void GXCopyDisp(void* dest, GXBool clear) {}

void GXCopyTex(void* dest, GXBool clear) {
  const auto& rect = g_gxState.texCopySrc;
  porpoise::gfx::TextureHandle* handle = nullptr;
  const auto it = g_gxState.copyTextures.find(dest);
  if (it == g_gxState.copyTextures.end() || 
      it->second->width != rect.width || 
      it->second->height != rect.height) {
    handle = aurora::gfx::new_render_texture(rect.width, rect.height, g_gxState.texCopyFmt, "Resolved Texture");
    g_gxState.copyTextures[dest] = handle;
  } else {
    handle = it->second;
  }
  aurora::gfx::resolve_pass(*handle, rect, clear, g_gxState.clearColor);
}

f32 GXGetYScaleFactor(u16 efbHeight, u16 xfbHeight) {
  ASSERT(xfbHeight <= kMaxXfbLines,
         "GXGetYScaleFactor: Display copy only supports up to 1024 lines.");
  ASSERT(efbHeight <= xfbHeight,
         "GXGetYScaleFactor: EFB height should not be greater than XFB height.");

  if (efbHeight == 0 || xfbHeight == 0) {
    return 1.0f;
  }

  u32 tgtHt = xfbHeight;
  f32 yScale = static_cast<f32>(tgtHt) / static_cast<f32>(efbHeight);
  u32 iScale = packYScaleRegister(yScale);
  u32 realHt = __GXGetNumXfbLines(efbHeight, iScale);

  while (realHt > xfbHeight && tgtHt > 0) {
    --tgtHt;
    yScale = static_cast<f32>(tgtHt) / static_cast<f32>(efbHeight);
    iScale = packYScaleRegister(yScale);
    realHt = __GXGetNumXfbLines(efbHeight, iScale);
  }

  f32 fScale = yScale;
  while (realHt < xfbHeight) {
    fScale = yScale;
    ++tgtHt;
    yScale = static_cast<f32>(tgtHt) / static_cast<f32>(efbHeight);
    iScale = packYScaleRegister(yScale);
    realHt = __GXGetNumXfbLines(efbHeight, iScale);
  }

  return fScale;
}

// TODO GXGetNumXfbLines
// TODO GXClearBoundingBox
// TODO GXReadBoundingBox
}