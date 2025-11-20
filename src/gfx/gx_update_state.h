/*---------------------------------------------------------------------------*
  gx_update_state.h - Aurora's update_gx_state template function
  
  This mimics Aurora's update_gx_state template pattern.
  In C, we use a macro to simulate the reference-based syntax.
 *---------------------------------------------------------------------------*/

#ifndef GX_UPDATE_STATE_H
#define GX_UPDATE_STATE_H

#include <dolphin/types.h>
#include "gx_state.h"
#include "gx_state_helpers.h"  // For GXColorF32

#ifdef __cplusplus
extern "C" {
#endif

#include <dolphin/os.h>  // For OSReport
#include <string.h>  // For memcmp

/* Internal implementation - updates a value and marks state dirty
 * This matches Aurora's pattern: if (val != newVal) { val = newVal; g_gxState.stateDirty = true; }
 * 
 * Aurora's code:
 * template <typename T>
 * static inline void update_gx_state(T& val, T newVal) {
 *   if (val != newVal) {
 *     val = std::move(newVal);
 *     g_gxState.stateDirty = true;
 *   }
 * }
 */
static inline void update_gx_state_impl_u32(u32* val, u32 newVal) {
    if (val && *val != newVal) {
        *val = newVal;
        GXStateMarkDirty();
    }
}

static inline void update_gx_state_impl_f32(f32* val, f32 newVal) {
    if (val && *val != newVal) {
        *val = newVal;
        GXStateMarkDirty();
    }
}

static inline void update_gx_state_impl_bool(BOOL* val, BOOL newVal) {
    if (val && *val != newVal) {
        *val = newVal;
        GXStateMarkDirty();
    }
}

/* Vec4/GXColorF32 support - matches Aurora's Vec4<float> */
static inline void update_gx_state_impl_colorf32(GXColorF32* val, GXColorF32 newVal) {
    if (val && (val->r != newVal.r || val->g != newVal.g || 
                val->b != newVal.b || val->a != newVal.a)) {
        val->r = newVal.r;
        val->g = newVal.g;
        val->b = newVal.b;
        val->a = newVal.a;
        GXStateMarkDirty();
    }
}

/* TevPass support - struct with 4 u32 fields (16 bytes) */
static inline void update_gx_state_impl_tevpass(void* val, void* newVal) {
    if (val && newVal && memcmp(val, newVal, 16) != 0) {
        memcpy(val, newVal, 16);
        GXStateMarkDirty();
    }
}

/* TevOp support - struct with op, bias, scale, outReg, clamp, padding (20 bytes) */
static inline void update_gx_state_impl_tevop(void* val, void* newVal) {
    if (val && newVal && memcmp(val, newVal, 20) != 0) {
        memcpy(val, newVal, 20);
        GXStateMarkDirty();
    }
}

/* Helper function for MSVC to avoid type checking issues in macro */
static inline void update_gx_state_msvc_helper(void* field_ptr, void* newVal_ptr, size_t field_size) {
    if (field_size == 16) {
        /* Could be GXColorF32 or TevPass - check if it's a struct by comparing */
        if (memcmp(field_ptr, newVal_ptr, 16) != 0) {
            memcpy(field_ptr, newVal_ptr, 16);
            GXStateMarkDirty();
        }
    } else if (field_size == 20) {
        /* TevOp */
        update_gx_state_impl_tevop(field_ptr, newVal_ptr);
    } else if (field_size == sizeof(u32)) {
        /* u32 */
        update_gx_state_impl_u32((u32*)field_ptr, *(u32*)newVal_ptr);
    } else if (field_size == sizeof(f32)) {
        /* f32 */
        update_gx_state_impl_f32((f32*)field_ptr, *(f32*)newVal_ptr);
    } else if (field_size == sizeof(BOOL)) {
        /* BOOL */
        update_gx_state_impl_bool((BOOL*)field_ptr, *(BOOL*)newVal_ptr);
    } else {
        /* Default: use memcmp for structs */
        if (memcmp(field_ptr, newVal_ptr, field_size) != 0) {
            memcpy(field_ptr, newVal_ptr, field_size);
            GXStateMarkDirty();
        }
    }
}

/* Macro to mimic Aurora's update_gx_state(T& val, T newVal) syntax
 * Usage: update_gx_state(g_gxState->cullMode, mode)
 * This automatically takes the address of the field
 * 
 * Note: We use typeof extension (GCC/Clang) or fallback to u32 for MSVC
 * For Vec4/GXColorF32, we use memcmp to compare structs
 */
#if defined(__GNUC__) || defined(__clang__)
#define update_gx_state(field, newVal) \
    do { \
        typeof(field) _val = (newVal); \
        if (sizeof(field) == 16 || sizeof(field) == 20) { \
            /* Struct types (GXColorF32, TevPass, TevOp) - use memcmp */ \
            if (memcmp(&(field), &_val, sizeof(field)) != 0) { \
                (field) = _val; \
                GXStateMarkDirty(); \
            } \
        } else { \
            /* Primitive types - use != comparison */ \
            if ((field) != _val) { \
                (field) = _val; \
                GXStateMarkDirty(); \
            } \
        } \
    } while(0)
#else
/* MSVC fallback - use helper function to avoid type checking issues */
#define update_gx_state(field, newVal) \
    update_gx_state_msvc_helper(&(field), &(newVal), sizeof(field))
#endif

#ifdef __cplusplus
}
#endif

#endif /* GX_UPDATE_STATE_H */

