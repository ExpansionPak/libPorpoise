#include "GXStubUtils.h"

#include <dolphin/gx/GXTexture.h>

void GXInitTexObj(GXTexObj* obj, const void* data, u16 width, u16 height, u32 format, GXTexWrapMode wrapS,
                  GXTexWrapMode wrapT, GXBool mipmap) {
    GX_UNUSED(obj);
    GX_UNUSED(data);
    GX_UNUSED(width);
    GX_UNUSED(height);
    GX_UNUSED(format);
    GX_UNUSED(wrapS);
    GX_UNUSED(wrapT);
    GX_UNUSED(mipmap);
    GX_STUB_NOTICE("GXInitTexObj");
}

void GXInitTexObjCI(GXTexObj* obj, const void* data, u16 width, u16 height, GXCITexFmt format, GXTexWrapMode wrapS,
                    GXTexWrapMode wrapT, GXBool mipmap, u32 tlutName) {
    GX_UNUSED(obj);
    GX_UNUSED(data);
    GX_UNUSED(width);
    GX_UNUSED(height);
    GX_UNUSED(format);
    GX_UNUSED(wrapS);
    GX_UNUSED(wrapT);
    GX_UNUSED(mipmap);
    GX_UNUSED(tlutName);
    GX_STUB_NOTICE("GXInitTexObjCI");
}

void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter minFilt, GXTexFilter magFilt, f32 minLod, f32 maxLod, f32 lodBias,
                     GXBool biasClamp, GXBool edgeLod, GXAnisotropy maxAniso) {
    GX_UNUSED(obj);
    GX_UNUSED(minFilt);
    GX_UNUSED(magFilt);
    GX_UNUSED(minLod);
    GX_UNUSED(maxLod);
    GX_UNUSED(lodBias);
    GX_UNUSED(biasClamp);
    GX_UNUSED(edgeLod);
    GX_UNUSED(maxAniso);
    GX_STUB_NOTICE("GXInitTexObjLOD");
}

void GXInitTexObjWrapMode(GXTexObj* obj, GXTexWrapMode wrapS, GXTexWrapMode wrapT) {
    GX_UNUSED(obj);
    GX_UNUSED(wrapS);
    GX_UNUSED(wrapT);
    GX_STUB_NOTICE("GXInitTexObjWrapMode");
}

void GXInitTexObjData(GXTexObj* obj, const void* data) {
    GX_UNUSED(obj);
    GX_UNUSED(data);
    GX_STUB_NOTICE("GXInitTexObjData");
}

void GXInitTlutObj(GXTlutObj* obj, const void* data, GXTlutFmt format, u16 entries) {
    GX_UNUSED(obj);
    GX_UNUSED(data);
    GX_UNUSED(format);
    GX_UNUSED(entries);
    GX_STUB_NOTICE("GXInitTlutObj");
}

void GXLoadTlut(const GXTlutObj* obj, GXTexMapID id) {
    GX_UNUSED(obj);
    GX_UNUSED(id);
    GX_STUB_NOTICE("GXLoadTlut");
}

void GXLoadTexObj(const GXTexObj* obj, GXTexMapID id) {
    GX_UNUSED(obj);
    GX_UNUSED(id);
    GX_STUB_NOTICE("GXLoadTexObj");
}

void GXInvalidateTexAll(void) {
    /* Copy from Aurora exactly: GXInvalidateTexAll is a no-op */
    /* Aurora's code: // no-op? */
    /* No-op for now */
}

void GXSetTexCoordScaleManually(GXTexCoordID coord, GXBool enable, u16 ss, u16 ts) {
    GX_UNUSED(coord);
    GX_UNUSED(enable);
    GX_UNUSED(ss);
    GX_UNUSED(ts);
    GX_STUB_NOTICE("GXSetTexCoordScaleManually");
}

u32 GXGetTexBufferSize(u16 width, u16 height, u32 format, GXBool mipmap, u8 maxLod) {
    GX_UNUSED(width);
    GX_UNUSED(height);
    GX_UNUSED(format);
    GX_UNUSED(mipmap);
    GX_UNUSED(maxLod);
    GX_STUB_NOTICE("GXGetTexBufferSize");
    return 0;
}

void GXInitTexCacheRegion(GXTexRegion* region, GXBool is32bMip, u32 tmemEven, GXTexCacheSize sizeEven, u32 tmemOdd,
                          GXTexCacheSize sizeOdd) {
    GX_UNUSED(region);
    GX_UNUSED(is32bMip);
    GX_UNUSED(tmemEven);
    GX_UNUSED(sizeEven);
    GX_UNUSED(tmemOdd);
    GX_UNUSED(sizeOdd);
    GX_STUB_NOTICE("GXInitTexCacheRegion");
}

static GXTexRegionCallback s_texRegionCallback = NULL;

GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback callback) {
    GXTexRegionCallback prev = s_texRegionCallback;
    s_texRegionCallback = callback;
    GX_STUB_NOTICE("GXSetTexRegionCallback");
    return prev;
}

void GXInvalidateTexRegion(const GXTexRegion* region) {
    GX_UNUSED(region);
    GX_STUB_NOTICE("GXInvalidateTexRegion");
}


