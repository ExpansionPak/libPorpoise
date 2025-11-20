#ifndef DOLPHIN_GXEXTRA_H
#define DOLPHIN_GXEXTRA_H

#include <dolphin/gx/GXStruct.h>
#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Extra types for PC
#ifdef TARGET_PC
typedef struct {
  float r;
  float g;
  float b;
  float a;
} GXColorF32;

typedef enum {
  GX_TF_R8_PC = 0x60,
  GX_TF_RGBA8_PC = 0x61,
} GXPCTexFmt;

void GXColor4f32(float r, float g, float b, float a);
#endif

void GXDestroyTexObj(GXTexObj* obj);
void GXDestroyTlutObj(GXTlutObj* obj);

#ifdef __cplusplus
}
#endif

#endif
