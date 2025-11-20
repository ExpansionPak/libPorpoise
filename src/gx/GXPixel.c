#include "GXStubUtils.h"

#include <dolphin/gx/GXPixel.h>
#include "../gfx/gx_state.h"
#include "../gfx/gx_update_state.h"  // For update_gx_state macro
#include "../gfx/gx_state_helpers.h"
#include "../gfx/gx_gl.h"
#include <dolphin/gx/GXStruct.h>

void GXSetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ, GXColor color) {
    /* Copy from Aurora exactly: GXSetFog calls update_gx_state for the entire fog state */
    /* Aurora's code: update_gx_state(g_gxState.fog, {type, startZ, endZ, nearZ, farZ, from_gx_color(color)}); */
    
    /* Update each fog field individually (C doesn't support struct initialization in update_gx_state) */
    update_gx_state(g_gxState->fogType, (u32)type);
    update_gx_state(g_gxState->fogStartZ, startZ);
    update_gx_state(g_gxState->fogEndZ, endZ);
    update_gx_state(g_gxState->fogNearZ, nearZ);
    update_gx_state(g_gxState->fogFarZ, farZ);
    
    /* Convert GXColor to GXColorF32 and update fog color */
    GXColorF32 fogColor = gx_from_gx_color(color);
    update_gx_state(g_gxState->fogColor, fogColor);
}

void GXSetFogColor(GXColor color) {
    GX_UNUSED(color);
    GX_STUB_NOTICE("GXSetFogColor");
}

void GXSetBlendMode(GXBlendMode mode, GXBlendFactor src, GXBlendFactor dst, GXLogicOp op) {
    /* Copy from Aurora exactly: GXSetBlendMode calls update_gx_state for each parameter */
    /* Aurora's code:
     *   update_gx_state(g_gxState.blendMode, mode);
     *   update_gx_state(g_gxState.blendFacSrc, src);
     *   update_gx_state(g_gxState.blendFacDst, dst);
     *   update_gx_state(g_gxState.blendOp, op);
     */
    update_gx_state(g_gxState->blendMode, (u32)mode);
    update_gx_state(g_gxState->blendFacSrc, (u32)src);
    update_gx_state(g_gxState->blendFacDst, (u32)dst);
    update_gx_state(g_gxState->blendOp, (u32)op);
    
    /* Also apply to OpenGL immediately */
    if (mode == GX_BM_NONE) {
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        /* TODO: Map GXBlendFactor and GXLogicOp to OpenGL blend functions */
        /* For now, use a default blend func */
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void GXSetColorUpdate(GXBool enabled) {
    /* Copy from Aurora exactly: GXSetColorUpdate uses update_gx_state */
    /* Aurora's code: update_gx_state(g_gxState.colorUpdate, enabled); */
    update_gx_state(g_gxState->colorUpdate, enabled);
}

void GXSetAlphaUpdate(GXBool enabled) {
    GX_UNUSED(enabled);
    GX_STUB_NOTICE("GXSetAlphaUpdate");
}

void GXSetZMode(GXBool compareEnable, GXCompare func, GXBool updateEnable) {
    /* Copy from Aurora exactly: GXSetZMode uses update_gx_state for each parameter */
    /* Aurora's code:
     *   update_gx_state(g_gxState.depthCompare, compare_enable);
     *   update_gx_state(g_gxState.depthFunc, func);
     *   update_gx_state(g_gxState.depthUpdate, update_enable);
     */
    update_gx_state(g_gxState->depthCompare, compareEnable);
    update_gx_state(g_gxState->depthFunc, (u32)func);
    update_gx_state(g_gxState->depthUpdate, updateEnable);
}

void GXSetZCompLoc(GXBool before_tex) {
    GX_UNUSED(before_tex);
    GX_STUB_NOTICE("GXSetZCompLoc");
}

void GXSetDither(GXBool enable) {
    GX_UNUSED(enable);
    GX_STUB_NOTICE("GXSetDither");
}

void GXSetDstAlpha(GXBool enable, u8 value) {
    GX_UNUSED(enable);
    GX_UNUSED(value);
    GX_STUB_NOTICE("GXSetDstAlpha");
}

void GXSetFieldMask(GXBool even_mask, GXBool odd_mask) {
    GX_UNUSED(even_mask);
    GX_UNUSED(odd_mask);
    GX_STUB_NOTICE("GXSetFieldMask");
}

void GXSetFieldMode(GXBool line, GXBool field) {
    GX_UNUSED(line);
    GX_UNUSED(field);
    GX_STUB_NOTICE("GXSetFieldMode");
}

void GXSetAlphaCompare(GXCompare comp0, u8 ref0, GXAlphaOp op, GXCompare comp1, u8 ref1) {
    GX_UNUSED(comp0);
    GX_UNUSED(ref0);
    GX_UNUSED(op);
    GX_UNUSED(comp1);
    GX_UNUSED(ref1);
    GX_STUB_NOTICE("GXSetAlphaCompare");
}

void GXSetLogicOp(GXLogicOp op) {
    GX_UNUSED(op);
    GX_STUB_NOTICE("GXSetLogicOp");
}


