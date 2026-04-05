#include "gx.hpp"

#include "../gfx/texture.hpp"

#include <unordered_map>

namespace {
GXTexRegion* default_tex_region(GXTexObj* obj, GXTexMapID id) {
  static GXTexRegion s_regions[GX_MAX_TEXMAP];
  (void)obj;
  return &s_regions[id % GX_MAX_TEXMAP];
}

GXTlutRegion* default_tlut_region(u32 tlutName) {
  static GXTlutRegion s_regions[GX_BIGTLUT3 + 1];
  return &s_regions[tlutName % (GX_BIGTLUT3 + 1)];
}

GXTexRegionCallback g_texRegionCallback = default_tex_region;
GXTlutRegionCallback g_tlutRegionCallback = default_tlut_region;

inline bool is_ci_tex_format(u32 fmt) {
  return fmt == GX_TF_C4 || fmt == GX_TF_C8 || fmt == GX_TF_C14X2;
}

inline void ensure_tex_uploaded(GXTexObj_* obj) {
  if (!obj->ref) {
    char name[64];
    snprintf(name, sizeof(name), "GXLoadTexObj_%u", obj->fmt);
    obj->ref =
        porpoise::gfx::new_dynamic_texture_2d(obj->width, obj->height, u32(obj->maxLod) + 1, obj->fmt, name);
  }
  if (obj->dataInvalidated) {
    u32 dataSize = obj->width * obj->height * 4;  // TODO: Calculate based on format
    std::string_view data(static_cast<const char*>(obj->data), dataSize);
    porpoise::gfx::write_texture(*obj->ref, data);
    obj->dataInvalidated = false;
  }
}

inline void notify_texture_state() {
}
} // namespace

extern "C" {
void GXInitTexObj(GXTexObj* obj_, const void* data, u16 width, u16 height, u32 format, GXTexWrapMode wrapS,
                  GXTexWrapMode wrapT, GXBool mipmap) {
  memset(obj_, 0, sizeof(GXTexObj));
  auto* obj = reinterpret_cast<GXTexObj_*>(obj_);
  obj->data = data;
  obj->width = width;
  obj->height = height;
  obj->fmt = format;
  obj->wrapS = wrapS;
  obj->wrapT = wrapT;
  obj->hasMips = mipmap;
  // TODO default values?
  obj->minFilter = GX_LINEAR;
  obj->magFilter = GX_LINEAR;
  obj->minLod = 0.f;
  obj->maxLod = 0.f;
  obj->lodBias = 0.f;
  obj->biasClamp = false;
  obj->doEdgeLod = false;
  obj->maxAniso = GX_ANISO_4;
  obj->tlut = GX_TLUT0;
  const auto it = g_gxState().copyTextures.find(data);
  if (it != g_gxState().copyTextures.end()) {
    obj->ref = it->second;
    obj->dataInvalidated = false;
  } else {
    obj->dataInvalidated = true;
  }
  g_gxState().texObjUserData[obj_] = nullptr;
  notify_texture_state();
}

void GXInitTexObjCI(GXTexObj* obj_, const void* data, u16 width, u16 height, GXCITexFmt format, GXTexWrapMode wrapS,
                    GXTexWrapMode wrapT, GXBool mipmap, u32 tlut) {
  memset(obj_, 0, sizeof(GXTexObj));
  auto* obj = reinterpret_cast<GXTexObj_*>(obj_);
  obj->data = data;
  obj->width = width;
  obj->height = height;
  obj->fmt = static_cast<GXTexFmt>(format);
  obj->wrapS = wrapS;
  obj->wrapT = wrapT;
  obj->hasMips = mipmap;
  obj->tlut = static_cast<GXTlut>(tlut);
  // TODO default values?
  obj->minFilter = GX_LINEAR;
  obj->magFilter = GX_LINEAR;
  obj->minLod = 0.f;
  obj->maxLod = 0.f;
  obj->lodBias = 0.f;
  obj->biasClamp = false;
  obj->doEdgeLod = false;
  obj->maxAniso = GX_ANISO_4;
  const auto it = g_gxState().copyTextures.find(data);
  if (it != g_gxState().copyTextures.end()) {
    obj->ref = it->second;
    obj->dataInvalidated = false;
  } else {
    obj->dataInvalidated = true;
  }
  g_gxState().texObjUserData[obj_] = nullptr;
  notify_texture_state();
}

void GXInitTexObjLOD(GXTexObj* obj_, GXTexFilter minFilt, GXTexFilter magFilt, float minLod, float maxLod,
                     float lodBias, GXBool biasClamp, GXBool doEdgeLod, GXAnisotropy maxAniso) {
  auto* obj = reinterpret_cast<GXTexObj_*>(obj_);
  obj->minFilter = minFilt;
  obj->magFilter = magFilt;
  obj->minLod = minLod;
  obj->maxLod = maxLod;
  obj->lodBias = lodBias;
  obj->biasClamp = biasClamp;
  obj->doEdgeLod = doEdgeLod;
  obj->maxAniso = maxAniso;
  notify_texture_state();
}

void GXInitTexObjData(GXTexObj* obj_, const void* data) {
  auto* obj = reinterpret_cast<GXTexObj_*>(obj_);
  const auto it = g_gxState().copyTextures.find(data);
  if (it != g_gxState().copyTextures.end()) {
    obj->ref = it->second;
    obj->dataInvalidated = false;
  } else {
    obj->data = data;
    obj->dataInvalidated = true;
  }
  notify_texture_state();
}

void GXInitTexObjUserData(GXTexObj* obj_, const void* userData) {
  g_gxState().texObjUserData[obj_] = userData;
}

void GXInitTexObjWrapMode(GXTexObj* obj_, GXTexWrapMode wrapS, GXTexWrapMode wrapT) {
  auto* obj = reinterpret_cast<GXTexObj_*>(obj_);
  obj->wrapS = wrapS;
  obj->wrapT = wrapT;
  notify_texture_state();
}

void GXInitTexObjTlut(GXTexObj* obj_, u32 tlut) {
  auto* obj = reinterpret_cast<GXTexObj_*>(obj_);
  obj->tlut = static_cast<GXTlut>(tlut);
  notify_texture_state();
}

// TODO GXInitTexObjFilter
// TODO GXInitTexObjMaxLOD
// TODO GXInitTexObjMinLOD
// TODO GXInitTexObjLODBias
// TODO GXInitTexObjBiasClamp
// TODO GXInitTexObjEdgeLOD
// TODO GXInitTexObjMaxAniso

void GXLoadTexObj(GXTexObj* obj_, GXTexMapID id) {
  auto* obj = reinterpret_cast<GXTexObj_*>(obj_);
  if (g_texRegionCallback) {
    (void)g_texRegionCallback(obj_, id);
  }
  if (is_ci_tex_format(obj->fmt) && g_tlutRegionCallback) {
    (void)g_tlutRegionCallback(static_cast<u32>(obj->tlut));
  }
  ensure_tex_uploaded(obj);
  g_gxState().textures[id] = {obj->ref};
  g_gxState().stateDirty = true; // TODO only if changed?
  notify_texture_state();
}

u32 GXGetTexBufferSize(u16 width, u16 height, u32 fmt, GXBool mips, u8 maxLod) {
  s32 shiftX = 0;
  s32 shiftY = 0;
  switch (fmt) {
  case GX_TF_I4:
  case GX_TF_C4:
  case GX_TF_CMPR:
  case GX_CTF_R4:
  case GX_CTF_Z4:
    shiftX = 3;
    shiftY = 3;
    break;
  case GX_TF_I8:
  case GX_TF_IA4:
  case GX_TF_C8:
  case GX_TF_Z8:
  case GX_CTF_RA4:
  case GX_CTF_A8:
  case GX_CTF_R8:
  case GX_CTF_G8:
  case GX_CTF_B8:
  case GX_CTF_Z8M:
  case GX_CTF_Z8L:
    shiftX = 3;
    shiftY = 2;
    break;
  case GX_TF_IA8:
  case GX_TF_RGB565:
  case GX_TF_RGB5A3:
  case GX_TF_RGBA8:
  case GX_TF_C14X2:
  case GX_TF_Z16:
  case GX_TF_Z24X8:
  case GX_CTF_RA8:
  case GX_CTF_YUVA8:
  case GX_CTF_RG8:
  case GX_CTF_GB8:
  case GX_CTF_Z16L:
    shiftX = 2;
    shiftY = 2;
    break;
  default:
    break;
  }
  u32 bitSize = (fmt == GX_TF_RGBA8 || fmt == GX_TF_Z24X8 || fmt == GX_CTF_YUVA8) ? 64 : 32;
  u32 bufLen = 0;
  if (mips) {
    while (maxLod != 0) {
      const u32 tileX = ((width + (1 << shiftX) - 1) >> shiftX);
      const u32 tileY = ((height + (1 << shiftY) - 1) >> shiftY);
      bufLen += bitSize * tileX * tileY;

      if (width == 1 && height == 1) {
        return bufLen;
      }

      width = (width < 2) ? 1 : width / 2;
      height = (height < 2) ? 1 : height / 2;
      --maxLod;
    };
  } else {
    const u32 tileX = ((width + (1 << shiftX) - 1) >> shiftX);
    const u32 tileY = ((height + (1 << shiftY) - 1) >> shiftY);
    bufLen = bitSize * tileX * tileY;
  }

  return bufLen;
}

void GXInitTlutObj(GXTlutObj* obj_, const void* data, GXTlutFmt format, u16 entries) {
  memset(obj_, 0, sizeof(GXTlutObj));
  GXTexFmt texFmt;
  switch (format) {
    DEFAULT_FATAL("invalid tlut format {}", static_cast<int>(format));
  case GX_TL_IA8:
    texFmt = GX_TF_IA8;
    break;
  case GX_TL_RGB565:
    texFmt = GX_TF_RGB565;
    break;
  case GX_TL_RGB5A3:
    texFmt = GX_TF_RGB5A3;
    break;
  }
  auto* obj = reinterpret_cast<GXTlutObj_*>(obj_);
  obj->ref = porpoise::gfx::new_static_texture_2d(
      entries, 1, 1, texFmt,
      porpoise::ArrayRef<uint8_t>{static_cast<const uint8_t*>(data), static_cast<size_t>(entries) * 2}, true,
      "GXInitTlutObj");
  notify_texture_state();
}

void GXLoadTlut(const GXTlutObj* obj_, GXTlut idx) {
  g_gxState().tluts[idx] = *reinterpret_cast<const GXTlutObj_*>(obj_);
  // TODO stateDirty?
  notify_texture_state();
}

void GXInitTexCacheRegion(GXTexRegion* region, GXBool is32bMipmap, u32 tmemEven, GXTexCacheSize sizeEven,
                          u32 tmemOdd, GXTexCacheSize sizeOdd) {
  memset(region, 0, sizeof(*region));
  region->data[0] = static_cast<u32>(is32bMipmap);
  region->data[1] = tmemEven;
  region->data[2] = static_cast<u32>(sizeEven);
  region->data[3] = tmemOdd;
  region->data[4] = static_cast<u32>(sizeOdd);
}

void GXInitTexPreLoadRegion(GXTexRegion* region, u32 tmemEven, u32 sizeEven, u32 tmemOdd, u32 sizeOdd) {
  memset(region, 0, sizeof(*region));
  region->data[0] = tmemEven;
  region->data[1] = sizeEven;
  region->data[2] = tmemOdd;
  region->data[3] = sizeOdd;
}

void GXInitTlutRegion(GXTlutRegion* region, u32 tmemAddr, u32 tlutSize) {
  memset(region, 0, sizeof(*region));
  region->data[0] = tmemAddr;
  region->data[1] = tlutSize;
}

void GXInvalidateTexRegion(const GXTexRegion* region) {
  (void)region;
}

void GXInvalidateTexAll() {
  // no-op?
}

GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback callback) {
  auto prev = g_texRegionCallback;
  g_texRegionCallback = callback ? callback : default_tex_region;
  return prev;
}

GXTlutRegionCallback GXSetTlutRegionCallBack(GXTlutRegionCallback callback) {
  auto prev = g_tlutRegionCallback;
  g_tlutRegionCallback = callback ? callback : default_tlut_region;
  return prev;
}

void GXPreLoadEntireTexture(GXTexObj* texObj, GXTexRegion* region) {
  (void)region;
  if (!texObj) return;
  ensure_tex_uploaded(reinterpret_cast<GXTexObj_*>(texObj));
}

void GXLoadTexObjPreLoaded(GXTexObj* obj_, GXTexRegion* region, GXTexMapID id) {
  (void)region;
  auto* obj = reinterpret_cast<GXTexObj_*>(obj_);
  ensure_tex_uploaded(obj);
  g_gxState().textures[id] = {obj->ref};
  g_gxState().stateDirty = true;
}

void GXSetTexCoordScaleManually(GXTexCoordID coord, GXBool enable, u16 ss, u16 ts) {
  // TODO
}
// TODO GXSetTexCoordCylWrap
// TODO GXSetTexCoordBias
}
