#include "gx.hpp"

#include "../gfx/render.hpp"
#include "../gfx/texture.hpp"
#include "../gfx/gx_state.hpp"
#include "../gfx/window.hpp"
#include <dolphin/os.h>
#include <dolphin/vi.h>

// OpenGL includes
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <cstring>
#include <vector>

namespace {

constexpr u32 kMaxXfbLines = 1024;
constexpr u32 kYScaleMask = 0x1FF;
constexpr f32 kYScaleRegisterScale = 256.0f;

u16 s_dispCopyLeft = 0;
u16 s_dispCopyTop = 0;
u16 s_dispCopyWidth = 0;
u16 s_dispCopyHeight = 0;
u16 s_dispCopyDstWidth = 0;
u16 s_dispCopyDstHeight = 0;
GXCopyMode s_dispCopyMode = GX_COPY_PROGRESSIVE;
GXGamma s_dispCopyGamma = GX_GM_1_0;
GXFBClamp s_copyClamp = static_cast<GXFBClamp>(GX_CLAMP_TOP | GX_CLAMP_BOTTOM);
u16 s_bboxLeft = 0;
u16 s_bboxTop = 0;
u16 s_bboxRight = 0;
u16 s_bboxBottom = 0;
bool s_bboxValid = false;

u32 __GXGetNumXfbLines(u32 efbHeight, u32 yScaleReg) {
  if (yScaleReg == 0) {
    return efbHeight;
  }

  u32 count = (efbHeight - 1u) * static_cast<u32>(kYScaleRegisterScale);
  u32 realHeight = count / yScaleReg + 1u;

  u32 scaleDown = yScaleReg;
  if (scaleDown > 0x80 && scaleDown < 0x100) {
    while ((scaleDown & 0x1u) == 0u) {
      scaleDown >>= 1u;
    }
    if ((efbHeight % scaleDown) == 0u) {
      ++realHeight;
    }
  }

  return (std::min)(realHeight, kMaxXfbLines);
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
GXRenderModeObj GXNtsc480IntAa = {
    VI_TVMODE_NTSC_INT, 640, 480, 480, 40, 0, 640, 480, VI_XFBMODE_DF, 0, 1,
};
GXRenderModeObj GXPal528IntDf = {
    VI_TVMODE_PAL_INT, 704, 528, 480, 40, 0, 640, 480, VI_XFBMODE_DF, 0, 0,
};
GXRenderModeObj GXMpal480IntDf = {
    VI_TVMODE_PAL_INT, 640, 480, 480, 40, 0, 640, 480, VI_XFBMODE_DF, 0, 0,
};


void GXAdjustForOverscan(GXRenderModeObj* rmin, GXRenderModeObj* rmout, u16 hor, u16 ver) {
  if (!rmin || !rmout) {
    return;
  }
  if (rmin != rmout) {
    *rmout = *rmin;
  }

  const u16 hor2 = static_cast<u16>(hor * 2);
  const u16 ver2 = static_cast<u16>(ver * 2);
  const u32 verf = (static_cast<u32>(ver2) * static_cast<u32>(rmin->efbHeight)) /
                   static_cast<u32>(rmin->xfbHeight == 0 ? 1 : rmin->xfbHeight);

  rmout->fbWidth = static_cast<u16>(rmin->fbWidth - hor2);
  rmout->efbHeight = static_cast<u16>(rmin->efbHeight - verf);
  if (rmin->xFBmode == VI_XFBMODE_SF && (rmin->viTVmode & 2) != 2) {
    rmout->xfbHeight = static_cast<u16>(rmin->xfbHeight - ver);
  } else {
    rmout->xfbHeight = static_cast<u16>(rmin->xfbHeight - ver2);
  }
  rmout->viWidth = static_cast<u16>(rmin->viWidth - hor2);
  rmout->viHeight = static_cast<u16>(rmin->viHeight - ver2);
  rmout->viXOrigin = static_cast<u16>(rmin->viXOrigin + hor);
  rmout->viYOrigin = static_cast<u16>(rmin->viYOrigin + ver);
}

void GXSetDispCopySrc(u16 left, u16 top, u16 wd, u16 ht) {
  s_dispCopyLeft = left;
  s_dispCopyTop = top;
  s_dispCopyWidth = wd;
  s_dispCopyHeight = ht;
}

void GXSetTexCopySrc(u16 left, u16 top, u16 wd, u16 ht) { g_gxState().texCopySrc = {left, top, wd, ht}; }

void GXSetDispCopyDst(u16 wd, u16 ht) {
  s_dispCopyDstWidth = wd;
  s_dispCopyDstHeight = ht;
}

void GXSetTexCopyDst(u16 wd, u16 ht, GXTexFmt fmt, GXBool mipmap) {
  // TODO texture copy scaling (mipmap)
  (void)wd;
  (void)ht;
  (void)mipmap;
  g_gxState().texCopyFmt = fmt;
}

void GXSetDispCopyFrame2Field(GXCopyMode mode) { s_dispCopyMode = mode; }

void GXSetCopyClamp(GXFBClamp clamp) {
  s_copyClamp = clamp;
}

u32 GXSetDispCopyYScale(f32 vscale) {
  if (vscale <= 0.0f || s_dispCopyHeight == 0) {
    return 0;
  }
  const u32 reg = packYScaleRegister(vscale);
  return __GXGetNumXfbLines(s_dispCopyHeight, reg);
}

void GXSetCopyClear(GXColor color, u32 depth) { update_gx_state(g_gxState().clearColor, from_gx_color(color)); }

void GXSetCopyFilter(GXBool aa, u8 sample_pattern[12][2], GXBool vf, u8 vfilter[7]) {
  (void)aa;
  (void)sample_pattern;
  (void)vf;
  (void)vfilter;
}

void GXSetDispCopyGamma(GXGamma gamma) { s_dispCopyGamma = gamma; }

// Convert RGBA8 to RGB565
static inline u16 rgba8_to_rgb565(u8 r, u8 g, u8 b) {
    // RGB565: RRRRR GGGGGG BBBBB
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

void GXCopyDisp(void* dest, GXBool clear) {
    if (!dest) return;

    // Render pending draws before copy (frb_aa_full calls GXCopyDisp without DEMODoneRender)
    porpoise::gfx::render_before_copy();

    // Get copy region (set by GXSetDispCopySrc)
    u16 left = s_dispCopyLeft;
    u16 top = s_dispCopyTop;
    u16 width = s_dispCopyWidth;
    u16 height = s_dispCopyHeight;

    if (width == 0 || height == 0) {
        const auto windowSize = porpoise::window::get_window_size();
        width = windowSize.fb_width;
        height = windowSize.fb_height;
        left = 0;
        top = 0;
    }
    if (s_dispCopyDstWidth != 0) {
      width = s_dispCopyDstWidth;
    }
    if (s_dispCopyDstHeight != 0) {
      height = s_dispCopyDstHeight;
    }

    {
    // OpenGL path: read region from framebuffer
    // GX uses top-left origin; OpenGL uses bottom-left - convert Y
    const auto windowSize = porpoise::window::get_window_size();
    GLint glX = static_cast<GLint>(left);
    GLint glY = static_cast<GLint>(windowSize.fb_height) - static_cast<GLint>(top) - static_cast<GLint>(height);

    std::vector<u8> rgbaBuffer(static_cast<size_t>(width) * height * 4);
    glReadPixels(glX, glY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgbaBuffer.data());

    // Convert RGBA8 to RGB565, flip Y (OpenGL row 0 = bottom, GX row 0 = top)
    u16* dest16 = static_cast<u16*>(dest);
    for (u16 y = 0; y < height; ++y) {
        u16 srcY = height - 1 - y;
        const u8* srcRow = rgbaBuffer.data() + (static_cast<size_t>(srcY) * width * 4);
        u16* destRow = dest16 + (y * width);
        for (u16 x = 0; x < width; ++x) {
            u8 r = srcRow[x * 4 + 0];
            u8 g = srcRow[x * 4 + 1];
            u8 b = srcRow[x * 4 + 2];
            destRow[x] = rgba8_to_rgb565(r, g, b);
        }
    }
    }

    // Clear AFTER copy. On real GX this resets the EFB for the next pass.
    // For frb_aa_full we must NOT clear: pass 2 draws the bottom half (scissored) on top of
    // the existing buffer, and we need the full composite when we swap. Clearing would wipe
    // the top half. Skip clear for now so full-frame AA shows correctly.
    (void)clear;

    s_bboxLeft = left;
    s_bboxTop = top;
    s_bboxRight = static_cast<u16>(left + width - 1);
    s_bboxBottom = static_cast<u16>(top + height - 1);
    s_bboxValid = true;
}

void GXCopyTex(void* dest, GXBool clear) {
  const auto& rect = g_gxState().texCopySrc;
  porpoise::gfx::TextureHandle* handle = nullptr;
  const auto it = g_gxState().copyTextures.find(dest);
  if (it == g_gxState().copyTextures.end() || 
      it->second->width != rect.width || 
      it->second->height != rect.height) {
    handle = porpoise::gfx::new_render_texture(rect.width, rect.height, g_gxState().texCopyFmt, "Resolved Texture");
    g_gxState().copyTextures[dest] = handle;
  } else {
    handle = it->second;
  }
  porpoise::gfx::resolve_pass(*handle, rect, clear, g_gxState().clearColor);
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

u16 GXGetNumXfbLines(u16 efbHeight, f32 yScale) {
  ASSERT(yScale >= 1.0f, "GXGetNumXfbLines: Vertical scale must be >= 1.0");
  if (efbHeight == 0) {
    return 0;
  }

  const u32 reg = packYScaleRegister(yScale);
  return static_cast<u16>(__GXGetNumXfbLines(efbHeight, reg));
}

void GXClearBoundingBox(void) {
  s_bboxLeft = 0;
  s_bboxTop = 0;
  s_bboxRight = 0;
  s_bboxBottom = 0;
  s_bboxValid = false;
}

void GXReadBoundingBox(u16* left, u16* top, u16* right, u16* bottom) {
  if (!s_bboxValid) {
    if (left) *left = 0;
    if (top) *top = 0;
    if (right) *right = 0;
    if (bottom) *bottom = 0;
    return;
  }

  if (left) *left = s_bboxLeft;
  if (top) *top = s_bboxTop;
  if (right) *right = s_bboxRight;
  if (bottom) *bottom = s_bboxBottom;
}
}
