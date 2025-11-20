#include "GXStubUtils.h"

#include <dolphin/gx/GXTransform.h>
#include "../gfx/gx_state.h"
#include "../gfx/gx_update_state.h"  // For update_gx_state
#include "../gfx/gx_helpers.h"
#include <string.h>  // For memcpy/memcmp

void GXSetProjection(const void* mtx, GXProjectionType type) {
    /* Copy from Aurora exactly: GXSetProjection calls update_gx_state */
    /* Aurora's code:
     *   const auto& mtx = *reinterpret_cast<const aurora::Mat4x4<float>*>(mtx_);
     *   g_gxState.projType = type;
     *   update_gx_state(g_gxState.proj, mtx);
     */
    
    if (!mtx) {
        OSReport("[GX ERROR] GXSetProjection: NULL matrix pointer\n");
        return;
    }
    
    /* Cast to 4x4 float matrix (Aurora uses Mat4x4<float>) */
    const f32(*mtxArray)[4] = (const f32(*)[4])mtx;
    
    /* Set projection type */
    update_gx_state(g_gxState->projType, (u32)type);
    
    /* Copy matrix and update state (Aurora's update_gx_state for Mat4x4) */
    /* For arrays, we need to manually copy and mark dirty */
    if (memcmp(g_gxState->proj, mtxArray, sizeof(f32) * 16) != 0) {
        memcpy(g_gxState->proj, mtxArray, sizeof(f32) * 16);
        GXStateMarkDirty();
    }
}

void GXSetProjectionv(const f32* proj) {
    GX_UNUSED(proj);
    GX_STUB_NOTICE("GXSetProjectionv");
}

static BOOL CopyIfChanged(void* dst, const void* src, size_t size)
{
    if (!dst || !src) {
        return FALSE;
    }
    if (memcmp(dst, src, size) == 0) {
        return FALSE;
    }
    memcpy(dst, src, size);
    return TRUE;
}

static int GetPnMtxSlot(u32 id)
{
    /* Position/normal matrix registers advance in steps of 3 */
    if ((id % 3) != 0) {
        return -1;
    }
    int slot = (int)(id / 3);
    if (slot < 0 || slot >= GX_PNMTX_COUNT) {
        return -1;
    }
    return slot;
}

static int GetTexMtxSlot(u32 id)
{
    if (id == GX_IDENTITY) {
        return GX_TEXMTX_COUNT; /* Special slot for identity */
    }
    if (id < GX_TEXMTX0 || id > GX_TEXMTX9) {
        return -1;
    }
    if (((id - GX_TEXMTX0) % 3) != 0) {
        return -1;
    }
    int slot = (int)((id - GX_TEXMTX0) / 3);
    if (slot < 0 || slot >= GX_TEXMTX_COUNT) {
        return -1;
    }
    return slot;
}

void GXLoadPosMtxImm(const void* mtx, u32 id) {
    /* Copy from Aurora exactly: GXLoadPosMtxImm uses update_gx_state */
    /* Aurora's code:
     *   CHECK(id >= GX_PNMTX0 && id <= GX_PNMTX9, "invalid pn mtx {}", static_cast<int>(id));
     *   auto& state = g_gxState.pnMtx[id / 3];
     *   const auto& mtx = *reinterpret_cast<const aurora::Mat3x4<float>*>(mtx_);
     *   update_gx_state(state.pos, mtx);
     */
    
    if (!mtx) {
        OSReport("[GX ERROR] GXLoadPosMtxImm: NULL matrix pointer\n");
        return;
    }
    
    if (id < GX_PNMTX0 || id > GX_PNMTX9) {
        OSReport("[GX ERROR] GXLoadPosMtxImm: Invalid matrix id %u (must be GX_PNMTX0-9)\n", id);
        return;
    }
    
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    
    /* Aurora uses id / 3 to get slot index */
    u32 slot = id / 3;
    if (slot >= GX_PNMTX_COUNT) {
        OSReport("[GX ERROR] GXLoadPosMtxImm: Slot %u out of range\n", slot);
        return;
    }
    
    /* Cast to Mtx (3x4 matrix) and use update_gx_state pattern (like Aurora) */
    const Mtx* mtxPtr = (const Mtx*)mtx;
    /* Use update_gx_state macro - it will memcmp for structs and mark dirty */
    if (memcmp(&state->pnMtx[slot].pos, mtxPtr, sizeof(Mtx)) != 0) {
        memcpy(&state->pnMtx[slot].pos, mtxPtr, sizeof(Mtx));
        GXStateMarkDirty();
    }
}

void GXLoadNrmMtxImm(const void* mtx, u32 id) {
    /* Copy from Aurora exactly: GXLoadNrmMtxImm uses update_gx_state */
    /* Aurora's code:
     *   CHECK(id >= GX_PNMTX0 && id <= GX_PNMTX9, "invalid pn mtx {}", static_cast<int>(id));
     *   auto& state = g_gxState.pnMtx[id / 3];
     *   const auto& mtx = *reinterpret_cast<const aurora::Mat3x4<float>*>(mtx_);
     *   update_gx_state(state.nrm, mtx);
     */
    
    if (!mtx) {
        OSReport("[GX ERROR] GXLoadNrmMtxImm: NULL matrix pointer\n");
        return;
    }
    
    if (id < GX_PNMTX0 || id > GX_PNMTX9) {
        OSReport("[GX ERROR] GXLoadNrmMtxImm: Invalid matrix id %u (must be GX_PNMTX0-9)\n", id);
        return;
    }
    
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    
    /* Aurora uses id / 3 to get slot index */
    u32 slot = id / 3;
    if (slot >= GX_PNMTX_COUNT) {
        OSReport("[GX ERROR] GXLoadNrmMtxImm: Slot %u out of range\n", slot);
        return;
    }
    
    /* Cast to Mtx (3x4 matrix) and use update_gx_state pattern (like Aurora) */
    const Mtx* mtxPtr = (const Mtx*)mtx;
    /* Use update_gx_state macro - it will memcmp for structs and mark dirty */
    if (memcmp(&state->pnMtx[slot].nrm, mtxPtr, sizeof(Mtx)) != 0) {
        memcpy(&state->pnMtx[slot].nrm, mtxPtr, sizeof(Mtx));
        GXStateMarkDirty();
    }
}

void GXSetCurrentMtx(u32 id) {
    /* Copy from Aurora exactly: GXSetCurrentMtx uses update_gx_state */
    /* Aurora's code:
     *   CHECK(id >= GX_PNMTX0 && id <= GX_PNMTX9, "invalid pn mtx {}", id);
     *   update_gx_state(g_gxState.currentPnMtx, id / 3);
     */
    
    if (id < GX_PNMTX0 || id > GX_PNMTX9) {
        OSReport("[GX ERROR] GXSetCurrentMtx: Invalid matrix id %u (must be GX_PNMTX0-9)\n", id);
        return;
    }
    
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    
    /* Aurora uses id / 3 to get slot index */
    u32 slot = id / 3;
    if (slot >= GX_PNMTX_COUNT) {
        OSReport("[GX ERROR] GXSetCurrentMtx: Slot %u out of range\n", slot);
        return;
    }
    
    update_gx_state(state->currentPnMtx, slot);
}

static void CopyTexMatrix(Mtx44 dst, const void* src, GXTexMtxType type)
{
    /* Start with identity then overwrite supplied rows */
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            dst[row][col] = (row == col) ? 1.0f : 0.0f;
        }
    }
    
    int rows = (type == GX_MTX2x4) ? 2 : 3;
    const f32* srcF32 = (const f32*)src;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < 4; ++c) {
            dst[r][c] = srcF32[r * 4 + c];
        }
    }
}

void GXLoadTexMtxImm(const void* mtx, u32 id, GXTexMtxType type) {
    if (!mtx) {
        OSReport("[GX ERROR] GXLoadTexMtxImm: NULL matrix pointer\n");
        return;
    }
    
    int slot = GetTexMtxSlot(id);
    if (slot == -1) {
        OSReport("[GX ERROR] GXLoadTexMtxImm: Invalid matrix id %u\n", id);
        return;
    }
    
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    
    Mtx44 temp;
    CopyTexMatrix(temp, mtx, type);
    
    if (slot == GX_TEXMTX_COUNT) {
        if (CopyIfChanged(state->texIdentityMtx, temp, sizeof(Mtx44))) {
            GXStateMarkDirty();
        }
        return;
    }
    
    if (CopyIfChanged(state->texMtx[slot], temp, sizeof(Mtx44))) {
        state->texMtxType[slot] = type;
        GXStateMarkDirty();
    }
}

void GXSetViewport(f32 left, f32 top, f32 width, f32 height, f32 nearZ, f32 farZ) {
    /* Copy from Aurora exactly: GXSetViewport calls aurora::gfx::set_viewport */
    /* Aurora's code: aurora::gfx::set_viewport(left, top, width, height, nearZ, farZ); */
    gfx_set_viewport(left, top, width, height, nearZ, farZ);
}

void GXSetViewportJitter(f32 left, f32 top, f32 width, f32 height, f32 nearZ, f32 farZ, u32 field) {
    GX_UNUSED(left);
    GX_UNUSED(top);
    GX_UNUSED(width);
    GX_UNUSED(height);
    GX_UNUSED(nearZ);
    GX_UNUSED(farZ);
    GX_UNUSED(field);
    GX_STUB_NOTICE("GXSetViewportJitter");
}


