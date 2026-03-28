#include "gx.hpp"

#include <optional>

extern "C" {

void GXSetVtxDesc(GXAttr attr, GXAttrType type) { update_gx_state(g_gxState().vtxDesc[attr], type); }

void GXSetVtxDescv(GXVtxDescList* list) {
  while (list->attr != GX_VA_NULL) {
    update_gx_state(g_gxState().vtxDesc[list->attr], list->type);
    ++list;
  }
}

void GXClearVtxDesc() { g_gxState().vtxDesc.fill({}); }

void GXSetVtxAttrFmt(GXVtxFmt vtxfmt, GXAttr attr, GXCompCnt cnt, GXCompType type, u8 frac) {
  CHECK(vtxfmt >= GX_VTXFMT0 && vtxfmt < GX_MAX_VTXFMT, "invalid vtxfmt {}", underlying(vtxfmt));
  CHECK(attr >= GX_VA_PNMTXIDX && attr < GX_VA_MAX_ATTR, "invalid attr {}", underlying(attr));
  auto& fmt = g_gxState().vtxFmts[vtxfmt].attrs[attr];
  update_gx_state(fmt.cnt, cnt);
  update_gx_state(fmt.type, type);
  update_gx_state(fmt.frac, frac);
}

void GXSetVtxAttrFmtv(GXVtxFmt vtxfmt, GXVtxAttrFmtList* list) {
  while (list && list->attr != GX_VA_NULL) {
    GXSetVtxAttrFmt(vtxfmt, list->attr, list->cnt, list->type, list->frac);
    ++list;
  }
}

void GXSetArray(GXAttr attr, const void* data, u8 stride) {
  auto& array = g_gxState().arrays[attr];
  update_gx_state(array.data, data);
  // Size cannot be calculated from just data pointer and stride without knowing vertex count
  // Set to 0xFFFFFFFF (UINT32_MAX) as sentinel - will be calculated when array is actually used for rendering
  // The rendering code will calculate size from vertex count * stride when needed
  update_gx_state(array.size, 0xFFFFFFFFu);
  update_gx_state(array.stride, stride);
  array.cachedRange = {};
}

// TODO move GXBegin, GXEnd here

void GXSetTexCoordGen2(GXTexCoordID dst, GXTexGenType type, GXTexGenSrc src, u32 mtx, GXBool normalize, u32 postMtx) {
  CHECK(dst >= GX_TEXCOORD0 && dst <= GX_TEXCOORD7, "invalid tex coord {}", underlying(dst));
  update_gx_state(g_gxState().tcgs[dst],
                  {type, src, static_cast<GXTexMtx>(mtx), static_cast<GXPTTexMtx>(postMtx), normalize});
}

void GXSetNumTexGens(u8 num) { update_gx_state(g_gxState().numTexGens, num); }

void GXInvalidateVtxCache() {
  // TODO
}

void GXSetLineWidth(u8 width, GXTexOffset offs) {
  // TODO
}

void GXSetPointSize(u8, GXTexOffset) { /* stub: no-op on PC */ }
// TODO GXEnableTexOffsets
}