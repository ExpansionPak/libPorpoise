#ifndef DOLPHIN_GXDRAW_H
#define DOLPHIN_GXDRAW_H

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void GXDrawSphere(u8 numMajor, u8 numMinor);
void GXDrawSphere1(u8 depth);

#ifdef __cplusplus
}
#endif

#endif
