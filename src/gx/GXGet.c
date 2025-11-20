#include "GXStubUtils.h"

#include <dolphin/gx/GXGet.h>

GXBool GXGetTexObjMipMap(GXTexObj* tex_obj) {
    GX_UNUSED(tex_obj);
    GX_STUB_NOTICE("GXGetTexObjMipMap");
    return GX_FALSE;
}

GXTexFmt GXGetTexObjFmt(GXTexObj* tex_obj) {
    GX_UNUSED(tex_obj);
    GX_STUB_NOTICE("GXGetTexObjFmt");
    return GX_TF_I4;
}

u16 GXGetTexObjHeight(GXTexObj* tex_obj) {
    GX_UNUSED(tex_obj);
    GX_STUB_NOTICE("GXGetTexObjHeight");
    return 0;
}

u16 GXGetTexObjWidth(GXTexObj* tex_obj) {
    GX_UNUSED(tex_obj);
    GX_STUB_NOTICE("GXGetTexObjWidth");
    return 0;
}

GXTexWrapMode GXGetTexObjWrapS(GXTexObj* tex_obj) {
    GX_UNUSED(tex_obj);
    GX_STUB_NOTICE("GXGetTexObjWrapS");
    return GX_CLAMP;
}

GXTexWrapMode GXGetTexObjWrapT(GXTexObj* tex_obj) {
    GX_UNUSED(tex_obj);
    GX_STUB_NOTICE("GXGetTexObjWrapT");
    return GX_CLAMP;
}

void* GXGetTexObjData(GXTexObj* tex_obj) {
    GX_UNUSED(tex_obj);
    GX_STUB_NOTICE("GXGetTexObjData");
    return NULL;
}

void GXGetProjectionv(f32* p) {
    if (p) {
        for (int i = 0; i < 7; ++i) {
            p[i] = 0.0f;
        }
    }
    GX_STUB_NOTICE("GXGetProjectionv");
}

void GXGetLightAttnA(GXLightObj* light_, float* a0, float* a1, float* a2) {
    GX_UNUSED(light_);
    if (a0) *a0 = 1.0f;
    if (a1) *a1 = 0.0f;
    if (a2) *a2 = 0.0f;
    GX_STUB_NOTICE("GXGetLightAttnA");
}

void GXGetLightAttnK(GXLightObj* light_, float* k0, float* k1, float* k2) {
    GX_UNUSED(light_);
    if (k0) *k0 = 1.0f;
    if (k1) *k1 = 0.0f;
    if (k2) *k2 = 0.0f;
    GX_STUB_NOTICE("GXGetLightAttnK");
}

void GXGetLightPos(GXLightObj* light_, float* x, float* y, float* z) {
    GX_UNUSED(light_);
    if (x) *x = 0.0f;
    if (y) *y = 0.0f;
    if (z) *z = 0.0f;
    GX_STUB_NOTICE("GXGetLightPos");
}

void GXGetLightDir(GXLightObj* light_, float* nx, float* ny, float* nz) {
    GX_UNUSED(light_);
    if (nx) *nx = 0.0f;
    if (ny) *ny = 0.0f;
    if (nz) *nz = -1.0f;
    GX_STUB_NOTICE("GXGetLightDir");
}

void GXGetLightColor(GXLightObj* light_, GXColor* col) {
    GX_UNUSED(light_);
    if (col) {
        col->r = 255;
        col->g = 255;
        col->b = 255;
        col->a = 255;
    }
    GX_STUB_NOTICE("GXGetLightColor");
}

void GXGetVtxAttrFmt(GXVtxFmt idx, GXAttr attr, GXCompCnt* compCnt, GXCompType* compType, u8* shift) {
    GX_UNUSED(idx);
    GX_UNUSED(attr);
    if (compCnt) *compCnt = GX_POS_XYZ;
    if (compType) *compType = GX_F32;
    if (shift) *shift = 0;
    GX_STUB_NOTICE("GXGetVtxAttrFmt");
}


