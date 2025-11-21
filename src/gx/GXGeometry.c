#include "GXStubUtils.h"
#include "gx_stream.h"

#include <dolphin/gx/GXGeometry.h>
#include "../gfx/gx_state.h"
#include "../gfx/gx_update_state.h"  // For update_gx_state
#include "../gfx/gx_helpers.h"  // For gfx_push_command
#include <dolphin/os.h>
#include <stdlib.h>
#include <string.h>

void GXSetVtxDesc(GXAttr attr, GXAttrType type) {
    /* Copy from Aurora exactly: GXSetVtxDesc calls update_gx_state */
    /* Aurora's code: update_gx_state(g_gxState.vtxDesc[attr], type); */
    
    if (attr >= GX_VA_MAX_ATTR) {
        OSReport("[GX ERROR] GXSetVtxDesc: Invalid attribute %d (max is %d)\n", attr, GX_VA_MAX_ATTR - 1);
        return;
    }
    
    update_gx_state(g_gxState->vtxDesc[attr], (u32)type);
}

void GXSetVtxDescv(GXVtxDescList* list) {
    GX_UNUSED(list);
    GX_STUB_NOTICE("GXSetVtxDescv");
}

void GXClearVtxDesc(void) {
    /* Copy from Aurora exactly: GXClearVtxDesc clears vtxDesc array */
    /* Aurora's code: g_gxState.vtxDesc.fill({}); */
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    memset(state->vtxDesc, 0, sizeof(state->vtxDesc));
    GXStateMarkDirty();
}

void GXSetVtxAttrFmt(GXVtxFmt vtxfmt, GXAttr attr, GXCompCnt cnt, GXCompType type, u8 frac) {
    /* Copy from Aurora exactly: GXSetVtxAttrFmt uses update_gx_state for each field */
    /* Aurora's code:
     *   CHECK(vtxfmt >= GX_VTXFMT0 && vtxfmt < GX_MAX_VTXFMT, "invalid vtxfmt {}", underlying(vtxfmt));
     *   CHECK(attr >= GX_VA_PNMTXIDX && attr < GX_VA_MAX_ATTR, "invalid attr {}", underlying(attr));
     *   auto& fmt = g_gxState.vtxFmts[vtxfmt].attrs[attr];
     *   update_gx_state(fmt.cnt, cnt);
     *   update_gx_state(fmt.type, type);
     *   update_gx_state(fmt.frac, frac);
     */
    
    if (vtxfmt < GX_VTXFMT0 || vtxfmt >= GX_MAX_VTXFMT) {
        OSReport("[GX ERROR] GXSetVtxAttrFmt: Invalid vtxfmt %u\n", vtxfmt);
        return;
    }
    
    if (attr >= GX_VA_MAX_ATTR) {
        OSReport("[GX ERROR] GXSetVtxAttrFmt: Invalid attr %u\n", attr);
        return;
    }
    
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    
    VtxAttrFmt* fmt = &state->vtxFmts[vtxfmt].attrs[attr];
    update_gx_state(fmt->cnt, (u32)cnt);
    update_gx_state(fmt->type, (u32)type);
    update_gx_state(fmt->frac, frac);
}

void GXSetNumTexGens(u8 num) {
    /* Copy from Aurora exactly: GXSetNumTexGens uses update_gx_state */
    /* Aurora's code: update_gx_state(g_gxState.numTexGens, num); */
    update_gx_state(g_gxState->numTexGens, num);
}

void GXSetTexCoordGen2(GXTexCoordID dst, GXTexGenType type, GXTexGenSrc src, u32 mtx, GXBool normalize, u32 postMtx) {
    GX_UNUSED(dst);
    GX_UNUSED(type);
    GX_UNUSED(src);
    GX_UNUSED(mtx);
    GX_UNUSED(normalize);
    GX_UNUSED(postMtx);
    GX_STUB_NOTICE("GXSetTexCoordGen2");
}

void GXSetLineWidth(u8 width, GXTexOffset offs) {
    GX_UNUSED(width);
    GX_UNUSED(offs);
    GX_STUB_NOTICE("GXSetLineWidth");
}

void GXSetPointSize(u8 pointSize, GXTexOffset texOffsets) {
    GX_UNUSED(pointSize);
    GX_UNUSED(texOffsets);
    GX_STUB_NOTICE("GXSetPointSize");
}

void GXEnableTexOffsets(GXTexCoordID coord, GXBool line_enable, GXBool point_enable) {
    GX_UNUSED(coord);
    GX_UNUSED(line_enable);
    GX_UNUSED(point_enable);
    GX_STUB_NOTICE("GXEnableTexOffsets");
}

/* 4-parameter version (TARGET_PC) - matches Aurora exactly */
/* 3-parameter version (GameCube SDK standard) - matches original SDK exactly */
void GXSetArray(GXAttr attr, const void* data, u8 stride) {
    /* Original GameCube SDK signature: 3 parameters (attr, base_ptr, stride) */
    /* Size is not provided - we infer it dynamically from the maximum index used during rendering */
    /* This allows compatibility with unmodified demo code */
    
    if (attr >= GX_VA_MAX_ATTR) {
        OSReport("[GX ERROR] GXSetArray: Invalid attr %u\n", attr);
        return;
    }
    
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    
    AttrArray* array = &state->arrays[attr];
    if (array->data != data || array->stride != stride) {
        array->data = data;
        array->stride = stride;
        array->size = 0;  /* 0 = unknown size, will be inferred from max index during rendering */
        GXStateMarkDirty();
    }
}

#ifdef TARGET_PC
/* 4-parameter version (Aurora extension) - for code that can be modified */
/* This is provided for compatibility with Aurora-style code, but the 3-param version is preferred */
void GXSetArray_4Param(GXAttr attr, const void* data, u32 size, u8 stride) {
    /* Aurora extension: 4 parameters (adds size for explicit size specification) */
    /* If size is provided, we use it; otherwise fall back to 3-param version */
    
    if (attr >= GX_VA_MAX_ATTR) {
        OSReport("[GX ERROR] GXSetArray_4Param: Invalid attr %u\n", attr);
        return;
    }
    
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    
    AttrArray* array = &state->arrays[attr];
    if (array->data != data || array->size != size || array->stride != stride) {
        array->data = data;
        array->stride = stride;
        array->size = size;  /* Use provided size, or 0 if unknown */
        GXStateMarkDirty();
    }
}
#endif

void GXInvalidateVtxCache(void) {
    /* Copy from Aurora exactly: GXInvalidateVtxCache is a TODO/no-op */
    /* Aurora's code: // TODO */
    /* No-op for now */
}

/* Stream state - exposed for GXVert.c */
StreamState* g_streamState = NULL;
static u16 lastVertexStart = 0;

/* Helper to calculate vertex size from vertex descriptor */
static u16 CalculateVertexSize(void) {
    u16 vertexSize = 0;
    u16 numIndexedAttrs = 0;
    
    for (GXAttr attr = 0; attr < GX_VA_MAX_ATTR; attr++) {
        u32 type = g_gxState->vtxDesc[attr];
        if (type == GX_DIRECT) {
            if (attr == GX_VA_POS || attr == GX_VA_NRM) {
                vertexSize += 12;  // 3 floats
            } else if (attr == GX_VA_CLR0 || attr == GX_VA_CLR1) {
                vertexSize += 16;  // 4 floats (RGBA)
            } else if (attr >= GX_VA_TEX0 && attr <= GX_VA_TEX7) {
                vertexSize += 8;   // 2 floats (ST)
            }
        } else if (type == GX_INDEX8 || type == GX_INDEX16) {
            numIndexedAttrs++;
        }
    }
    
    /* Calculate index area size (similar to Aurora's num4xAttr/num2xAttr logic) */
    u32 num4xAttr = numIndexedAttrs / 4;
    u32 rem = numIndexedAttrs % 4;
    u32 num2xAttr = 0;
    if (rem > 2) {
        num4xAttr++;
    } else if (rem > 0) {
        num2xAttr++;
    }
    u32 directStart = num4xAttr * 8 + num2xAttr * 4;
    vertexSize += directStart;
    
    return vertexSize;
}

void GXBegin(GXPrimitive primitive, GXVtxFmt vtxFmt, u16 nVerts) {
    /* Check if display list is active (similar to Aurora's dynamicDlBuf check) */
    /* For now, we'll skip display list handling */
    
    /* Check if stream already active */
    if (g_streamState != NULL && g_streamState->active) {
        OSReport("[GX ERROR] GXBegin: Stream began twice!\n");
        return;
    }
    
    /* Allocate stream state if needed */
    if (g_streamState == NULL) {
        g_streamState = (StreamState*)malloc(sizeof(StreamState));
        if (g_streamState == NULL) {
            OSReport("[GX ERROR] GXBegin: Failed to allocate stream state\n");
            return;
        }
        memset(g_streamState, 0, sizeof(StreamState));
    }
    
    /* Calculate vertex size */
    u16 vertexSize = CalculateVertexSize();
    if (vertexSize == 0) {
        OSReport("[GX ERROR] GXBegin: No vertex attributes enabled\n");
        return;
    }
    
    /* Initialize stream state (similar to Aurora's SStreamState constructor) */
    g_streamState->primitive = primitive;
    g_streamState->vtxFmt = vtxFmt;
    g_streamState->vertexCount = 0;
    g_streamState->vertexStart = 0;  /* TODO: Use lastVertexStart if state not dirty */
    g_streamState->vertexSize = vertexSize;
    g_streamState->curAttr = 0;
    g_streamState->active = TRUE;
    
    /* Allocate vertex buffer - free old one first if it exists */
    if (g_streamState->vertexBuffer) {
        free(g_streamState->vertexBuffer);
        g_streamState->vertexBuffer = NULL;
    }
    u32 estimatedVerts = nVerts;
    g_streamState->vertexBufferCapacity = estimatedVerts * vertexSize;
    g_streamState->vertexBuffer = (u8*)malloc(g_streamState->vertexBufferCapacity);
    if (g_streamState->vertexBuffer == NULL) {
        OSReport("[GX ERROR] GXBegin: Failed to allocate vertex buffer\n");
        g_streamState->active = FALSE;
        return;
    }
    g_streamState->vertexBufferSize = 0;
    
    /* Allocate index buffer (estimate based on primitive type) */
    if (nVerts > 3 && (primitive == GX_TRIANGLEFAN || primitive == GX_TRIANGLESTRIP)) {
        g_streamState->indicesCapacity = ((nVerts - 3) * 3) + 3;
    } else if (nVerts > 4 && primitive == GX_QUADS) {
        g_streamState->indicesCapacity = (nVerts / 4) * 6;
    } else {
        g_streamState->indicesCapacity = nVerts;
    }
    /* Free old index buffer if it exists */
    if (g_streamState->indices) {
        free(g_streamState->indices);
        g_streamState->indices = NULL;
    }
    g_streamState->indices = (u16*)malloc(g_streamState->indicesCapacity * sizeof(u16));
    if (g_streamState->indices == NULL) {
        OSReport("[GX ERROR] GXBegin: Failed to allocate index buffer\n");
        free(g_streamState->vertexBuffer);
        g_streamState->vertexBuffer = NULL;
        g_streamState->active = FALSE;
        return;
    }
    g_streamState->indicesCount = 0;
}

void GXEnd(void) {
    if (g_streamState == NULL || !g_streamState->active) {
        OSReport("[GX WARN] GXEnd: No active stream\n");
        return;
    }
    
    /* If no vertices, just reset (similar to Aurora) */
    if (g_streamState->vertexCount == 0) {
        OSReport("[GX WARN] GXEnd: No vertices in stream\n");
        g_streamState->active = FALSE;
        return;
    }
    
    /* Copy from Aurora exactly: GXEnd queues a draw command */
    /* Aurora's code:
     *   const auto vertRange = aurora::gfx::push_verts(...);
     *   const auto indexRange = aurora::gfx::push_indices(...);
     *   aurora::gfx::push_draw_command(aurora::gfx::model::DrawData{...});
     */
    
    /* For now, queue a simplified draw command with the vertex data */
    /* We'll copy the data since the stream state may be reused */
    DrawCommand drawCmd;
    drawCmd.primitive = g_streamState->primitive;
    drawCmd.vertexDataSize = g_streamState->vertexBufferSize;
    
    /* Allocate and copy vertex data */
    if (drawCmd.vertexDataSize > 0) {
        drawCmd.vertexData = (u8*)malloc(drawCmd.vertexDataSize);
        if (drawCmd.vertexData) {
            memcpy(drawCmd.vertexData, g_streamState->vertexBuffer, drawCmd.vertexDataSize);
        }
    } else {
        drawCmd.vertexData = NULL;
    }
    
    /* Allocate and copy index data */
    drawCmd.indexCount = g_streamState->indicesCount;
    if (drawCmd.indexCount > 0) {
        drawCmd.indices = (u16*)malloc(drawCmd.indexCount * sizeof(u16));
        if (drawCmd.indices) {
            memcpy(drawCmd.indices, g_streamState->indices, drawCmd.indexCount * sizeof(u16));
        }
    } else {
        drawCmd.indices = NULL;
    }
    
    /* Queue the draw command */
    gfx_push_command(COMMAND_TYPE_DRAW, &drawCmd);
    
    lastVertexStart = g_streamState->vertexStart + g_streamState->vertexCount;
    g_streamState->active = FALSE;
    
    /* Note: We don't free the stream buffers here - they may be reused */
    /* The draw command now owns copies of the data */
}


