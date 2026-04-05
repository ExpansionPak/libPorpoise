#include "gx.hpp"

extern "C" {
static GXIndTexWrap tile_size_to_wrap(u16 tileSize) {
  switch (tileSize) {
  case 256:
    return GX_ITW_256;
  case 128:
    return GX_ITW_128;
  case 64:
    return GX_ITW_64;
  case 32:
    return GX_ITW_32;
  case 16:
    return GX_ITW_16;
  default:
    return GX_ITW_OFF;
  }
}

void GXSetNumIndStages(u8 num) {
  if (num > 4) {
    num = 4;
  }
  update_gx_state(g_gxState().numIndStages, num);
}

void GXSetIndTexOrder(GXIndTexStageID indStage, GXTexCoordID texCoord, GXTexMapID texMap) {
  auto& stage = g_gxState().indStages[indStage];
  update_gx_state(stage.texCoordId, texCoord);
  update_gx_state(stage.texMapId, texMap);
}

void GXSetIndTexCoordScale(GXIndTexStageID indStage, GXIndTexScale scaleS, GXIndTexScale scaleT) {
  auto& stage = g_gxState().indStages[indStage];
  update_gx_state(stage.scaleS, scaleS);
  update_gx_state(stage.scaleT, scaleT);
}

void GXSetIndTexMtx(GXIndTexMtxID id, const void* offset, s8 scaleExp) {
  CHECK(id >= GX_ITM_0 && id <= GX_ITM_2, "invalid ind tex mtx ID {}", static_cast<int>(id));
  update_gx_state(g_gxState().indTexMtxs[id - 1], {*reinterpret_cast<const porpoise::Mat3x2<float>*>(offset), scaleExp});
}

void GXSetTevIndirect(GXTevStageID tevStage, GXIndTexStageID indStage, GXIndTexFormat fmt, GXIndTexBiasSel biasSel,
                      GXIndTexMtxID matrixSel, GXIndTexWrap wrapS, GXIndTexWrap wrapT, GXBool addPrev, GXBool indLod,
                      GXIndTexAlphaSel alphaSel) {
  auto& stage = g_gxState().tevStages[tevStage];
  update_gx_state(stage.indTexStage, indStage);
  update_gx_state(stage.indTexFormat, fmt);
  update_gx_state(stage.indTexBiasSel, biasSel);
  update_gx_state(stage.indTexAlphaSel, alphaSel);
  update_gx_state(stage.indTexMtxId, matrixSel);
  update_gx_state(stage.indTexWrapS, wrapS);
  update_gx_state(stage.indTexWrapT, wrapT);
  update_gx_state(stage.indTexAddPrev, addPrev);
  update_gx_state(stage.indTexUseOrigLOD, indLod);
}

void GXSetTevDirect(GXTevStageID stageId) {
  auto& stage = g_gxState().tevStages[stageId];
  update_gx_state(stage.indTexStage, GX_INDTEXSTAGE0);
  update_gx_state(stage.indTexFormat, GX_ITF_8);
  update_gx_state(stage.indTexBiasSel, GX_ITB_NONE);
  update_gx_state(stage.indTexAlphaSel, GX_ITBA_OFF);
  update_gx_state(stage.indTexMtxId, GX_ITM_OFF);
  update_gx_state(stage.indTexWrapS, GX_ITW_OFF);
  update_gx_state(stage.indTexWrapT, GX_ITW_OFF);
  update_gx_state(stage.indTexUseOrigLOD, false);
  update_gx_state(stage.indTexAddPrev, false);
}

void GXSetTevIndWarp(GXTevStageID tevStage, GXIndTexStageID indStage, GXBool signedOffsets, GXBool replaceMode,
                     GXIndTexMtxID matrixSel) {
  const auto wrap = replaceMode ? GX_ITW_0 : GX_ITW_OFF;
  const auto biasSel = signedOffsets ? GX_ITB_STU : GX_ITB_NONE;
  GXSetTevIndirect(tevStage, indStage, GX_ITF_8, biasSel, matrixSel, wrap, wrap, false, false, GX_ITBA_OFF);
}

void GXSetTevIndBumpXYZ(GXTevStageID tevStage, GXIndTexStageID indStage, GXIndTexMtxID matrixSel) {
  GXSetTevIndirect(tevStage, indStage, GX_ITF_8, GX_ITB_STU, matrixSel, GX_ITW_OFF, GX_ITW_OFF, false, false,
                   GX_ITBA_OFF);
}

void GXSetTevIndTile(GXTevStageID tevStage, GXIndTexStageID indStage, u16 tileSizeS, u16 tileSizeT, u16 tileSpacingS,
                     u16 tileSpacingT, GXIndTexFormat format, GXIndTexMtxID matrixSel, GXIndTexBiasSel biasSel,
                     GXIndTexAlphaSel alphaSel) {
  const GXIndTexWrap wrapS = tile_size_to_wrap(tileSizeS);
  const GXIndTexWrap wrapT = tile_size_to_wrap(tileSizeT);
  const float mtx[2][3] = {
      {static_cast<float>(tileSpacingS) / 1024.0f, 0.0f, 0.0f},
      {0.0f, static_cast<float>(tileSpacingT) / 1024.0f, 0.0f},
  };

  GXSetIndTexMtx(matrixSel, mtx, 10);
  GXSetTevIndirect(tevStage, indStage, format, biasSel, matrixSel, wrapS, wrapT, false, true, alphaSel);
}

void GXSetTevIndBumpST(GXTevStageID tevStage, GXIndTexStageID indStage, GXIndTexMtxID matrixSel) {
  GXIndTexMtxID sm = GX_ITM_S0;
  GXIndTexMtxID tm = GX_ITM_T0;

  switch (matrixSel) {
  case GX_ITM_0:
    sm = GX_ITM_S0;
    tm = GX_ITM_T0;
    break;
  case GX_ITM_1:
    sm = GX_ITM_S1;
    tm = GX_ITM_T1;
    break;
  case GX_ITM_2:
    sm = GX_ITM_S2;
    tm = GX_ITM_T2;
    break;
  default:
    return;
  }

  GXSetTevIndirect(tevStage, indStage, GX_ITF_8, GX_ITB_ST, sm, GX_ITW_0, GX_ITW_0, false, false, GX_ITBA_OFF);
  GXSetTevIndirect(static_cast<GXTevStageID>(tevStage + 1), indStage, GX_ITF_8, GX_ITB_ST, tm, GX_ITW_0, GX_ITW_0,
                   true, false, GX_ITBA_OFF);
  GXSetTevIndirect(static_cast<GXTevStageID>(tevStage + 2), indStage, GX_ITF_8, GX_ITB_NONE, GX_ITM_OFF, GX_ITW_OFF,
                   GX_ITW_OFF, true, false, GX_ITBA_OFF);
}

void GXSetTevIndRepeat(GXTevStageID tevStage) {
  GXSetTevIndirect(tevStage, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_OFF, GX_ITW_0, GX_ITW_0, true, false,
                   GX_ITBA_OFF);
}
}
