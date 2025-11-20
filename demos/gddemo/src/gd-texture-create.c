/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gd-texture-create.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-texture-create.c $
    
    4     11/08/01 6:38p Carl
    Added code to change TMEM configuration.
    
    3     10/13/01 2:29a Hirose
    Fixes due to GDSetTexCoordGen API change.
    
    2     10/11/01 4:05p Carl
    Added use of color-index texture and TLUT.
    
    1     9/19/01 4:27p Carl
    Sources for GD texture demo.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin/gd.h>
#include <dolphin/mtx/GeoTypes.h>

#ifdef WIN32
#include <assert.h>
#include <stdlib.h>
#else
#include <dolphin/os.h>
#endif

/*---------------------------------------------------------------------------*/

#ifdef WIN32
#define ASSERT           assert
#define OSRoundUp32B(x)  (((u32)(x) + 31) & ~31)
#define OSAlloc(x)       ((void*)OSRoundUp32B(malloc((x)+31)))
#define OSFree(x)        free(x)
#endif

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void CreateDLs( void );

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

// Display lists *************************************************************

// This DL is used with the "Draw" display list.
// It initializes state that will be used by the Draw DL.

extern GDLObj InitDLO;
#define INIT_STATE_SIZE 2048  // maximum (not actual) size

// This DL draws a textured cube.  It must be paired with the Init DL.

extern GDLObj DrawDLO;
#define DRAW_SIZE 2048        // maximum (not actual) size

// This array indicates the offsets to patch memory addresses for the
// primitive data arrays (positions, normals, texture coordinates)
// referred to in the Init DL.

extern u32 *setArrayOffsets;

// This array tells us the offsets for where to patch the memory addresses
// for the textures in the Draw DL.

extern u32 *texAddrOffsets;

// This array tells us the offsets for where to patch the main memory
// addresses for loading the TLUTs in the Init DL.

extern u32 *tlutAddrOffsets;

// This TLUT address (in TMEM) corresponds to the first 256-entry
// TLUT in the default GX allocation (tlut 0).  Consider using the
// array GXTlutRegions (from GDTexture.c) if you extensively use the
// default GX TLUT allocation scheme.

#define TLUT0_ADDR 0xC0000

/*---------------------------------------------------------------------------*/

// This array is used to map texture cache regions in TMEM.  It
// follows GX's default TMEM configuration.  In the default configuration,
// texmap 0 uses the first pair of addresses in the list, texmap 1 uses the
// second, and so on.  In this demo, we use only texmap 0 and change its
// mapping based on the texture we draw, making sure that each different
// texture uses a unique cache region.  Of course, we could just as easily
// have used a unique texmap per texture, but for this demo we wanted to
// emphasize how you must pay attention to texture cache use.

u32 MyTmemRegions[8][2] = {
   // TMEM LO, TMEM HI
    { 0x00000, 0x80000 },  // (default assignment for texmap 0)
    { 0x08000, 0x88000 },  //                                1
    { 0x10000, 0x90000 },  //                                2
    { 0x18000, 0x98000 },  //                                3
    { 0x20000, 0xa0000 },  //                                4
    { 0x28000, 0xa8000 },  //                                5
    { 0x30000, 0xb0000 },  //                                6
    { 0x38000, 0xb8000 }   //                                7
};

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Name:           CreateDLs
    
    Description:    Creates the display lists used by the program.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void CreateDLs ( void )
{
    u8 *InitStateList;
    u8 *DrawList;

    //
    // Create the init-state DL
    //

    // This display list initializes some important drawing state that will
    // be used by the Draw DL.  We will initialize the VCD/VAT, the array
    // base pointers, the texture lookup state, and the texcoord scaling.

    InitStateList = OSAlloc( INIT_STATE_SIZE );
    ASSERT(InitStateList);

    setArrayOffsets = OSAlloc( 3 * sizeof(u32) );
    ASSERT(setArrayOffsets);

    tlutAddrOffsets = OSAlloc( 1 * sizeof(u32) );
    ASSERT(tlutAddrOffsets);

    GDInitGDLObj( &InitDLO, InitStateList, INIT_STATE_SIZE );
    GDSetCurrent( &InitDLO );
    
    // Set the VCD and VAT
    {
        static GXVtxDescList vcd[] = {
            { GX_VA_POS, GX_INDEX8 },
            { GX_VA_NRM, GX_INDEX8 },
            { GX_VA_TEX0, GX_INDEX8 },
            { GX_VA_NULL, GX_NONE }, // NULL terminator is required
        };
        static GXVtxAttrFmtList vat[] = {
            { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
            { GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0 },
            { GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 },
            { GX_VA_NULL, GX_POS_XY, GX_U8, 0 }, // NULL terminator is required
        };
        
        GDSetVtxDescv( vcd );
        GDSetVtxAttrFmtv( GX_VTXFMT0, vat );
    }

    // Set the vertex array pointers and strides.  The actual array
    // pointers will be patched in later, so we need to keep track of
    // these patch points.
    // We add a bit to skip over the command token and register ID bytes.

    setArrayOffsets[0] = GDGetCurrOffset() + CP_DATA_OFFSET;
    GDSetArrayRaw(GX_VA_POS, 0, 12);

    setArrayOffsets[1] = GDGetCurrOffset() + CP_DATA_OFFSET;
    GDSetArrayRaw(GX_VA_NRM, 0, 12);

    setArrayOffsets[2] = GDGetCurrOffset() + CP_DATA_OFFSET;
    GDSetArrayRaw(GX_VA_TEX0, 0, 8);
    
    // Commands to initialize (most of) texture ID 0 and texcoord 0 scale:
    // This DL is used with the Draw display list, which assumes that
    // texture ID 0 and texcoord 0 have been set up in this way.
    
    // Since all the textures we're using are the same size and have mostly
    // the same attributes, we can get away with this.  Otherwise, these
    // registers would have to be loaded for each face in the DrawList.
    
    // Initialize TEXMAP0 lookup attributes.
    GDSetTexLookupMode( GX_TEXMAP0,             // tmap
                        GX_REPEAT,              // wrap s
                        GX_REPEAT,              // wrap t
                        GX_LIN_MIP_LIN,         // min filt
                        GX_LINEAR,              // mag filt
                        0.0f,                   // min LOD
                        7.0f,                   // max LOD
                        0.0f,                   // LOD bias
                        GX_FALSE,               // bias clamp?
                        GX_TRUE,                // do edge LOD?
                        GX_ANISO_1 );           // max aniso

    // And texcoord0 scale parameters.
    GDSetTexCoordScale2( GX_TEXCOORD0,               // tcoord
                         128, GX_FALSE, GX_FALSE,    // s size, bias, wrap
                         128, GX_FALSE, GX_FALSE );  // t size, bias, wrap

    // Set the texture TLUT parameters.  This will only be used by the
    // texture format that is color-indexed.
    GDSetTexTlut ( GX_TEXMAP0, TLUT0_ADDR, GX_TL_RGB565 );

    // Need to save this offset for the LoadTlut command.
    tlutAddrOffsets[0] = GDGetCurrOffset() + BP_CMD_LENGTH*2 + BP_DATA_OFFSET;

    // Load the TLUT.  Again, this is only needed for the CI texture.
    // We don't know the correct memory address now; it will be patched later.
    GDLoadTlutRaw ( 0, TLUT0_ADDR, GX_TLUT_256 );

    // That's all the commands for that DL; now pad to 32-byte boundary.
    GDPadCurr32();

    GDFlushCurrToMem(); // not strictly needed for PC-side, but not a bad idea

    //
    // Create the cube drawing DL
    //

    // Draw DL: commands to select textures and draw the actual geometry.
    // This display list must be paired with the Init DL.  It assumes a
    // particular texture setup which is setup by that DL.  As a result,
    // it can switch textures by changing only the minimum number of
    // registers.

    DrawList = OSAlloc( DRAW_SIZE );
    ASSERT(DrawList);

    texAddrOffsets = OSAlloc( 6 * sizeof(u32) );
    ASSERT(texAddrOffsets);

    GDInitGDLObj( &DrawDLO, DrawList, DRAW_SIZE );
    GDSetCurrent( &DrawDLO );
    
    // face 1

    // Set texture format.  This changes among the various faces.
    GDSetTexImgAttr( GX_TEXMAP0,        // tmap
                     128,               // width
                     128,               // height
                     GX_TF_RGB5A3 );    // format
    
    // Set the texture TMEM cache usage for this texture.
    // We make sure that each texture uses a different region.
    // (See the comment for MyTmemRegions up above.)
    GDSetTexCached( GX_TEXMAP0, MyTmemRegions[0][0], GX_TEXCACHE_32K,
                                MyTmemRegions[0][1], GX_TEXCACHE_32K );

    // Save the offset that will indicate the texture main-memory address.
    // We add a bit to skip over the command token and register ID bytes.
    texAddrOffsets[0] = GDGetCurrOffset() + BP_DATA_OFFSET;

    // Insert command to load the texture main-memory address.
    // We use the raw form since we don't have a valid address to use now.
    // The real address will be patched in later.
    GDSetTexImgPtrRaw( GX_TEXMAP0, 0 );

    // Draw face.
    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 0 ); GDNormal1x8( 0 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 1 ); GDNormal1x8( 1 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 2 ); GDNormal1x8( 2 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 3 ); GDNormal1x8( 3 ); GDTexCoord1x8( 3 );
    GDEnd();
    
    // That's it for this face.
    // The next 5 faces follow mostly the same layout.

    // face 2

    GDSetTexImgAttr( GX_TEXMAP0,        // tmap
                     128,               // width
                     128,               // height
                     GX_TF_CMPR );      // format
    
    GDSetTexCached( GX_TEXMAP0, MyTmemRegions[1][0], GX_TEXCACHE_32K,
                                MyTmemRegions[1][1], GX_TEXCACHE_32K );

    texAddrOffsets[1] = GDGetCurrOffset() + BP_DATA_OFFSET;

    GDSetTexImgPtrRaw( GX_TEXMAP0, 0 );

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 4 ); GDNormal1x8( 4 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 5 ); GDNormal1x8( 5 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 6 ); GDNormal1x8( 6 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 7 ); GDNormal1x8( 7 ); GDTexCoord1x8( 3 );
    GDEnd();
    
    // face 3

    GDSetTexImgAttr( GX_TEXMAP0,        // tmap
                     128,               // width
                     128,               // height
                     GX_TF_RGBA8 );     // format
    
    GDSetTexCached( GX_TEXMAP0, MyTmemRegions[2][0], GX_TEXCACHE_32K,
                                MyTmemRegions[2][1], GX_TEXCACHE_32K );

    texAddrOffsets[2] = GDGetCurrOffset() + BP_DATA_OFFSET;

    GDSetTexImgPtrRaw( GX_TEXMAP0, 0 );

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 2 ); GDNormal1x8( 2 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 6 ); GDNormal1x8( 6 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 5 ); GDNormal1x8( 5 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 3 ); GDNormal1x8( 3 ); GDTexCoord1x8( 3 );
    GDEnd();
    
    // face 4

    GDSetTexImgAttr( GX_TEXMAP0,        // tmap
                     128,               // width
                     128,               // height
                     GX_TF_I4 );        // format
    
    GDSetTexCached( GX_TEXMAP0, MyTmemRegions[3][0], GX_TEXCACHE_32K,
                                MyTmemRegions[3][1], GX_TEXCACHE_32K );

    texAddrOffsets[3] = GDGetCurrOffset() + BP_DATA_OFFSET;

    GDSetTexImgPtrRaw( GX_TEXMAP0, 0 );

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 1 ); GDNormal1x8( 1 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 0 ); GDNormal1x8( 0 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 4 ); GDNormal1x8( 4 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 7 ); GDNormal1x8( 7 ); GDTexCoord1x8( 3 );
    GDEnd();
    
    // face 5

    GDSetTexImgAttr( GX_TEXMAP0,        // tmap
                     128,               // width
                     128,               // height
                     GX_TF_IA8 );       // format
    
    GDSetTexCached( GX_TEXMAP0, MyTmemRegions[4][0], GX_TEXCACHE_32K,
                                MyTmemRegions[4][1], GX_TEXCACHE_32K );

    texAddrOffsets[4] = GDGetCurrOffset() + BP_DATA_OFFSET;

    GDSetTexImgPtrRaw( GX_TEXMAP0, 0 );

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 5 ); GDNormal1x8( 5 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 4 ); GDNormal1x8( 4 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 0 ); GDNormal1x8( 0 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 3 ); GDNormal1x8( 3 ); GDTexCoord1x8( 3 );
    GDEnd();
    
    // face 6

    GDSetTexImgAttr( GX_TEXMAP0,            // tmap
                     128,                   // width
                     128,                   // height
                     (GXTexFmt)GX_TF_C8 );  // format
    
    GDSetTexCached( GX_TEXMAP0, MyTmemRegions[5][0], GX_TEXCACHE_32K,
                                MyTmemRegions[5][1], GX_TEXCACHE_32K );

    // The color-index texture has different lookup properties:

    GDSetTexLookupMode( GX_TEXMAP0,             // tmap
                        GX_REPEAT,              // wrap s
                        GX_REPEAT,              // wrap t
                        GX_LIN_MIP_NEAR,        // min filt
                        GX_LINEAR,              // mag filt
                        0.0f,                   // min LOD
                        0.0f,                   // max LOD
                        0.0f,                   // LOD bias
                        GX_FALSE,               // bias clamp?
                        GX_TRUE,                // do edge LOD?
                        GX_ANISO_1 );           // max aniso

    texAddrOffsets[5] = GDGetCurrOffset() + BP_DATA_OFFSET;

    GDSetTexImgPtrRaw( GX_TEXMAP0, 0 );

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 6 ); GDNormal1x8( 6 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 2 ); GDNormal1x8( 2 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 1 ); GDNormal1x8( 1 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 7 ); GDNormal1x8( 7 ); GDTexCoord1x8( 3 );
    GDEnd();
    
    // pad & flush
    GDPadCurr32();
    GDFlushCurrToMem(); // not strictly needed for PC-side, but not a bad idea
    
    GDSetCurrent(NULL); // bug-prevention
}
