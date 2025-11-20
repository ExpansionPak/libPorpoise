/*---------------------------------------------------------------------------*
  GXVert.c - GX Vertex Functions
  
  Based on reverse-engineered Dolphin emulator GX implementation.
  Recreates the original GameCube/Wii GX vertex functions.
  
  Source: c:\rabin\dolphin\build\libraries\gx\src\gxvert.c
 *---------------------------------------------------------------------------*/

#include "GXStubUtils.h"
#include "gx_stream.h"
#include <dolphin/gx/GXVert.h>
#include "../gfx/gx_state.h"
#include <dolphin/os.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*
    External Variables (for emulator display list support)
 *---------------------------------------------------------------------------*/

u8* __EmBuffPtr = NULL;
u8* __EmBuffTop = NULL;
BOOL __EmDisplayListInProgress = FALSE;

/*---------------------------------------------------------------------------*
    Stub helpers for emulator functions
 *---------------------------------------------------------------------------*/

static void GXStubOnce(const char* name)
{
    enum { kMaxStubNames = 256 };
    static const char* seen[kMaxStubNames];
    static u32 seenCount = 0;
    
    for (u32 i = 0; i < seenCount; ++i) {
        if (seen[i] == name) {
            return;
        }
    }
    
    if (seenCount < kMaxStubNames) {
        seen[seenCount++] = name;
    }
    
    OSReport("[GX STUB] %s() - Not yet implemented\n", name);
}

#define DEFINE_EM_STUB(fn, body) \
    static void fn body

DEFINE_EM_STUB(EmGXCmd1u8, (u8 cmd))
{
    GX_UNUSED(cmd);
    GXStubOnce("EmGXCmd1u8");
}

DEFINE_EM_STUB(EmGXCmd1u16, (u16 cmd))
{
    GX_UNUSED(cmd);
    GXStubOnce("EmGXCmd1u16");
}

DEFINE_EM_STUB(EmGXCmd1u32, (u32 cmd))
{
    GX_UNUSED(cmd);
    GXStubOnce("EmGXCmd1u32");
}

DEFINE_EM_STUB(EmGXParam1u8, (u8 param))
{
    GX_UNUSED(param);
    GXStubOnce("EmGXParam1u8");
}

DEFINE_EM_STUB(EmGXParam1u16, (u16 param))
{
    GX_UNUSED(param);
    GXStubOnce("EmGXParam1u16");
}

DEFINE_EM_STUB(EmGXParam1u32, (u32 param))
{
    GX_UNUSED(param);
    GXStubOnce("EmGXParam1u32");
}

DEFINE_EM_STUB(EmGXParam1s8, (s8 param))
{
    GX_UNUSED(param);
    GXStubOnce("EmGXParam1s8");
}

DEFINE_EM_STUB(EmGXParam1s16, (s16 param))
{
    GX_UNUSED(param);
    GXStubOnce("EmGXParam1s16");
}

DEFINE_EM_STUB(EmGXParam1s32, (s32 param))
{
    GX_UNUSED(param);
    GXStubOnce("EmGXParam1s32");
}

DEFINE_EM_STUB(EmGXParam1f32, (f32 param))
{
    GX_UNUSED(param);
    GXStubOnce("EmGXParam1f32");
}

DEFINE_EM_STUB(EmGXParam3f32, (f32 x, f32 y, f32 z))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GXStubOnce("EmGXParam3f32");
}

DEFINE_EM_STUB(EmGXParam4f32, (f32 x, f32 y, f32 z, f32 w))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GX_UNUSED(w);
    GXStubOnce("EmGXParam4f32");
}

DEFINE_EM_STUB(EmGXPosition3f32, (f32 x, f32 y, f32 z))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GXStubOnce("EmGXPosition3f32");
}

DEFINE_EM_STUB(EmGXPosition3u8, (u8 x, u8 y, u8 z))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GXStubOnce("EmGXPosition3u8");
}

DEFINE_EM_STUB(EmGXPosition3s8, (s8 x, s8 y, s8 z))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GXStubOnce("EmGXPosition3s8");
}

DEFINE_EM_STUB(EmGXPosition3u16, (u16 x, u16 y, u16 z))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GXStubOnce("EmGXPosition3u16");
}

DEFINE_EM_STUB(EmGXPosition3s16, (s16 x, s16 y, s16 z))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GXStubOnce("EmGXPosition3s16");
}

DEFINE_EM_STUB(EmGXPosition2f32, (f32 x, f32 y))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GXStubOnce("EmGXPosition2f32");
}

DEFINE_EM_STUB(EmGXPosition2u8, (u8 x, u8 y))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GXStubOnce("EmGXPosition2u8");
}

DEFINE_EM_STUB(EmGXPosition2s8, (s8 x, s8 y))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GXStubOnce("EmGXPosition2s8");
}

DEFINE_EM_STUB(EmGXPosition2u16, (u16 x, u16 y))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GXStubOnce("EmGXPosition2u16");
}

DEFINE_EM_STUB(EmGXPosition2s16, (s16 x, s16 y))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GXStubOnce("EmGXPosition2s16");
}

DEFINE_EM_STUB(EmGXPosition1x16, (u16 index))
{
    GX_UNUSED(index);
    GXStubOnce("EmGXPosition1x16");
}

DEFINE_EM_STUB(EmGXPosition1x8, (u8 index))
{
    /* Copy from Aurora exactly: GXPosition1x8 uses stream->check_indexed and stream->append<u16> */
    /* Aurora's code:
     *   stream->check_indexed(GX_VA_POS, GX_INDEX8);
     *   stream->append<u16>(idx);
     */
    
    if (g_streamState == NULL || !g_streamState->active) {
        OSReport("[GX ERROR] EmGXPosition1x8: No active stream\n");
        return;
    }
    
    /* Validate attribute type (similar to Aurora's check_indexed) */
    if (g_gxState->vtxDesc[GX_VA_POS] != GX_INDEX8) {
        OSReport("[GX ERROR] EmGXPosition1x8: Position attribute not set to GX_INDEX8\n");
        return;
    }
    
    /* Append index as u16 to vertex buffer (Aurora stores u8 index as u16) */
    /* For now, append to the end of the buffer - proper offset calculation will come later */
    if (g_streamState->vertexBufferSize + sizeof(u16) > g_streamState->vertexBufferCapacity) {
        /* Grow buffer if needed */
        u32 newCapacity = g_streamState->vertexBufferCapacity * 2;
        if (newCapacity < g_streamState->vertexBufferSize + sizeof(u16)) {
            newCapacity = g_streamState->vertexBufferSize + sizeof(u16);
        }
        u8* newBuffer = (u8*)realloc(g_streamState->vertexBuffer, newCapacity);
        if (newBuffer == NULL) {
            OSReport("[GX ERROR] EmGXPosition1x8: Failed to grow vertex buffer\n");
            return;
        }
        g_streamState->vertexBuffer = newBuffer;
        g_streamState->vertexBufferCapacity = newCapacity;
    }
    
    /* Append index as u16 (Aurora stores u8 as u16) */
    u16* idxPtr = (u16*)(g_streamState->vertexBuffer + g_streamState->vertexBufferSize);
    *idxPtr = (u16)index;
    g_streamState->vertexBufferSize += sizeof(u16);
    
    /* Move to next attribute (similar to Aurora's next_attribute) */
    g_streamState->curAttr++;
}

DEFINE_EM_STUB(EmGXNormal3f32, (f32 x, f32 y, f32 z))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GXStubOnce("EmGXNormal3f32");
}

DEFINE_EM_STUB(EmGXNormal3s16, (s16 x, s16 y, s16 z))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GXStubOnce("EmGXNormal3s16");
}

DEFINE_EM_STUB(EmGXNormal3s8, (s8 x, s8 y, s8 z))
{
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GXStubOnce("EmGXNormal3s8");
}

DEFINE_EM_STUB(EmGXNormal1x16, (u16 index))
{
    GX_UNUSED(index);
    GXStubOnce("EmGXNormal1x16");
}

DEFINE_EM_STUB(EmGXNormal1x8, (u8 index))
{
    GX_UNUSED(index);
    GXStubOnce("EmGXNormal1x8");
}

DEFINE_EM_STUB(EmGXColor4u8, (u8 r, u8 g, u8 b, u8 a))
{
    GX_UNUSED(r);
    GX_UNUSED(g);
    GX_UNUSED(b);
    GX_UNUSED(a);
    GXStubOnce("EmGXColor4u8");
}

DEFINE_EM_STUB(EmGXColor1u32, (u32 color))
{
    GX_UNUSED(color);
    GXStubOnce("EmGXColor1u32");
}

DEFINE_EM_STUB(EmGXColor3u8, (u8 r, u8 g, u8 b))
{
    GX_UNUSED(r);
    GX_UNUSED(g);
    GX_UNUSED(b);
    GXStubOnce("EmGXColor3u8");
}

DEFINE_EM_STUB(EmGXColor1u16, (u16 color))
{
    GX_UNUSED(color);
    GXStubOnce("EmGXColor1u16");
}

DEFINE_EM_STUB(EmGXColor1x16, (u16 index))
{
    GX_UNUSED(index);
    GXStubOnce("EmGXColor1x16");
}

DEFINE_EM_STUB(EmGXColor1x8, (u8 index))
{
    /* Copy from Aurora exactly: GXColor1x8 uses stream->check_indexed and stream->append<u16> */
    /* Aurora's code:
     *   stream->check_indexed(GX_VA_CLR0, GX_INDEX8);
     *   stream->append<u16>(index);
     */
    
    if (g_streamState == NULL || !g_streamState->active) {
        OSReport("[GX ERROR] EmGXColor1x8: No active stream\n");
        return;
    }
    
    /* Validate attribute type (similar to Aurora's check_indexed) */
    if (g_gxState->vtxDesc[GX_VA_CLR0] != GX_INDEX8) {
        OSReport("[GX ERROR] EmGXColor1x8: Color attribute not set to GX_INDEX8\n");
        return;
    }
    
    /* Append index as u16 to vertex buffer (Aurora stores u8 index as u16) */
    /* For now, append to the end of the buffer - proper offset calculation will come later */
    if (g_streamState->vertexBufferSize + sizeof(u16) > g_streamState->vertexBufferCapacity) {
        /* Grow buffer if needed */
        u32 newCapacity = g_streamState->vertexBufferCapacity * 2;
        if (newCapacity < g_streamState->vertexBufferSize + sizeof(u16)) {
            newCapacity = g_streamState->vertexBufferSize + sizeof(u16);
        }
        u8* newBuffer = (u8*)realloc(g_streamState->vertexBuffer, newCapacity);
        if (newBuffer == NULL) {
            OSReport("[GX ERROR] EmGXColor1x8: Failed to grow vertex buffer\n");
            return;
        }
        g_streamState->vertexBuffer = newBuffer;
        g_streamState->vertexBufferCapacity = newCapacity;
    }
    
    /* Append index as u16 (Aurora stores u8 as u16) */
    u16* idxPtr = (u16*)(g_streamState->vertexBuffer + g_streamState->vertexBufferSize);
    *idxPtr = (u16)index;
    g_streamState->vertexBufferSize += sizeof(u16);
    
    /* Move to next attribute (similar to Aurora's next_attribute) */
    g_streamState->curAttr++;
}

DEFINE_EM_STUB(EmGXTexCoord2f32, (f32 s, f32 t))
{
    GX_UNUSED(s);
    GX_UNUSED(t);
    GXStubOnce("EmGXTexCoord2f32");
}

DEFINE_EM_STUB(EmGXTexCoord2s16, (s16 s, s16 t))
{
    GX_UNUSED(s);
    GX_UNUSED(t);
    GXStubOnce("EmGXTexCoord2s16");
}

DEFINE_EM_STUB(EmGXTexCoord2u16, (u16 s, u16 t))
{
    GX_UNUSED(s);
    GX_UNUSED(t);
    GXStubOnce("EmGXTexCoord2u16");
}

DEFINE_EM_STUB(EmGXTexCoord2s8, (s8 s, s8 t))
{
    GX_UNUSED(s);
    GX_UNUSED(t);
    GXStubOnce("EmGXTexCoord2s8");
}

DEFINE_EM_STUB(EmGXTexCoord2u8, (u8 s, u8 t))
{
    GX_UNUSED(s);
    GX_UNUSED(t);
    GXStubOnce("EmGXTexCoord2u8");
}

DEFINE_EM_STUB(EmGXTexCoord1f32, (f32 s))
{
    GX_UNUSED(s);
    GXStubOnce("EmGXTexCoord1f32");
}

DEFINE_EM_STUB(EmGXTexCoord1s16, (s16 s))
{
    GX_UNUSED(s);
    GXStubOnce("EmGXTexCoord1s16");
}

DEFINE_EM_STUB(EmGXTexCoord1u16, (u16 s))
{
    GX_UNUSED(s);
    GXStubOnce("EmGXTexCoord1u16");
}

DEFINE_EM_STUB(EmGXTexCoord1s8, (s8 s))
{
    GX_UNUSED(s);
    GXStubOnce("EmGXTexCoord1s8");
}

DEFINE_EM_STUB(EmGXTexCoord1u8, (u8 s))
{
    GX_UNUSED(s);
    GXStubOnce("EmGXTexCoord1u8");
}

DEFINE_EM_STUB(EmGXTexCoord1x16, (u16 index))
{
    GX_UNUSED(index);
    GXStubOnce("EmGXTexCoord1x16");
}

DEFINE_EM_STUB(EmGXTexCoord1x8, (u8 index))
{
    GX_UNUSED(index);
    GXStubOnce("EmGXTexCoord1x8");
}

DEFINE_EM_STUB(EmGXMatrixIndex1u8, (u8 index))
{
    GX_UNUSED(index);
    GXStubOnce("EmGXMatrixIndex1u8");
}

/*---------------------------------------------------------------------------*
    Command Functions
 *---------------------------------------------------------------------------*/

void GXCmd1u8(u8 cmd)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXCmd1u8(cmd);
    } else {
        *((u8*)__EmBuffPtr) = cmd;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXCmd1u16(u16 cmd)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXCmd1u16(cmd);
    } else {
        *((u16*)__EmBuffPtr) = cmd;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXCmd1u32(u32 cmd)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXCmd1u32(cmd);
    } else {
        *((u32*)__EmBuffPtr) = cmd;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

/*---------------------------------------------------------------------------*
    Parameter Functions
 *---------------------------------------------------------------------------*/

void GXParam1u8(u8 param)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXParam1u8(param);
    } else {
        *((u8*)__EmBuffPtr) = param;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXParam1u16(u16 param)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXParam1u16(param);
    } else {
        *((u16*)__EmBuffPtr) = param;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXParam1u32(u32 param)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXParam1u32(param);
    } else {
        *((u32*)__EmBuffPtr) = param;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXParam1s8(s8 param)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXParam1s8(param);
    } else {
        *((s8*)__EmBuffPtr) = param;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXParam1s16(s16 param)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXParam1s16(param);
    } else {
        *((s16*)__EmBuffPtr) = param;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXParam1s32(s32 param)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXParam1s32(param);
    } else {
        *((s32*)__EmBuffPtr) = param;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXParam1f32(f32 param)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXParam1f32(param);
    } else {
        *((f32*)__EmBuffPtr) = param;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXParam3f32(f32 x, f32 y, f32 z)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXParam3f32(x, y, z);
    } else {
        *((f32*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXParam4f32(f32 x, f32 y, f32 z, f32 w)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXParam4f32(x, y, z, w);
    } else {
        *((f32*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = w;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

/*---------------------------------------------------------------------------*
    Position Functions
 *---------------------------------------------------------------------------*/

void GXPosition3f32(f32 x, f32 y, f32 z)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition3f32(x, y, z);
    } else {
        *((f32*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXPosition3u8(u8 x, u8 y, u8 z)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition3u8(x, y, z);
    } else {
        *((u8*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((u8*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((u8*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXPosition3s8(s8 x, s8 y, s8 z)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition3s8(x, y, z);
    } else {
        *((s8*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((s8*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((s8*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXPosition3u16(u16 x, u16 y, u16 z)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition3u16(x, y, z);
    } else {
        *((u16*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((u16*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((u16*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXPosition3s16(s16 x, s16 y, s16 z)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition3s16(x, y, z);
    } else {
        *((s16*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((s16*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((s16*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXPosition2f32(f32 x, f32 y)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition2f32(x, y);
    } else {
        *((f32*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXPosition2u8(u8 x, u8 y)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition2u8(x, y);
    } else {
        *((u8*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((u8*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXPosition2s8(s8 x, s8 y)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition2s8(x, y);
    } else {
        *((s8*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((s8*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXPosition2u16(u16 x, u16 y)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition2u16(x, y);
    } else {
        *((u16*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((u16*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXPosition2s16(s16 x, s16 y)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition2s16(x, y);
    } else {
        *((s16*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((s16*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXPosition1x16(u16 index)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition1x16(index);
    } else {
        *((u16*)__EmBuffPtr) = index;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXPosition1x8(u8 index)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXPosition1x8(index);
    } else {
        *((u8*)__EmBuffPtr) = index;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

/*---------------------------------------------------------------------------*
    Normal Functions
 *---------------------------------------------------------------------------*/

void GXNormal3f32(f32 x, f32 y, f32 z)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXNormal3f32(x, y, z);
    } else {
        *((f32*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXNormal3s16(s16 x, s16 y, s16 z)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXNormal3s16(x, y, z);
    } else {
        *((s16*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((s16*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((s16*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXNormal3s8(s8 x, s8 y, s8 z)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXNormal3s8(x, y, z);
    } else {
        *((s8*)__EmBuffPtr) = x;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((s8*)__EmBuffPtr) = y;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((s8*)__EmBuffPtr) = z;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXNormal1x16(u16 index)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXNormal1x16(index);
    } else {
        *((u16*)__EmBuffPtr) = index;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXNormal1x8(u8 index)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXNormal1x8(index);
    } else {
        *((u8*)__EmBuffPtr) = index;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

/*---------------------------------------------------------------------------*
    Color Functions
 *---------------------------------------------------------------------------*/

void GXColor4u8(u8 r, u8 g, u8 b, u8 a)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXColor4u8(r, g, b, a);
    } else {
        *((u8*)__EmBuffPtr) = r;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((u8*)__EmBuffPtr) = g;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((u8*)__EmBuffPtr) = b;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((u8*)__EmBuffPtr) = a;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXColor1u32(u32 color)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXColor1u32(color);
    } else {
        *((u32*)__EmBuffPtr) = color;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXColor3u8(u8 r, u8 g, u8 b)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXColor3u8(r, g, b);
    } else {
        *((u8*)__EmBuffPtr) = r;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((u8*)__EmBuffPtr) = g;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((u8*)__EmBuffPtr) = b;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXColor1u16(u16 color)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXColor1u16(color);
    } else {
        *((u16*)__EmBuffPtr) = color;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXColor1x16(u16 index)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXColor1x16(index);
    } else {
        *((u16*)__EmBuffPtr) = index;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXColor1x8(u8 index)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXColor1x8(index);
    } else {
        *((u8*)__EmBuffPtr) = index;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

/*---------------------------------------------------------------------------*
    Texture Coordinate Functions
 *---------------------------------------------------------------------------*/

void GXTexCoord2f32(f32 s, f32 t)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord2f32(s, t);
    } else {
        *((f32*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 4;
        *((f32*)__EmBuffPtr) = t;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXTexCoord2s16(s16 s, s16 t)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord2s16(s, t);
    } else {
        *((s16*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((s16*)__EmBuffPtr) = t;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXTexCoord2u16(u16 s, u16 t)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord2u16(s, t);
    } else {
        *((u16*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 2;
        *((u16*)__EmBuffPtr) = t;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXTexCoord2s8(s8 s, s8 t)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord2s8(s, t);
    } else {
        *((s8*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((s8*)__EmBuffPtr) = t;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXTexCoord2u8(u8 s, u8 t)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord2u8(s, t);
    } else {
        *((u8*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 1;
        *((u8*)__EmBuffPtr) = t;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXTexCoord1f32(f32 s)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord1f32(s);
    } else {
        *((f32*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 4;
    }
}

void GXTexCoord1s16(s16 s)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord1s16(s);
    } else {
        *((s16*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXTexCoord1u16(u16 s)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord1u16(s);
    } else {
        *((u16*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXTexCoord1s8(s8 s)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord1s8(s);
    } else {
        *((s8*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXTexCoord1u8(u8 s)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord1u8(s);
    } else {
        *((u8*)__EmBuffPtr) = s;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

void GXTexCoord1x16(u16 index)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord1x16(index);
    } else {
        *((u16*)__EmBuffPtr) = index;
        __EmBuffPtr = __EmBuffPtr + 2;
    }
}

void GXTexCoord1x8(u8 index)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXTexCoord1x8(index);
    } else {
        *((u8*)__EmBuffPtr) = index;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

/*---------------------------------------------------------------------------*
    Matrix Index Functions
 *---------------------------------------------------------------------------*/

void GXMatrixIndex1u8(u8 index)
{
    if (__EmDisplayListInProgress == FALSE) {
        EmGXMatrixIndex1u8(index);
    } else {
        *((u8*)__EmBuffPtr) = index;
        __EmBuffPtr = __EmBuffPtr + 1;
    }
}

