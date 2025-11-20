#include "GXStubUtils.h"

#include <dolphin/gx/GXFrameBuffer.h>
#include "../gfx/gx_state.h"
#include "../gfx/gx_state_helpers.h"
#include "../gfx/gx_update_state.h"

void GXAdjustForOverscan(GXRenderModeObj* rmin, GXRenderModeObj* rmout, u16 hor, u16 ver) {
    GX_UNUSED(rmin);
    GX_UNUSED(rmout);
    GX_UNUSED(hor);
    GX_UNUSED(ver);
    GX_STUB_NOTICE("GXAdjustForOverscan");
}

void GXSetDispCopySrc(u16 left, u16 top, u16 wd, u16 ht) {
    GX_UNUSED(left);
    GX_UNUSED(top);
    GX_UNUSED(wd);
    GX_UNUSED(ht);
    GX_STUB_NOTICE("GXSetDispCopySrc");
}

void GXSetTexCopySrc(u16 left, u16 top, u16 wd, u16 ht) {
    GX_UNUSED(left);
    GX_UNUSED(top);
    GX_UNUSED(wd);
    GX_UNUSED(ht);
    GX_STUB_NOTICE("GXSetTexCopySrc");
}

void GXSetDispCopyDst(u16 wd, u16 ht) {
    GX_UNUSED(wd);
    GX_UNUSED(ht);
    GX_STUB_NOTICE("GXSetDispCopyDst");
}

void GXSetTexCopyDst(u16 wd, u16 ht, GXTexFmt fmt, GXBool mipmap) {
    GX_UNUSED(wd);
    GX_UNUSED(ht);
    GX_UNUSED(fmt);
    GX_UNUSED(mipmap);
    GX_STUB_NOTICE("GXSetTexCopyDst");
}

void GXSetCopyClear(GXColor color, u32 depth) {
    GX_UNUSED(depth);
    
    /* Copy from Aurora exactly: GXSetCopyClear calls update_gx_state with from_gx_color */
    /* Aurora's code: update_gx_state(g_gxState.clearColor, from_gx_color(color)); */
    GXColorF32 clearColor = gx_from_gx_color(color);
    update_gx_state(g_gxState->clearColor, clearColor);
}

void GXSetCopyFilter(GXBool aa, u8 sample_pattern[12][2], GXBool vf, u8 vfilter[7]) {
    GX_UNUSED(aa);
    GX_UNUSED(sample_pattern);
    GX_UNUSED(vf);
    GX_UNUSED(vfilter);
    GX_STUB_NOTICE("GXSetCopyFilter");
}

void GXSetDispCopyGamma(GXGamma gamma) {
    GX_UNUSED(gamma);
    GX_STUB_NOTICE("GXSetDispCopyGamma");
}

u32 GXSetDispCopyYScale(f32 vscale) {
    GX_UNUSED(vscale);
    GX_STUB_NOTICE("GXSetDispCopyYScale");
    return 0;
}

void GXSetPixelFmt(GXPixelFmt pix_fmt, GXZFmt16 z_fmt) {
    GX_UNUSED(pix_fmt);
    GX_UNUSED(z_fmt);
    GX_STUB_NOTICE("GXSetPixelFmt");
}

void GXCopyDisp(void* dest, GXBool clear) {
    /* Copy from Aurora exactly: GXCopyDisp is a no-op stub */
    /* Aurora's code: void GXCopyDisp(void* dest, GXBool clear) {} */
    (void)dest;
    (void)clear;
    /* On GameCube: Copies EFB to XFB in main memory for VI display */
    /* On PC: No-op since we use OpenGL/SDL directly (SDL_GL_SwapWindow handles display) */
}

void GXCopyTex(void* dest, GXBool clear) {
    GX_UNUSED(dest);
    GX_UNUSED(clear);
    GX_STUB_NOTICE("GXCopyTex");
}


