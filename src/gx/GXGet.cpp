#include "gx.hpp"

#include "../gfx/texture.hpp"

extern "C" {

// TODO GXGetVtxDesc
// TODO GXGetVtxDescv
// TODO GXGetVtxAttrFmtv
// TODO GXGetLineWidth
// TODO GXGetPointSize

void GXGetVtxDesc(GXAttr attr, GXAttrType* type) {
  if (!type) return;
  if (attr >= GX_VA_MAX_ATTR) {
    *type = GX_NONE;
    return;
  }
  *type = g_gxState().vtxDesc[attr];
}

void GXGetVtxDescv(GXVtxDescList* list) {
  if (!list) return;
  u32 n = 0;
  for (u32 attr = GX_VA_PNMTXIDX; attr < GX_VA_MAX_ATTR && n < GX_MAX_VTXDESCLIST_SZ - 1; ++attr) {
    const auto type = g_gxState().vtxDesc[attr];
    if (type == GX_NONE) continue;
    list[n].attr = static_cast<GXAttr>(attr);
    list[n].type = type;
    ++n;
  }
  list[n].attr = GX_VA_NULL;
  list[n].type = GX_NONE;
}

void GXGetVtxAttrFmt(GXVtxFmt idx, GXAttr attr, GXCompCnt* compCnt, GXCompType* compType, u8* shift) {
  const auto& fmt = g_gxState().vtxFmts[idx].attrs[attr];
  *compCnt = fmt.cnt;
  *compType = fmt.type;
  *shift = fmt.frac;
}

// TODO GXGetViewportv

void GXGetProjectionv(f32* p) {
  const auto& mtx = g_gxState().proj;
  const auto type = g_gxState().projType;
  p[0] = static_cast<float>(type);
  p[1] = mtx(0, 0);
  p[3] = mtx(1, 1);
  p[5] = mtx(2, 2);
  p[6] = mtx(2, 3);
  if (type == GX_ORTHOGRAPHIC) {
    p[2] = mtx(0, 3);
    p[4] = mtx(1, 3);
  } else {
    p[2] = mtx(0, 2);
    p[4] = mtx(1, 2);
  }
}

void GXGetScissor(u32* xOrig, u32* yOrig, u32* wd, u32* ht) {
  if (xOrig) *xOrig = g_gxState().scissorLeft;
  if (yOrig) *yOrig = g_gxState().scissorTop;
  if (wd) *wd = g_gxState().scissorWd;
  if (ht) *ht = g_gxState().scissorHt;
}

void GXGetCullMode(GXCullMode* mode) {
  if (!mode) return;
  *mode = g_gxState().cullMode;
}

void GXGetLightAttnA(GXLightObj* light_, float* a0, float* a1, float* a2) {
  auto* light = reinterpret_cast<const GXLightObj_*>(light_);
  *a0 = light->a0;
  *a1 = light->a1;
  *a2 = light->a2;
}

void GXGetLightAttnK(GXLightObj* light_, float* k0, float* k1, float* k2) {
  auto* light = reinterpret_cast<const GXLightObj_*>(light_);
  *k0 = light->k0;
  *k1 = light->k1;
  *k2 = light->k2;
}

void GXGetLightPos(GXLightObj* light_, float* x, float* y, float* z) {
  auto* light = reinterpret_cast<const GXLightObj_*>(light_);
  *x = light->px;
  *y = light->py;
  *z = light->pz;
}

void GXGetLightDir(GXLightObj* light_, float* nx, float* ny, float* nz) {
  auto* light = reinterpret_cast<const GXLightObj_*>(light_);
  *nx = -light->nx;
  *ny = -light->ny;
  *nz = -light->nz;
}

void GXGetLightColor(GXLightObj* light_, GXColor* col) {
  auto* light = reinterpret_cast<const GXLightObj_*>(light_);
  *col = light->color;
}

void* GXGetTexObjData(GXTexObj* tex_obj) {
  return const_cast<void*>(reinterpret_cast<const GXTexObj_*>(tex_obj)->data);
}

u16 GXGetTexObjWidth(GXTexObj* tex_obj) { return reinterpret_cast<const GXTexObj_*>(tex_obj)->width; }

u16 GXGetTexObjHeight(GXTexObj* tex_obj) { return reinterpret_cast<const GXTexObj_*>(tex_obj)->height; }

GXTexFmt GXGetTexObjFmt(GXTexObj* tex_obj) {
  return static_cast<GXTexFmt>(reinterpret_cast<const GXTexObj_*>(tex_obj)->fmt);
}

GXTexWrapMode GXGetTexObjWrapS(GXTexObj* tex_obj) { return reinterpret_cast<const GXTexObj_*>(tex_obj)->wrapS; }

GXTexWrapMode GXGetTexObjWrapT(GXTexObj* tex_obj) { return reinterpret_cast<const GXTexObj_*>(tex_obj)->wrapT; }

GXBool GXGetTexObjMipMap(GXTexObj* tex_obj) { return reinterpret_cast<const GXTexObj_*>(tex_obj)->hasMips; }

void* GXGetTexObjUserData(GXTexObj* tex_obj) {
  auto it = g_gxState().texObjUserData.find(tex_obj);
  if (it == g_gxState().texObjUserData.end()) {
    return nullptr;
  }
  return const_cast<void*>(it->second);
}

void GXGetTexObjAll(GXTexObj* tex_obj, void** image_ptr, u16* width, u16* height, GXTexFmt* format,
                    GXTexWrapMode* wrap_s, GXTexWrapMode* wrap_t, GXBool* mipmap) {
  if (image_ptr) *image_ptr = GXGetTexObjData(tex_obj);
  if (width) *width = GXGetTexObjWidth(tex_obj);
  if (height) *height = GXGetTexObjHeight(tex_obj);
  if (format) *format = GXGetTexObjFmt(tex_obj);
  if (wrap_s) *wrap_s = GXGetTexObjWrapS(tex_obj);
  if (wrap_t) *wrap_t = GXGetTexObjWrapT(tex_obj);
  if (mipmap) *mipmap = GXGetTexObjMipMap(tex_obj);
}
// TODO GXGetTexObjMinFilt
// TODO GXGetTexObjMagFilt
// TODO GXGetTexObjMinLOD
// TODO GXGetTexObjMaxLOD
// TODO GXGetTexObjLODBias
// TODO GXGetTexObjBiasClamp
// TODO GXGetTexObjEdgeLOD
// TODO GXGetTexObjMaxAniso
// TODO GXGetTexObjLODAll
// TODO GXGetTexObjTlut
void* GXGetTlutObjData(GXTlutObj* tlut_obj) {
  return const_cast<void*>(reinterpret_cast<const GXTlutObj_*>(tlut_obj)->data);
}

GXTlutFmt GXGetTlutObjFmt(GXTlutObj* tlut_obj) {
  return reinterpret_cast<const GXTlutObj_*>(tlut_obj)->fmt;
}

u16 GXGetTlutObjNumEntries(GXTlutObj* tlut_obj) {
  return reinterpret_cast<const GXTlutObj_*>(tlut_obj)->numEntries;
}

void GXGetTlutObjAll(GXTlutObj* tlut_obj, void** lut, GXTlutFmt* fmt, u16* n_entries) {
  if (lut) *lut = GXGetTlutObjData(tlut_obj);
  if (fmt) *fmt = GXGetTlutObjFmt(tlut_obj);
  if (n_entries) *n_entries = GXGetTlutObjNumEntries(tlut_obj);
}
// TODO GXGetTexRegionAll
// TODO GXGetTlutRegionAll
}
