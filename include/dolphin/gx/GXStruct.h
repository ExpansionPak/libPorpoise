#ifndef DOLPHIN_GXSTRUCT_H
#define DOLPHIN_GXSTRUCT_H

#include <dolphin/types.h>
#include <dolphin/gx/GXEnum.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VITVMode VITVMode;
typedef enum VIXFBMode VIXFBMode;

struct GXRenderModeObj {
  /*0x00*/ VITVMode viTVmode;
  /*0x04*/ u16 fbWidth;
  /*0x06*/ u16 efbHeight;
  /*0x08*/ u16 xfbHeight;
  /*0x0A*/ u16 viXOrigin;
  /*0x0C*/ u16 viYOrigin;
  /*0x0E*/ u16 viWidth;
  /*0x10*/ u16 viHeight;
  /*0x14*/ VIXFBMode xFBmode;
  /*0x18*/ u8 field_rendering;
  u8 aa;
  u8 sample_pattern[12][2];
  u8 vfilter[7];
};
typedef struct GXRenderModeObj GXRenderModeObj;

typedef struct {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
} GXColor;

typedef struct {
#ifdef TARGET_PC
  u32 dummy[22];
#else
  u32 dummy[8];
#endif
} GXTexObj;

typedef struct {
#ifdef TARGET_PC
  u32 dummy[4];
#else
  u32 dummy[3];
#endif
} GXTlutObj;

typedef struct {
  u32 dummy[16];
} GXLightObj;

typedef struct {
  GXAttr attr;
  GXAttrType type;
} GXVtxDescList;

typedef struct {
  GXAttr attr;
  GXCompCnt cnt;
  GXCompType type;
  u8 frac;
} GXVtxAttrFmtList;

typedef struct {
  s16 r;
  s16 g;
  s16 b;
  s16 a;
} GXColorS10;

typedef struct _GXTexRegion {
  u32 dummy[4];
} GXTexRegion;

typedef struct _GXTlutRegion {
  u32 dummy[4];
} GXTlutRegion;

#ifdef __cplusplus
}
#endif

#endif
