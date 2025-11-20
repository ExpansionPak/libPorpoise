/*---------------------------------------------------------------------------*
  GXDispList.c - GX Display List Functions
  
  Based on reverse-engineered Dolphin emulator GX implementation.
  Recreates the original GameCube/Wii GX display list functions.
  
  Source: c:\rabin\dolphin\build\libraries\gx\src\gxdisplist.c
 *---------------------------------------------------------------------------*/

#include <dolphin/gx/GXDispList.h>
#include <dolphin/gx/GXEnum.h>
#include <dolphin/os.h>
#include <string.h>

/*---------------------------------------------------------------------------*
    External Variables
 *---------------------------------------------------------------------------*/

/* Display list buffer pointers (defined in GXVert.c) */
extern u8* __EmBuffPtr;
extern u8* __EmBuffTop;
extern BOOL __EmDisplayListInProgress;

/* Display list start pointer */
static u8* __EmBuffStart = NULL;

/*---------------------------------------------------------------------------*
    External Function Declarations
 *---------------------------------------------------------------------------*/

/* GX primitive functions */
extern void GXBegin(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts);
extern void GXEnd(void);

/* GX vertex functions */
extern void GXPosition1x8(u8 index);
extern void GXPosition1x16(u16 index);
extern void GXPosition2u8(u8 x, u8 y);
extern void GXPosition2s8(s8 x, s8 y);
extern void GXPosition2u16(u16 x, u16 y);
extern void GXPosition2s16(s16 x, s16 y);
extern void GXPosition2f32(f32 x, f32 y);
extern void GXPosition3u8(u8 x, u8 y, u8 z);
extern void GXPosition3s8(s8 x, s8 y, s8 z);
extern void GXPosition3u16(u16 x, u16 y, u16 z);
extern void GXPosition3s16(s16 x, s16 y, s16 z);
extern void GXPosition3f32(f32 x, f32 y, f32 z);

extern void GXNormal3s8(s8 x, s8 y, s8 z);
extern void GXNormal3s16(s16 x, s16 y, s16 z);
extern void GXNormal3f32(f32 x, f32 y, f32 z);

extern void GXColor1u16(u16 color);
extern void GXColor3u8(u8 r, u8 g, u8 b);
extern void GXColor4u8(u8 r, u8 g, u8 b, u8 a);

extern void GXTexCoord1u8(u8 s);
extern void GXTexCoord1s8(s8 s);
extern void GXTexCoord1u16(u16 s);
extern void GXTexCoord1s16(s16 s);
extern void GXTexCoord1f32(f32 s);
extern void GXTexCoord2u8(u8 s, u8 t);
extern void GXTexCoord2s8(s8 s, s8 t);
extern void GXTexCoord2u16(u16 s, u16 t);
extern void GXTexCoord2s16(s16 s, s16 t);
extern void GXTexCoord2f32(f32 s, f32 t);

extern void GXMatrixIndex1u8(u8 index);

/*---------------------------------------------------------------------------*
    Internal Helper Functions
 *---------------------------------------------------------------------------*/

/* Get a 32-bit float from buffer */
static f32 GetF32(u8** buf, u32* remaining);

/* Get a 16-bit unsigned integer from buffer */
static u16 GetU16(u8** buf, u32* remaining);

/* Get an 8-bit unsigned integer from buffer */
static u8 GetU8(u8** buf, u32* remaining);

/* Get vertex length based on VCD */
static u8 GetVertexLength(void);

/* Get VCD component length */
static u8 VCDComponentLength(u32 vcd_value, u8 component);

/* Send primitive data */
static void SendPrimitiveData(u8** buf, u32* remaining);

/* Send position data directly */
static void SendPositionDirect(u8 index, u8** buf, u32* remaining);

/* Send normal data directly */
static void SendNormalDirect(u8 index, u8** buf, u32* remaining);

/* Send color data directly */
static void SendColorDirect(u8 index, u8** buf, u32* remaining);

/* Send texture coordinate data directly */
static void SendTexCoordDirect(u8 index, u8** buf, u32* remaining);

/*---------------------------------------------------------------------------*
    External Data Structures
 *---------------------------------------------------------------------------*/

/* GfxTrack structure - vertex attribute tracking (12 bytes per entry) */
extern struct {
    u8 attr_type;      /* Attribute type (0-5: MatrixIndex, Position, Normal, Color, TexCoord), bit 7 = continue */
    u8 comp_type;       /* Component type (0-4: u8, s8, u16, s16, f32) */
    u8 comp_count;      /* Component count (0=1, 1=2, 2=3, 3=4) */
    u8 direct;          /* Direct mode flag */
    u8 pad[8];          /* Padding to 12 bytes */
} __GfxTrack[26];

/* VCD array - vertex component descriptor */
extern u32 gVCD[26];

/*---------------------------------------------------------------------------*
    Implementation
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         GXBeginDisplayList
  
  Description:  Begin recording a display list into the specified buffer.
                The buffer size must be a multiple of 32 bytes.
  
  Arguments:    list - Pointer to display list buffer
                size - Size of buffer in bytes (must be multiple of 32)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXBeginDisplayList(void* list, u32 size) {
    if (__EmDisplayListInProgress != FALSE) {
        OSPanic("gxdisplist.c", 
                __LINE__ + 2, "GXBeginDisplayList: display list already in progress");
    }
    
    if ((size & 0x1f) != 0) {
        OSPanic("gxdisplist.c", 
                __LINE__ + 4, "GXBeginDisplayList: size is not a multiple of 32");
    }
    
    __EmBuffPtr = (u8*)list;
    __EmBuffStart = (u8*)list;
    __EmBuffTop = (u8*)list + size;
    __EmDisplayListInProgress = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         GXEndDisplayList
  
  Description:  End recording a display list. Pads the buffer to 32-byte
                alignment and returns the size used.
  
  Arguments:    None
  
  Returns:      Size of display list in bytes
 *---------------------------------------------------------------------------*/
u32 GXEndDisplayList(void) {
    u32 pad_bytes;
    u32 size;
    
    if (__EmDisplayListInProgress != TRUE) {
        OSPanic("gxdisplist.c", 
                __LINE__ + 3, "GXEndDisplayList: no display list in progress");
    }
    
    if (__EmBuffTop < __EmBuffPtr) {
        OSPanic("gxdisplist.c", 
                __LINE__ + 4, "GXEndDisplayList: display list overflow");
    }
    
    /* Calculate size before padding */
    size = (u32)(__EmBuffPtr - __EmBuffStart);
    
    /* Pad to 32-byte alignment */
    pad_bytes = (0x20 - ((u32)__EmBuffPtr & 0x1f)) & 0x1f;
    while (pad_bytes != 0) {
        *__EmBuffPtr = 0;
        __EmBuffPtr = __EmBuffPtr + 1;
        pad_bytes--;
    }
    
    __EmDisplayListInProgress = FALSE;
    __EmBuffPtr = NULL;
    __EmBuffTop = NULL;
    __EmBuffStart = NULL;
    
    return size;
}

/*---------------------------------------------------------------------------*
  Name:         GXCallDisplayList
  
  Description:  Execute a display list from memory.
  
  Arguments:    list - Pointer to display list data
                size - Size of display list in bytes (must be multiple of 32)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXCallDisplayList(void* list, u32 size) {
    u8* buf;
    u32 remaining;
    u8 opcode;
    u8 vtxfmt;
    u16 nverts;
    u8 vertex_len;
    u32 data_size;
    
    buf = (u8*)list;
    remaining = size;
    
    if (__EmDisplayListInProgress != FALSE) {
        OSPanic("gxdisplist.c", 
                __LINE__ + 10, "GXCallDisplayList: display list already in progress");
    }
    
    if ((remaining & 0x1f) != 0) {
        OSPanic("gxdisplist.c", 
                __LINE__ + 12, "GXCallDisplayList: nbytes is not a multiple of 32");
    }
    
    while (remaining > 0) {
        if (remaining < 1) {
            if (remaining != 0) {
                OSPanic("gxdisplist.c", 
                        __LINE__ + 0x4f, "GXCallDisplayList: nbytes is unmatch");
            }
            return;
        }
        
        opcode = GetU8(&buf, &remaining);
        vtxfmt = opcode & 0x07;
        opcode = opcode & 0xf8;
        
        switch (opcode) {
            case 0:
                /* NOP */
                break;
                
            case 0x80:  /* GX_DRAW_QUADS */
                nverts = GetU16(&buf, &remaining);
                GXBegin(0x80, vtxfmt, nverts);
                break;
                
            case 0x90:  /* GX_DRAW_TRIANGLES */
                nverts = GetU16(&buf, &remaining);
                GXBegin(0x90, vtxfmt, nverts);
                break;
                
            case 0x98:  /* GX_DRAW_TRIANGLE_STRIP */
                nverts = GetU16(&buf, &remaining);
                GXBegin(0x98, vtxfmt, nverts);
                break;
                
            case 0xa0:  /* GX_DRAW_TRIANGLE_FAN */
                nverts = GetU16(&buf, &remaining);
                GXBegin(0xa0, vtxfmt, nverts);
                break;
                
            case 0xa8:  /* GX_DRAW_LINES */
                nverts = GetU16(&buf, &remaining);
                GXBegin(0xa8, vtxfmt, nverts);
                break;
                
            case 0xb0:  /* GX_DRAW_LINE_STRIP */
                nverts = GetU16(&buf, &remaining);
                GXBegin(0xb0, vtxfmt, nverts);
                break;
                
            case 0xb8:  /* GX_DRAW_POINTS */
                nverts = GetU16(&buf, &remaining);
                GXBegin(0xb8, vtxfmt, nverts);
                break;
                
            default:
                OSPanic("gxdisplist.c", 
                        __LINE__ + 0x41, "GXCallDisplayList: Unknown opcode");
                break;
        }
        
        vertex_len = GetVertexLength();
        data_size = vertex_len * (nverts & 0xffff);
        remaining = remaining - data_size;
        
        if ((s32)remaining < 0) {
            OSPanic("gxdisplist.c", 
                    __LINE__ + 0x4f, "GXCallDisplayList: nbytes is unmatch");
        }
        
        SendPrimitiveData(&buf, &remaining);
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
  Name:         SendPrimitiveData
  
  Description:  Send vertex data for a primitive based on VCD configuration.
  
  Arguments:    buf - Pointer to buffer pointer (updated)
                remaining - Pointer to remaining bytes (updated)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
static void SendPrimitiveData(u8** buf, u32* remaining) {
    u8 index;
    u8 attr_type;
    u8 comp_type;
    u8 comp_count;
    u8 direct;
    
    while (*remaining > 0) {
        index = 0;
        do {
            attr_type = (__GfxTrack[index].attr_type & 0x7f) - 1;
            
            switch (attr_type) {
                case 0:  /* MatrixIndex */
                case 1:  /* MatrixIndex */
                    GetU8(buf, remaining);
                    GXMatrixIndex1u8(GetU8(buf, remaining));
                    break;
                    
                case 2:  /* Position */
                    comp_type = __GfxTrack[index].comp_type;
                    comp_count = __GfxTrack[index].comp_count;
                    direct = __GfxTrack[index].direct;
                    
                    if (direct == 1) {
                        SendPositionDirect(index, buf, remaining);
                    } else if (comp_type == 2) {
                        GXPosition1x8(GetU8(buf, remaining));
                    } else if (comp_type == 3) {
                        GXPosition1x16(GetU16(buf, remaining));
                    }
                    break;
                    
                case 3:  /* Normal */
                    comp_type = __GfxTrack[index].comp_type;
                    direct = __GfxTrack[index].direct;
                    
                    if (direct == 1) {
                        SendNormalDirect(index, buf, remaining);
                    } else if (comp_type == 2) {
                        /* s8 normal: read 3 components */
                        s8 nx = (s8)GetU8(buf, remaining);
                        s8 ny = (s8)GetU8(buf, remaining);
                        s8 nz = (s8)GetU8(buf, remaining);
                        GXNormal3s8(nx, ny, nz);
                    } else if (comp_type == 3) {
                        /* s16 normal: read 3 components */
                        s16 nx = (s16)GetU16(buf, remaining);
                        s16 ny = (s16)GetU16(buf, remaining);
                        s16 nz = (s16)GetU16(buf, remaining);
                        GXNormal3s16(nx, ny, nz);
                    }
                    break;
                    
                case 4:  /* Color */
                    comp_type = __GfxTrack[index].comp_type;
                    direct = __GfxTrack[index].direct;
                    
                    if (direct == 1) {
                        SendColorDirect(index, buf, remaining);
                    } else if (comp_type == 2) {
                        GXColor1u16(GetU16(buf, remaining));
                    } else if (comp_type == 3) {
                        GXColor3u8(GetU8(buf, remaining), GetU8(buf, remaining), GetU8(buf, remaining));
                    } else if (comp_type == 4) {
                        GXColor4u8(GetU8(buf, remaining), GetU8(buf, remaining), GetU8(buf, remaining), GetU8(buf, remaining));
                    }
                    break;
                    
                case 5:  /* TexCoord */
                    comp_type = __GfxTrack[index].comp_type;
                    direct = __GfxTrack[index].direct;
                    
                    if (direct == 1) {
                        SendTexCoordDirect(index, buf, remaining);
                    } else if (comp_type == 2) {
                        GXTexCoord1x8(GetU8(buf, remaining));
                    } else if (comp_type == 3) {
                        GXTexCoord1x16(GetU16(buf, remaining));
                    }
                    break;
                    
                default:
                    OSPanic("gxdisplist.c", 
                            __LINE__ + 0x73, "GXCallDisplayList: decode error");
                    break;
            }
            
            if ((__GfxTrack[index].attr_type & 0x80) == 0) {
                index++;
            } else {
                break;
            }
        } while (index < 26);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SendPositionDirect
  
  Description:  Send position data in direct mode.
  
  Arguments:    index - Attribute index
                buf - Pointer to buffer pointer (updated)
                remaining - Pointer to remaining bytes (updated)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
static void SendPositionDirect(u8 index, u8** buf, u32* remaining) {
    u8 comp_count;
    u8 comp_type;
    f32 x, y, z;
    u16 u16_val;
    u8 u8_val;
    s16 s16_val;
    s8 s8_val;
    
    comp_count = __GfxTrack[index].comp_count;
    comp_type = __GfxTrack[index].comp_type;
    
    if (comp_count == 0) {
        /* 2 components */
        switch (comp_type) {
            case 0:  /* u8 */
                GXPosition2u8(GetU8(buf, remaining), GetU8(buf, remaining));
                break;
            case 1:  /* s8 */
                GXPosition2s8((s8)GetU8(buf, remaining), (s8)GetU8(buf, remaining));
                break;
            case 2:  /* u16 */
                GXPosition2u16(GetU16(buf, remaining), GetU16(buf, remaining));
                break;
            case 3:  /* s16 */
                GXPosition2s16((s16)GetU16(buf, remaining), (s16)GetU16(buf, remaining));
                break;
            case 4:  /* f32 */
                GXPosition2f32(GetF32(buf, remaining), GetF32(buf, remaining));
                break;
            default:
                OSPanic("gxdisplist.c", 
                        __LINE__ + 0x4f, "GXCallDisplayList: decode error");
                break;
        }
    } else if (comp_count == 1) {
        /* 3 components */
        switch (comp_type) {
            case 0:  /* u8 */
                GXPosition3u8(GetU8(buf, remaining), GetU8(buf, remaining), GetU8(buf, remaining));
                break;
            case 1:  /* s8 */
                GXPosition3s8((s8)GetU8(buf, remaining), (s8)GetU8(buf, remaining), (s8)GetU8(buf, remaining));
                break;
            case 2:  /* u16 */
                GXPosition3u16(GetU16(buf, remaining), GetU16(buf, remaining), GetU16(buf, remaining));
                break;
            case 3:  /* s16 */
                GXPosition3s16((s16)GetU16(buf, remaining), (s16)GetU16(buf, remaining), (s16)GetU16(buf, remaining));
                break;
            case 4:  /* f32 */
                GXPosition3f32(GetF32(buf, remaining), GetF32(buf, remaining), GetF32(buf, remaining));
                break;
            default:
                OSPanic("gxdisplist.c", 
                        __LINE__ + 0x4f, "GXCallDisplayList: decode error");
                break;
        }
    } else {
        OSPanic("gxdisplist.c", 
                __LINE__ + 0x4f, "GXCallDisplayList: decode error");
    }
}

/*---------------------------------------------------------------------------*
  Name:         SendNormalDirect
  
  Description:  Send normal data in direct mode.
  
  Arguments:    index - Attribute index
                buf - Pointer to buffer pointer (updated)
                remaining - Pointer to remaining bytes (updated)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
static void SendNormalDirect(u8 index, u8** buf, u32* remaining) {
    u8 comp_type;
    
    comp_type = __GfxTrack[index].comp_type;
    
    if (comp_type == 1) {  /* s8 */
        GXNormal3s8((s8)GetU8(buf, remaining), (s8)GetU8(buf, remaining), (s8)GetU8(buf, remaining));
    } else if (comp_type == 3) {  /* s16 */
        GXNormal3s16((s16)GetU16(buf, remaining), (s16)GetU16(buf, remaining), (s16)GetU16(buf, remaining));
    } else if (comp_type == 4) {  /* f32 */
        GXNormal3f32(GetF32(buf, remaining), GetF32(buf, remaining), GetF32(buf, remaining));
    } else {
        OSPanic("gxdisplist.c", 
                __LINE__ + 0x16, "GXCallDisplayList: decode error");
    }
}

/*---------------------------------------------------------------------------*
  Name:         SendColorDirect
  
  Description:  Send color data in direct mode.
  
  Arguments:    index - Attribute index
                buf - Pointer to buffer pointer (updated)
                remaining - Pointer to remaining bytes (updated)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
static void SendColorDirect(u8 index, u8** buf, u32* remaining) {
    u8 comp_type;
    
    comp_type = __GfxTrack[index].comp_type;
    
    switch (comp_type) {
        case 0:  /* u16 */
        case 3:  /* u16 */
            GXColor1u16(GetU16(buf, remaining));
            break;
        case 1:  /* u8 */
        case 4:  /* u8 */
            GXColor3u8(GetU8(buf, remaining), GetU8(buf, remaining), GetU8(buf, remaining));
            break;
        case 2:  /* u8 */
        case 5:  /* u8 */
            GXColor4u8(GetU8(buf, remaining), GetU8(buf, remaining), GetU8(buf, remaining), GetU8(buf, remaining));
            break;
        default:
            OSPanic("gxdisplist.c", 
                    __LINE__ + 0x18, "GXCallDisplayList: decode error");
            break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SendTexCoordDirect
  
  Description:  Send texture coordinate data in direct mode.
  
  Arguments:    index - Attribute index
                buf - Pointer to buffer pointer (updated)
                remaining - Pointer to remaining bytes (updated)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
static void SendTexCoordDirect(u8 index, u8** buf, u32* remaining) {
    u8 comp_count;
    u8 comp_type;
    
    comp_count = __GfxTrack[index].comp_count;
    comp_type = __GfxTrack[index].comp_type;
    
    if (comp_count == 0) {
        /* 1 component */
        switch (comp_type) {
            case 0:  /* u8 */
                GXTexCoord1u8(GetU8(buf, remaining));
                break;
            case 1:  /* s8 */
                GXTexCoord1s8((s8)GetU8(buf, remaining));
                break;
            case 2:  /* u16 */
                GXTexCoord1u16(GetU16(buf, remaining));
                break;
            case 3:  /* s16 */
                GXTexCoord1s16((s16)GetU16(buf, remaining));
                break;
            case 4:  /* f32 */
                GXTexCoord1f32(GetF32(buf, remaining));
                break;
            default:
                OSPanic("gxdisplist.c", 
                        __LINE__ + 0x45, "GXCallDisplayList: decode error");
                break;
        }
    } else if (comp_count == 1) {
        /* 2 components */
        switch (comp_type) {
            case 0:  /* u8 */
                GXTexCoord2u8(GetU8(buf, remaining), GetU8(buf, remaining));
                break;
            case 1:  /* s8 */
                GXTexCoord2s8((s8)GetU8(buf, remaining), (s8)GetU8(buf, remaining));
                break;
            case 2:  /* u16 */
                GXTexCoord2u16(GetU16(buf, remaining), GetU16(buf, remaining));
                break;
            case 3:  /* s16 */
                GXTexCoord2s16((s16)GetU16(buf, remaining), (s16)GetU16(buf, remaining));
                break;
            case 4:  /* f32 */
                GXTexCoord2f32(GetF32(buf, remaining), GetF32(buf, remaining));
                break;
            default:
                OSPanic("gxdisplist.c", 
                        __LINE__ + 0x45, "GXCallDisplayList: decode error");
                break;
        }
    } else {
        OSPanic("gxdisplist.c", 
                __LINE__ + 0x45, "GXCallDisplayList: decode error");
    }
}

/*---------------------------------------------------------------------------*
  Name:         GetF32
  
  Description:  Read a 32-bit float from buffer (little-endian).
  
  Arguments:    buf - Pointer to buffer pointer (updated)
                remaining - Pointer to remaining bytes (updated)
  
  Returns:      f32 value
 *---------------------------------------------------------------------------*/
static f32 GetF32(u8** buf, u32* remaining) {
    f32 value;
    u8* p = *buf;
    
    /* Read 4 bytes as float (little-endian) */
    value = *(f32*)p;
    *buf = p + 4;
    *remaining = *remaining - 4;
    
    return value;
}

/*---------------------------------------------------------------------------*
  Name:         GetU16
  
  Description:  Read a 16-bit unsigned integer from buffer (little-endian).
  
  Arguments:    buf - Pointer to buffer pointer (updated)
                remaining - Pointer to remaining bytes (updated)
  
  Returns:      u16 value
 *---------------------------------------------------------------------------*/
static u16 GetU16(u8** buf, u32* remaining) {
    u16 value;
    u8* p = *buf;
    
    /* Read 2 bytes as u16 (little-endian) */
    value = (u16)p[0] | ((u16)p[1] << 8);
    *buf = p + 2;
    *remaining = *remaining - 2;
    
    return value;
}

/*---------------------------------------------------------------------------*
  Name:         GetU8
  
  Description:  Read an 8-bit unsigned integer from buffer.
  
  Arguments:    buf - Pointer to buffer pointer (updated)
                remaining - Pointer to remaining bytes (updated)
  
  Returns:      u8 value
 *---------------------------------------------------------------------------*/
static u8 GetU8(u8** buf, u32* remaining) {
    u8 value;
    u8* p = *buf;
    
    value = *p;
    *buf = p + 1;
    *remaining = *remaining - 1;
    
    return value;
}

/*---------------------------------------------------------------------------*
  Name:         GetVertexLength
  
  Description:  Calculate vertex length based on VCD configuration.
  
  Arguments:    None
  
  Returns:      u8 vertex length in bytes
 *---------------------------------------------------------------------------*/
static u8 GetVertexLength(void) {
    u8 length = 0;
    u8 index;
    
    for (index = 0; index < 26; index++) {
        length += VCDComponentLength(gVCD[index], index);
    }
    
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         VCDComponentLength
  
  Description:  Get the length of a VCD component.
  
  Arguments:    vcd_value - VCD value for the component
                component - Component index (0-25)
  
  Returns:      u8 component length in bytes
 *---------------------------------------------------------------------------*/
static u8 VCDComponentLength(u32 vcd_value, u8 component) {
    switch (vcd_value) {
        case 0:  /* GX_NONE */
            return 0;
        case 1:  /* GX_DIRECT */
            if (component < 8) {
                OSPanic("gxdisplist.c", 
                        __LINE__ + 8, "GXCallDisplayList: GX_DIRECT not allowed for index < 8");
            }
            return 0;  /* Direct mode - length calculated elsewhere */
        case 2:  /* GX_INDEX8 */
            return 1;
        case 3:  /* GX_INDEX16 */
            return 2;
        default:
            OSPanic("gxdisplist.c", 
                    __LINE__ + 10, "GXCallDisplayList: Illigal VCD setting");
            return 0;
    }
}

