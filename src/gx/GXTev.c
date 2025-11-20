#include "GXStubUtils.h"

#include <dolphin/gx/GXTev.h>
#include <dolphin/gx/GXEnum.h>  // For GX_TEVSTAGE0, GX_CC_*, GX_CA_*, etc.
#include "../gfx/gx_state.h"  // For GXState, TevPass, TevOp
#include "../gfx/gx_update_state.h"  // For update_gx_state

void GXSetTevOp(GXTevStageID id, GXTevMode mode) {
    /* Copy from Aurora exactly: GXSetTevOp sets up TEV stage based on mode */
    /* Aurora's code:
     *   GXTevColorArg inputColor = GX_CC_RASC;
     *   GXTevAlphaArg inputAlpha = GX_CA_RASA;
     *   if (id != GX_TEVSTAGE0) {
     *     inputColor = GX_CC_CPREV;
     *     inputAlpha = GX_CA_APREV;
     *   }
     *   switch (mode) { ... }
     *   GXSetTevColorOp(...);
     *   GXSetTevAlphaOp(...);
     */
    
    GXTevColorArg inputColor = GX_CC_RASC;
    GXTevAlphaArg inputAlpha = GX_CA_RASA;
    if (id != GX_TEVSTAGE0) {
        inputColor = GX_CC_CPREV;
        inputAlpha = GX_CA_APREV;
    }
    
    switch (mode) {
    case GX_MODULATE:
        GXSetTevColorIn(id, GX_CC_ZERO, GX_CC_TEXC, inputColor, GX_CC_ZERO);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_TEXA, inputAlpha, GX_CA_ZERO);
        break;
    case GX_DECAL:
        GXSetTevColorIn(id, inputColor, GX_CC_TEXC, GX_CC_TEXA, GX_CC_ZERO);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, inputAlpha);
        break;
    case GX_BLEND:
        GXSetTevColorIn(id, inputColor, GX_CC_ONE, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_TEXA, inputAlpha, GX_CA_ZERO);
        break;
    case GX_REPLACE:
        GXSetTevColorIn(id, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
        break;
    case GX_PASSCLR:
        GXSetTevColorIn(id, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, inputColor);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, inputAlpha);
        break;
    default:
        OSReport("[GX WARN] GXSetTevOp: Unknown mode %d\n", mode);
        break;
    }
    
    GXSetTevColorOp(id, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(id, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

void GXSetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c, GXTevColorArg d) {
    /* Copy from Aurora exactly: GXSetTevColorIn calls update_gx_state with TevPass struct */
    /* Aurora's code: update_gx_state(g_gxState.tevStages[stageId].colorPass, {a, b, c, d}); */
    if (stageId >= 0 && stageId < GX_MAX_TEVSTAGE) {
        TevPass colorPass = {.a = (u32)a, .b = (u32)b, .c = (u32)c, .d = (u32)d};
        update_gx_state(g_gxState->tevStages[stageId].colorPass, colorPass);
    } else {
        OSReport("[GX ERROR] GXSetTevColorIn: Invalid stage ID %d\n", stageId);
    }
}

void GXSetTevAlphaIn(GXTevStageID stageId, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c, GXTevAlphaArg d) {
    /* Copy from Aurora exactly: GXSetTevAlphaIn calls update_gx_state with TevPass struct */
    /* Aurora's code: update_gx_state(g_gxState.tevStages[stageId].alphaPass, {a, b, c, d}); */
    if (stageId >= 0 && stageId < GX_MAX_TEVSTAGE) {
        TevPass alphaPass = {.a = (u32)a, .b = (u32)b, .c = (u32)c, .d = (u32)d};
        update_gx_state(g_gxState->tevStages[stageId].alphaPass, alphaPass);
    } else {
        OSReport("[GX ERROR] GXSetTevAlphaIn: Invalid stage ID %d\n", stageId);
    }
}

void GXSetTevColorOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp,
                     GXTevRegID outReg) {
    /* Copy from Aurora exactly: GXSetTevColorOp calls update_gx_state with TevOp struct */
    /* Aurora's code: update_gx_state(g_gxState.tevStages[stageId].colorOp, {op, bias, scale, outReg, clamp}); */
    if (stageId >= 0 && stageId < GX_MAX_TEVSTAGE) {
        TevOp colorOp = {
            .op = (u32)op,
            .bias = (u32)bias,
            .scale = (u32)scale,
            .outReg = (u32)outReg,
            .clamp = clamp,
            ._p1 = 0,
            ._p2 = 0,
            ._p3 = 0
        };
        update_gx_state(g_gxState->tevStages[stageId].colorOp, colorOp);
    } else {
        OSReport("[GX ERROR] GXSetTevColorOp: Invalid stage ID %d\n", stageId);
    }
}

void GXSetTevAlphaOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp,
                     GXTevRegID outReg) {
    /* Copy from Aurora exactly: GXSetTevAlphaOp calls update_gx_state with TevOp struct */
    /* Aurora's code: update_gx_state(g_gxState.tevStages[stageId].alphaOp, {op, bias, scale, outReg, clamp}); */
    if (stageId >= 0 && stageId < GX_MAX_TEVSTAGE) {
        TevOp alphaOp = {
            .op = (u32)op,
            .bias = (u32)bias,
            .scale = (u32)scale,
            .outReg = (u32)outReg,
            .clamp = clamp,
            ._p1 = 0,
            ._p2 = 0,
            ._p3 = 0
        };
        update_gx_state(g_gxState->tevStages[stageId].alphaOp, alphaOp);
    } else {
        OSReport("[GX ERROR] GXSetTevAlphaOp: Invalid stage ID %d\n", stageId);
    }
}

void GXSetTevOrder(GXTevStageID id, GXTexCoordID tcid, GXTexMapID tmid, GXChannelID cid) {
    /* Copy from Aurora exactly: GXSetTevOrder uses update_gx_state for each field */
    /* Aurora's code:
     *   auto& stage = g_gxState.tevStages[id];
     *   update_gx_state(stage.texCoordId, tcid);
     *   update_gx_state(stage.texMapId, tmid);
     *   update_gx_state(stage.channelId, cid);
     */
    
    if (id >= GX_MAX_TEVSTAGE) {
        OSReport("[GX ERROR] GXSetTevOrder: Invalid stage id %u\n", id);
        return;
    }
    
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    
    TevStage* stage = &state->tevStages[id];
    update_gx_state(stage->texCoordId, (u32)tcid);
    update_gx_state(stage->texMapId, (u32)tmid);
    update_gx_state(stage->channelId, (u32)cid);
}

void GXSetNumTevStages(u8 num) {
    /* Copy from Aurora exactly: GXSetNumTevStages uses update_gx_state */
    /* Aurora's code: update_gx_state(g_gxState.numTevStages, num); */
    update_gx_state(g_gxState->numTevStages, num);
}

void GXSetTevKColor(GXTevKColorID id, GXColor color) {
    GX_UNUSED(id);
    GX_UNUSED(color);
    GX_STUB_NOTICE("GXSetTevKColor");
}

void GXSetTevKColorSel(GXTevStageID id, GXTevKColorSel sel) {
    GX_UNUSED(id);
    GX_UNUSED(sel);
    GX_STUB_NOTICE("GXSetTevKColorSel");
}

void GXSetTevKAlphaSel(GXTevStageID id, GXTevKAlphaSel sel) {
    GX_UNUSED(id);
    GX_UNUSED(sel);
    GX_STUB_NOTICE("GXSetTevKAlphaSel");
}

void GXSetTevSwapMode(GXTevStageID stageId, GXTevSwapSel rasSel, GXTevSwapSel texSel) {
    GX_UNUSED(stageId);
    GX_UNUSED(rasSel);
    GX_UNUSED(texSel);
    GX_STUB_NOTICE("GXSetTevSwapMode");
}

void GXSetTevSwapModeTable(GXTevSwapSel id, GXTevColorChan red, GXTevColorChan green, GXTevColorChan blue,
                           GXTevColorChan alpha) {
    GX_UNUSED(id);
    GX_UNUSED(red);
    GX_UNUSED(green);
    GX_UNUSED(blue);
    GX_UNUSED(alpha);
    GX_STUB_NOTICE("GXSetTevSwapModeTable");
}

void GXSetTevColor(GXTevRegID id, GXColor color) {
    GX_UNUSED(id);
    GX_UNUSED(color);
    GX_STUB_NOTICE("GXSetTevColor");
}

void GXSetTevColorS10(GXTevRegID id, GXColorS10 color) {
    GX_UNUSED(id);
    GX_UNUSED(color);
    GX_STUB_NOTICE("GXSetTevColorS10");
}

void GXSetZTexture(GXZTexOp op, GXTexFmt fmt, u32 bias) {
    GX_UNUSED(op);
    GX_UNUSED(fmt);
    GX_UNUSED(bias);
    GX_STUB_NOTICE("GXSetZTexture");
}


