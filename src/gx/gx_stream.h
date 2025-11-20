/*---------------------------------------------------------------------------*
  gx_stream.h - GX Stream State
  
  Internal header for stream-based vertex processing (similar to Aurora)
 *---------------------------------------------------------------------------*/

#ifndef GX_STREAM_H
#define GX_STREAM_H

#include <dolphin/types.h>
#include <dolphin/gx/GXEnum.h>

/* Stream state structure (similar to Aurora's SStreamState) */
typedef struct StreamState {
    GXPrimitive primitive;
    GXVtxFmt vtxFmt;
    u16 vertexCount;
    u16 vertexStart;
    u16 vertexSize;
    u8* vertexBuffer;
    u32 vertexBufferSize;
    u32 vertexBufferCapacity;
    u16* indices;
    u32 indicesCount;
    u32 indicesCapacity;
    u16 curAttr;
    BOOL active;
} StreamState;

/* Global stream state (exposed for GXVert.c) */
extern StreamState* g_streamState;

#endif /* GX_STREAM_H */

