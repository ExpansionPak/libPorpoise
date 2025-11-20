#include "GXStubUtils.h"

#include <dolphin/gx/GXCull.h>
#include "../gfx/gx_state.h"
#include "../gfx/gx_update_state.h"

void GXSetScissor(u32 left, u32 top, u32 width, u32 height) {
    /* Copy from Aurora exactly: GXSetScissor calls aurora::gfx::set_scissor */
    /* Aurora's code: aurora::gfx::set_scissor(left, top, width, height); */
    gfx_set_scissor(left, top, width, height);
}

void GXSetScissorBoxOffset(s32 xOffset, s32 yOffset) {
    GX_UNUSED(xOffset);
    GX_UNUSED(yOffset);
    GX_STUB_NOTICE("GXSetScissorBoxOffset");
}

void GXSetCullMode(GXCullMode mode) {
    /* Copy from Aurora exactly: GXSetCullMode calls update_gx_state */
    /* Aurora's code: update_gx_state(g_gxState.cullMode, mode); */
    update_gx_state(g_gxState->cullMode, (u32)mode);
}

void GXSetClipMode(GXClipMode mode) {
    GX_UNUSED(mode);
    GX_STUB_NOTICE("GXSetClipMode");
}

void GXSetCoPlanar(GXBool enable) {
    GX_UNUSED(enable);
    GX_STUB_NOTICE("GXSetCoPlanar");
}


