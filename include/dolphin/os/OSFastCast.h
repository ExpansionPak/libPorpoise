#ifndef DOLPHIN_OSFASTCAST_H
#define DOLPHIN_OSFASTCAST_H

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void OSInitFastCast(void);

void OSf32tos16(f32* in, s16* out);
void OSf32tos8(f32* in, s8* out);
void OSf32tou16(f32* in, u16* out);
void OSf32tou8(f32* in, u8* out);

void OSs16tof32(s16* in, f32* out);
void OSs8tof32(s8* in, f32* out);
void OSu16tof32(u16* in, f32* out);
void OSu8tof32(u8* in, f32* out);

#ifdef __cplusplus
}
#endif

#endif /* DOLPHIN_OSFASTCAST_H */
