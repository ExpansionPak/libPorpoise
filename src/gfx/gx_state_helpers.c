/*---------------------------------------------------------------------------*
  gx_state_helpers.c - GX State Helper Functions (Aurora-style)
  
  Implements update_gx_state() and from_gx_color() matching Aurora exactly
 *---------------------------------------------------------------------------*/

#include "gx_state_helpers.h"
#include "gx_state.h"
#include <dolphin/os.h>
#include <dolphin/gx/GXEnum.h>

/*---------------------------------------------------------------------------*
  Name:         gx_update_state_* (various types)
  
  Description:  Updates GX state value and marks state as dirty.
                Matches Aurora's update_gx_state<T>() template.
                
                Shows alert if state update system not fully implemented.
  
  Arguments:    val - Pointer to current value
                newVal - New value to set
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gx_update_state_u8(u8* val, u8 newVal) {
    if (!val) return;
    if (*val != newVal) {
        *val = newVal;
        GXStateMarkDirty();
    }
}

void gx_update_state_u32(u32* val, u32 newVal) {
    if (!val) return;
    if (*val != newVal) {
        *val = newVal;
        GXStateMarkDirty();
    }
}

void gx_update_state_f32(f32* val, f32 newVal) {
    if (!val) return;
    if (*val != newVal) {
        *val = newVal;
        GXStateMarkDirty();
    }
}

void gx_update_state_bool(BOOL* val, BOOL newVal) {
    if (!val) return;
    if (*val != newVal) {
        *val = newVal;
        GXStateMarkDirty();
    }
}

/* Type-specific update functions that access GXState fields */
void gx_update_state_cullmode(u32* val, u32 newVal) {
    if (!val) return;
    if (*val != newVal) {
        *val = newVal;
        /* Aurora calls: update_gx_state(g_gxState.cullMode, mode); */
        /* Update the actual GXState field */
        GXStateSetCullMode(newVal);
    }
}

void gx_update_state_blendmode(u32* val, u32 newVal) {
    if (!val) return;
    if (*val != newVal) {
        *val = newVal;
        /* Aurora calls: update_gx_state(g_gxState.blendMode, mode); */
        /* Update the actual GXState field */
        GXStateSetBlendMode(newVal);
    }
}

void gx_update_state_blendfactor(u32* val, u32 newVal) {
    if (!val) return;
    if (*val != newVal) {
        *val = newVal;
        GXStateMarkDirty();
    }
}

void gx_update_state_logicop(u32* val, u32 newVal) {
    if (!val) return;
    if (*val != newVal) {
        *val = newVal;
        GXStateMarkDirty();
    }
}

/*---------------------------------------------------------------------------*
  Name:         gx_from_gx_color
  
  Description:  Converts GXColor (u8 0-255) to normalized float (0.0-1.0).
                Matches Aurora's from_gx_color() exactly.
  
  Arguments:    color - GXColor to convert
  
  Returns:      GXColorF32 with normalized values
 *---------------------------------------------------------------------------*/
GXColorF32 gx_from_gx_color(GXColor color) {
    /* Copy from Aurora exactly:
     * return {
     *     static_cast<float>(color.r) / 255.f,
     *     static_cast<float>(color.g) / 255.f,
     *     static_cast<float>(color.b) / 255.f,
     *     static_cast<float>(color.a) / 255.f,
     * };
     */
    GXColorF32 result;
    result.r = (f32)color.r / 255.0f;
    result.g = (f32)color.g / 255.0f;
    result.b = (f32)color.b / 255.0f;
    result.a = (f32)color.a / 255.0f;
    return result;
}

