#ifndef DOLPHIN_GXBUMP_H
#define DOLPHIN_GXBUMP_H

#include <dolphin/gx/GXEnum.h>

#ifdef __cplusplus
extern "C" {
#endif

void GXSetTevDirect(GXTevStageID tev_stage);
void GXSetNumIndStages(u8 nIndStages);
#ifdef TARGET_PC
void GXSetIndTexMtx(GXIndTexMtxID mtx_sel, const void* offset, s8 scale_exp);
#else
void GXSetIndTexMtx(GXIndTexMtxID mtx_sel, f32 offset[2][3], s8 scale_exp);
#endif
void GXSetIndTexOrder(GXIndTexStageID ind_stage, GXTexCoordID tex_coord, GXTexMapID tex_map);
void GXSetIndTexCoordScale(GXIndTexStageID indStage, GXIndTexScale scaleS, GXIndTexScale scaleT);
void GXSetTevIndirect(GXTevStageID tev_stage, GXIndTexStageID ind_stage, GXIndTexFormat format,
                      GXIndTexBiasSel bias_sel, GXIndTexMtxID matrix_sel, GXIndTexWrap wrap_s, GXIndTexWrap wrap_t,
                      GXBool add_prev, GXBool ind_lod, GXIndTexAlphaSel alpha_sel);
void GXSetTevIndWarp(GXTevStageID tev_stage, GXIndTexStageID ind_stage, GXBool signed_offsets, GXBool replace_mode,
                     GXIndTexMtxID matrix_sel);
void GXSetTevIndBumpXYZ(GXTevStageID tev_stage, GXIndTexStageID ind_stage, GXIndTexMtxID matrix_sel);
void GXSetTevIndTile(GXTevStageID tev_stage, GXIndTexStageID ind_stage, u16 tilesize_s, u16 tilesize_t,
                     u16 tilespacing_s, u16 tilespacing_t, GXIndTexFormat format, GXIndTexMtxID matrix_sel,
                     GXIndTexBiasSel bias_sel, GXIndTexAlphaSel alpha_sel);
void GXSetTevIndBumpST(GXTevStageID tev_stage, GXIndTexStageID ind_stage, GXIndTexMtxID matrix_sel);
void GXSetTevIndRepeat(GXTevStageID tev_stage);

#ifdef __cplusplus
}
#endif

#endif
