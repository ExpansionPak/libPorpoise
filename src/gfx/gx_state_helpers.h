/*---------------------------------------------------------------------------*
  gx_state_helpers.h - GX State Helper Functions (Aurora-style)
  
  These functions mirror Aurora's update_gx_state() and from_gx_color()
 *---------------------------------------------------------------------------*/

#ifndef GX_STATE_HELPERS_H
#define GX_STATE_HELPERS_H

#include <dolphin/types.h>
#include <dolphin/gx/GXStruct.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Color structure (similar to Aurora's Vec4<float>) */
typedef struct {
    f32 r;
    f32 g;
    f32 b;
    f32 a;
} GXColorF32;

/* Helper function matching Aurora's update_gx_state template */
/* For different types, we'll need type-specific versions */
void gx_update_state_u8(u8* val, u8 newVal);
void gx_update_state_u32(u32* val, u32 newVal);
void gx_update_state_f32(f32* val, f32 newVal);
void gx_update_state_bool(BOOL* val, BOOL newVal);

/* Type-specific update functions for GX enums */
void gx_update_state_cullmode(u32* val, u32 newVal);
void gx_update_state_blendmode(u32* val, u32 newVal);
void gx_update_state_blendfactor(u32* val, u32 newVal);
void gx_update_state_logicop(u32* val, u32 newVal);

/* Helper function matching Aurora's from_gx_color */
GXColorF32 gx_from_gx_color(GXColor color);

#ifdef __cplusplus
}
#endif

#endif /* GX_STATE_HELPERS_H */

