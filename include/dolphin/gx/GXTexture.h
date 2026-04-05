#ifndef DOLPHIN_GXTEXTURE_H
#define DOLPHIN_GXTEXTURE_H

#include <dolphin/gx/GXEnum.h>
#include <dolphin/gx/GXStruct.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef GXTexRegion* (*GXTexRegionCallback)(GXTexObj* obj, GXTexMapID id);
typedef GXTlutRegion* (*GXTlutRegionCallback)(u32 tlut_name);

void GXInitTexObj(GXTexObj* obj, const void* data, u16 width, u16 height, u32 format, GXTexWrapMode wrapS,
                  GXTexWrapMode wrapT, GXBool mipmap);
void GXInitTexObjCI(GXTexObj* obj, const void* data, u16 width, u16 height, GXCITexFmt format, GXTexWrapMode wrapS,
                    GXTexWrapMode wrapT, GXBool mipmap, u32 tlut);
void GXInitTexObjData(GXTexObj* obj, const void* data);
void GXInitTexObjUserData(GXTexObj* obj, const void* user_data);
void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter min_filt, GXTexFilter mag_filt, f32 min_lod, f32 max_lod, f32 lod_bias,
                     GXBool bias_clamp, GXBool do_edge_lod, GXAnisotropy max_aniso);
void GXLoadTexObj(GXTexObj* obj, GXTexMapID id);
u32 GXGetTexBufferSize(u16 width, u16 height, u32 format, GXBool mipmap, u8 max_lod);
void GXInvalidateTexAll();
void GXInitTexObjWrapMode(GXTexObj* obj, GXTexWrapMode s, GXTexWrapMode t);
void GXInitTlutObj(GXTlutObj* obj, const void* data, GXTlutFmt format, u16 entries);
void GXLoadTlut(const GXTlutObj* obj, GXTlut idx);
void GXSetTexCoordScaleManually(GXTexCoordID coord, GXBool enable, u16 ss, u16 ts);
void GXInitTexCacheRegion(GXTexRegion* region, GXBool is_32b_mipmap, u32 tmem_even, GXTexCacheSize size_even,
                          u32 tmem_odd, GXTexCacheSize size_odd);
void GXInitTexPreLoadRegion(GXTexRegion* region, u32 tmem_even, u32 size_even, u32 tmem_odd, u32 size_odd);
void GXInitTlutRegion(GXTlutRegion* region, u32 tmem_addr, u32 tlut_size);
GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback callback);
GXTlutRegionCallback GXSetTlutRegionCallBack(GXTlutRegionCallback callback);
void GXPreLoadEntireTexture(GXTexObj* tex_obj, GXTexRegion* region);
void GXLoadTexObjPreLoaded(GXTexObj* obj, GXTexRegion* region, GXTexMapID id);
void GXInvalidateTexRegion(const GXTexRegion* region);

#define GXSetTexRegionCallBack GXSetTexRegionCallback
#define GXSetTlutRegionCallback GXSetTlutRegionCallBack

#ifdef __cplusplus
}
#endif

#endif
