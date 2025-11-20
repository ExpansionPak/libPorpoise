#include "GXStubUtils.h"

#include <dolphin/gx/GXBump.h>

void GXSetNumIndStages(u8 num) {
    GX_UNUSED(num);
    GX_STUB_NOTICE("GXSetNumIndStages");
}

void GXSetIndTexOrder(GXIndTexStageID indStage, GXTexCoordID texCoord, GXTexMapID texMap) {
    GX_UNUSED(indStage);
    GX_UNUSED(texCoord);
    GX_UNUSED(texMap);
    GX_STUB_NOTICE("GXSetIndTexOrder");
}

void GXSetIndTexCoordScale(GXIndTexStageID indStage, GXIndTexScale scaleS, GXIndTexScale scaleT) {
    GX_UNUSED(indStage);
    GX_UNUSED(scaleS);
    GX_UNUSED(scaleT);
    GX_STUB_NOTICE("GXSetIndTexCoordScale");
}

void GXSetIndTexMtx(GXIndTexMtxID id, const void* offset, s8 scaleExp) {
    GX_UNUSED(id);
    GX_UNUSED(offset);
    GX_UNUSED(scaleExp);
    GX_STUB_NOTICE("GXSetIndTexMtx");
}

void GXSetTevIndirect(GXTevStageID tevStage, GXIndTexStageID indStage, GXIndTexFormat fmt, GXIndTexBiasSel biasSel,
                      GXIndTexMtxID matrixSel, GXIndTexWrap wrapS, GXIndTexWrap wrapT, GXBool addPrev, GXBool indLod,
                      GXIndTexAlphaSel alphaSel) {
    GX_UNUSED(tevStage);
    GX_UNUSED(indStage);
    GX_UNUSED(fmt);
    GX_UNUSED(biasSel);
    GX_UNUSED(matrixSel);
    GX_UNUSED(wrapS);
    GX_UNUSED(wrapT);
    GX_UNUSED(addPrev);
    GX_UNUSED(indLod);
    GX_UNUSED(alphaSel);
    GX_STUB_NOTICE("GXSetTevIndirect");
}

void GXSetTevDirect(GXTevStageID stageId) {
    GX_UNUSED(stageId);
    GX_STUB_NOTICE("GXSetTevDirect");
}

void GXSetTevIndWarp(GXTevStageID tevStage, GXIndTexStageID indStage, GXBool signedOffsets, GXBool replaceMode,
                     GXIndTexMtxID matrixSel) {
    GX_UNUSED(tevStage);
    GX_UNUSED(indStage);
    GX_UNUSED(signedOffsets);
    GX_UNUSED(replaceMode);
    GX_UNUSED(matrixSel);
    GX_STUB_NOTICE("GXSetTevIndWarp");
}

void GXSetTevIndTile(GXTevStageID tevStage, GXIndTexStageID indStage, GXIndTexFormat fmt, GXIndTexBiasSel biasSel,
                     GXIndTexMtxID matrixSel, GXIndTexWrap wrapS, GXIndTexWrap wrapT, GXBool addPrev, GXBool indLod,
                     GXIndTexAlphaSel alphaSel) {
    GX_UNUSED(tevStage);
    GX_UNUSED(indStage);
    GX_UNUSED(fmt);
    GX_UNUSED(biasSel);
    GX_UNUSED(matrixSel);
    GX_UNUSED(wrapS);
    GX_UNUSED(wrapT);
    GX_UNUSED(addPrev);
    GX_UNUSED(indLod);
    GX_UNUSED(alphaSel);
    GX_STUB_NOTICE("GXSetTevIndTile");
}

void GXSetTevIndBumpST(GXTevStageID tevStage, GXIndTexStageID indStage, GXIndTexMtxID matrixSel) {
    GX_UNUSED(tevStage);
    GX_UNUSED(indStage);
    GX_UNUSED(matrixSel);
    GX_STUB_NOTICE("GXSetTevIndBumpST");
}

void GXSetTevIndBumpXYZ(GXTevStageID tevStage, GXIndTexStageID indStage, GXIndTexMtxID matrixSel) {
    GX_UNUSED(tevStage);
    GX_UNUSED(indStage);
    GX_UNUSED(matrixSel);
    GX_STUB_NOTICE("GXSetTevIndBumpXYZ");
}

void GXSetTevIndRepeat(GXTevStageID tevStage, GXIndTexStageID indStage, GXIndTexWrap wrapS, GXIndTexWrap wrapT) {
    GX_UNUSED(tevStage);
    GX_UNUSED(indStage);
    GX_UNUSED(wrapS);
    GX_UNUSED(wrapT);
    GX_STUB_NOTICE("GXSetTevIndRepeat");
}


