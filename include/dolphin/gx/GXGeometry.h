#ifndef DOLPHIN_GXGEOMETRY_H
#define DOLPHIN_GXGEOMETRY_H

#include <dolphin/gx/GXEnum.h>
#include <dolphin/gx/GXStruct.h>

#ifdef __cplusplus
extern "C" {
#endif

void GXSetVtxDesc(GXAttr attr, GXAttrType type);
void GXSetVtxDescv(GXVtxDescList* list);
void GXClearVtxDesc(void);
void GXSetVtxAttrFmt(GXVtxFmt vtxfmt, GXAttr attr, GXCompCnt cnt, GXCompType type, u8 frac);
void GXSetNumTexGens(u8 nTexGens);
void GXBegin(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts);
void GXEnd(void);
void GXSetTexCoordGen2(GXTexCoordID dst_coord, GXTexGenType func, GXTexGenSrc src_param, u32 mtx, GXBool normalize,
                       u32 postmtx);
void GXSetLineWidth(u8 width, GXTexOffset texOffsets);
void GXSetPointSize(u8 pointSize, GXTexOffset texOffsets);
void GXEnableTexOffsets(GXTexCoordID coord, GXBool line_enable, GXBool point_enable);
/* Original GameCube SDK signature: 3 parameters (attr, base_ptr, stride) */
/* This matches the original Nintendo SDK exactly */
void GXSetArray(GXAttr attr, const void* base_ptr, u8 stride);

#ifdef TARGET_PC
/* Aurora extension: 4 parameters (adds size for PC builds) */
/* This is an Aurora-specific extension, not in the original SDK */
void GXSetArray_4Param(GXAttr attr, const void* data, u32 size, u8 stride);
/* Macro for Aurora-style 4-parameter calls */
#define GXSETARRAY(attr, data, size, stride) GXSetArray_4Param((attr), (data), (size), (stride))
#else
/* Original SDK macro - ignores size parameter */
#define GXSETARRAY(attr, data, size, stride) GXSetArray((attr), (data), (stride))
#endif
void GXInvalidateVtxCache(void);

static inline void GXSetTexCoordGen(GXTexCoordID dst_coord, GXTexGenType func, GXTexGenSrc src_param, u32 mtx) {
  GXSetTexCoordGen2(dst_coord, func, src_param, mtx, GX_FALSE, GX_PTIDENTITY);
}

#ifdef __cplusplus
}
#endif

#endif
