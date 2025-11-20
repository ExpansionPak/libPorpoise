/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gd-init-create.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-init-create.c $
    
    1     9/25/01 6:23p Carl
    Sources for GD init demo.
    
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

// This DL draws a cube.

extern GDLObj DrawDLO;
#define DRAW_SIZE 2048        // maximum (not actual) size

// This array indicates the offsets to patch memory addresses for the
// primitive data arrays (positions, colors) referred to in the Init DL.

extern u32 *setArrayOffsets;

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

    f32 identity_mtx[3][4];
    GXColor black = {0, 0, 0, 0};
    GXColor white = {255, 255, 255, 255};
    u32 i;

    //
    // Create the init-state DL
    //

    // This display list (for the most part) follows the settings of GXInit.
    // Whatever GXInit does (regarding GX API initialization), this DL also
    // does (wherever possible).
    //
    // We also set up a few things in order to draw a cube in the draw DL.

    InitStateList = OSAlloc( INIT_STATE_SIZE );
    ASSERT(InitStateList);

    setArrayOffsets = OSAlloc( 2 * sizeof(u32) );
    ASSERT(setArrayOffsets);

    GDInitGDLObj( &InitDLO, InitStateList, INIT_STATE_SIZE );
    GDSetCurrent( &InitDLO );
    
    // ----------------------------------------------------------------------

    // Here we follow GXInit, section by section.
    // The comments not in parenthesis are from the original GXInit code.

    //
    //  Geometry and Vertex
    //

    GDSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX3,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX4,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX5,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD6, GX_TG_MTX2x4, GX_TG_TEX6,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD7, GX_TG_MTX2x4, GX_TG_TEX7,
                     GX_FALSE, GX_PTIDENTITY);
    // (GDSetGenMode2: nTexgens, nChannels, nTevs, nIndirects, cullmode)
    GDSetGenMode2(1, 0, 1, 0, GX_CULL_BACK);

    // (GXInit just clears the VCD and does not touch the VATs.
    //  That is not so useful, so here as an example we set the
    //  VCD and VAT 0 as required by this demo.)
    {
        static GXVtxDescList vcd[] = {
            { GX_VA_POS, GX_INDEX8 },
            { GX_VA_CLR0, GX_INDEX8 },
            { GX_VA_NULL, GX_NONE }, // NULL terminator is required
        };
        static GXVtxAttrFmtList vat[] = {
            { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
            { GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 },
            { GX_VA_NULL, GX_POS_XY, GX_U8, 0 }, // NULL terminator is required
        };
        
        GDSetVtxDescv( vcd );
        GDSetVtxAttrFmtv( GX_VTXFMT0, vat );
    }

    // (Vertex cache invalidation is not part of GD.)

    // (For the line-aspect ratio, GXInit looks at the render mode in
    //  order to set this properly.  We assume frame-rendering mode here.)
    GDSetLPSize(6, 6, GX_TO_ZERO, GX_TO_ZERO, GX_DISABLE);
    
    // (Line/point texture-offset enables are set with texcoord scaling.)
    // (As an example, we show how to set one here; we do not set all 8.)
    GDSetTexCoordScaleAndTOEs( GX_TEXCOORD0,             // tcoord
                               128, GX_FALSE, GX_FALSE,  // s size, bias, wrap
                               128, GX_FALSE, GX_FALSE,  // t size, bias, wrap
                               GX_DISABLE, GX_DISABLE ); // tex-offset enables
    
    //
    //  Transformation and Matrix
    //

    identity_mtx[0][0] = 1.0f; identity_mtx[0][1] = 0.0f;
    identity_mtx[0][2] = 0.0f; identity_mtx[0][3] = 0.0f;
    identity_mtx[1][0] = 0.0f; identity_mtx[1][1] = 1.0f;
    identity_mtx[1][2] = 0.0f; identity_mtx[1][3] = 0.0f;
    identity_mtx[2][0] = 0.0f; identity_mtx[2][1] = 0.0f;
    identity_mtx[2][2] = 1.0f; identity_mtx[2][3] = 0.0f;

    // (Setting projection matrix is not part of GXInit or GD.)
    GDLoadPosMtxImm(identity_mtx, GX_PNMTX0);
    GDLoadNrmMtxImm(identity_mtx, GX_PNMTX0);
    GDSetCurrentMtx(GX_PNMTX0, 
                    GX_IDENTITY, GX_IDENTITY, GX_IDENTITY, GX_IDENTITY,
                    GX_IDENTITY, GX_IDENTITY, GX_IDENTITY, GX_IDENTITY);
    GDLoadTexMtxImm(identity_mtx, GX_IDENTITY, GX_MTX3x4);
    GDLoadTexMtxImm(identity_mtx, GX_PTIDENTITY, GX_MTX3x4);
    // (Setting viewport is not part of GD.)

    //
    //  Clipping and Culling
    //

    GDSetCoPlanar(GX_DISABLE);
    // (Setting clip mode is not part of GD.)
    // (Setting scissoring is not part of GD.)

    //
    //  Lighting - pass vertex color through
    //

    GDSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,
        GX_SRC_REG,
        GX_SRC_VTX,
        GX_LIGHT_NULL,
        GX_DF_NONE,
        GX_AF_NONE );

    GDSetChanAmbColor(GX_COLOR0A0, black);
    GDSetChanMatColor(GX_COLOR0A0, white);

    GDSetChanCtrl(
        GX_COLOR1A1,
        GX_DISABLE,
        GX_SRC_REG,
        GX_SRC_VTX,
        GX_LIGHT_NULL,
        GX_DF_NONE,
        GX_AF_NONE );

    GDSetChanAmbColor(GX_COLOR1A1, black);
    GDSetChanMatColor(GX_COLOR1A1, white);

    //
    //  Texture
    //

    // (Texture cache invalidation is not part of GD.)

    //  Allocate 8 32k caches for RGBA texture mipmaps.
    //  Equal size caches to support 32b RGBA textures.
    //

    // (This is the default TMEM configuration set in GXInit.
    //  Note that texmap ID's are bound to specific regions.)

    GDSetTexCached( GX_TEXMAP0, 0x00000, GX_TEXCACHE_32K,
                                0x80000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP1, 0x08000, GX_TEXCACHE_32K,
                                0x88000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP2, 0x10000, GX_TEXCACHE_32K,
                                0x90000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP3, 0x18000, GX_TEXCACHE_32K,
                                0x98000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP4, 0x20000, GX_TEXCACHE_32K,
                                0xa0000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP5, 0x28000, GX_TEXCACHE_32K,
                                0xa8000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP6, 0x30000, GX_TEXCACHE_32K,
                                0xb0000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP7, 0x38000, GX_TEXCACHE_32K,
                                0xb8000, GX_TEXCACHE_32K );

    //  Allocate color index caches in low bank of TMEM.
    //  Each cache is 32kB.
    //  Even and odd regions should be allocated on different address.
    //
#if 0
    // (This is an example of how you would rebind 4 texmap ID's
    //  to map to the setup suggested above.)

    GDSetTexCached( GX_TEXMAP0, 0x40000, GX_TEXCACHE_32K,
                                0x48000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP1, 0x50000, GX_TEXCACHE_32K,
                                0x58000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP2, 0x60000, GX_TEXCACHE_32K,
                                0x68000, GX_TEXCACHE_32K );
    GDSetTexCached( GX_TEXMAP3, 0x70000, GX_TEXCACHE_32K,
                                0x78000, GX_TEXCACHE_32K );
#endif

    //  Allocate TLUTs, 16 256-entry TLUTs and 4 1K-entry TLUTs.
    //  256-entry TLUTs are 8kB, 1k-entry TLUTs are 32kB.
    //
#if 0
    // (This is an example of how you would bind 4 texmap ID's
    //  to 4 256-entry RGB565 TLUT's and the other 4 texmap ID's
    //  to 4 1K-entry RGB565 TLUT's.  The bindings are only relevant
    //  if the texmaps are color-index format, of course.)

    GDSetTexTlut( GX_TEXMAP0, 0xc0000, GX_TL_RGB565 );
    GDSetTexTlut( GX_TEXMAP1, 0xc2000, GX_TL_RGB565 );
    GDSetTexTlut( GX_TEXMAP2, 0xc4000, GX_TL_RGB565 );
    GDSetTexTlut( GX_TEXMAP3, 0xc6000, GX_TL_RGB565 );

    GDSetTexTlut( GX_TEXMAP4, 0xe0000, GX_TL_RGB565 );
    GDSetTexTlut( GX_TEXMAP5, 0xe8000, GX_TL_RGB565 );
    GDSetTexTlut( GX_TEXMAP6, 0xf0000, GX_TL_RGB565 );
    GDSetTexTlut( GX_TEXMAP7, 0xf8000, GX_TL_RGB565 );
#endif    

    //
    //  Set texture region and tlut region Callbacks
    //

    // (GD does not have any concept of region callbacks.
    //  Texmap ID's are bound to regions explicitly.)

    //
    //  Texture Environment
    //
    GDSetTevOrder(GX_TEVSTAGE0,
                  GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0,
                  GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0 );
    GDSetTevOrder(GX_TEVSTAGE2,
                  GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0,
                  GX_TEXCOORD3, GX_TEXMAP3, GX_COLOR0A0 );
    GDSetTevOrder(GX_TEVSTAGE4,
                  GX_TEXCOORD4, GX_TEXMAP4, GX_COLOR0A0,
                  GX_TEXCOORD5, GX_TEXMAP5, GX_COLOR0A0 );
    GDSetTevOrder(GX_TEVSTAGE6,
                  GX_TEXCOORD6, GX_TEXMAP6, GX_COLOR0A0,
                  GX_TEXCOORD7, GX_TEXMAP7, GX_COLOR0A0 );

    GDSetTevOrder(GX_TEVSTAGE8,
                  GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL,
                  GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
    GDSetTevOrder(GX_TEVSTAGE10,
                  GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL,
                  GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
    GDSetTevOrder(GX_TEVSTAGE12,
                  GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL,
                  GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
    GDSetTevOrder(GX_TEVSTAGE14,
                  GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL,
                  GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );

    GDSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GDSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GDSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);

    for (i = GX_TEVSTAGE0; i < GX_MAX_TEVSTAGE; i+=2) {
        GDSetTevKonstantSel((GXTevStageID) i,
                            GX_TEV_KCSEL_1_4, GX_TEV_KASEL_1,
                            GX_TEV_KCSEL_1_4, GX_TEV_KASEL_1 );
        // (Swap mode is set with TevOp.)
    }
    GDSetTevSwapModeTable(GX_TEV_SWAP0,
                          GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GDSetTevSwapModeTable(GX_TEV_SWAP1,
                          GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
    GDSetTevSwapModeTable(GX_TEV_SWAP2,
                          GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
    GDSetTevSwapModeTable(GX_TEV_SWAP3,
                          GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);

    //  Indirect Textures.

    // (Indirect texturing is not yet part of GD.)

    //
    //  Pixel Processing
    //
    GDSetFog(GX_FOG_NONE, 0.0F, 1.0F, 0.1F, 1.0F, black);
    // (Fog range adjust in not part of GD.)
    GDSetBlendModeEtc(GX_BM_NONE,
                      GX_BL_SRCALPHA,    // src factor
                      GX_BL_INVSRCALPHA, // dst factor
                      GX_LO_CLEAR,
                      GX_ENABLE,         // color update
                      GX_ENABLE,         // alpha update
                      GX_ENABLE);        // dither enable

    GDSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    // (Z compare location is not part of GD.)
    GDSetDstAlpha(GX_DISABLE, 0);
    // (pixel format is not part of GD.)
    // (field mask is not part of GD.)
    // (field mode is not part of GD, except for line aspect ratio.)

    //
    //  Framebuffer
    //

    // (Framebuffer operations are not part of GD.)

    //
    //  CPU direct EFB access
    //

    // (This is not part of GD either.)

    //
    //  Performance Counters
    //

    // (Nor is this.)

    // ----------------------------------------------------------------------

    // That is all of the GXInit-related code.
    //
    // The rest of this DL is for the demo.

    // Set the vertex array pointers and strides.  The actual array
    // pointers will be patched in later, so we need to keep track of
    // these patch points.
    // We add a bit to skip over the command token and register ID bytes.

    setArrayOffsets[0] = GDGetCurrOffset() + CP_DATA_OFFSET;
    GDSetArrayRaw(GX_VA_POS, 0, 12);

    setArrayOffsets[1] = GDGetCurrOffset() + CP_DATA_OFFSET;
    GDSetArrayRaw(GX_VA_CLR0, 0, 4);
    
    // The actual TEV/texture/color-channel configuration we need
    // is different than what GXInit sets up, so we change it here:

    GDSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GDSetGenMode(0, 1, 1); // 0 texgens, 1 channel, 1 tevstage

    // pad & flush
    GDPadCurr32();
    GDFlushCurrToMem(); // not strictly needed for PC-side, but not a bad idea

    //
    // Create the cube drawing DL
    //

    // Draw DL: commands to draw the actual geometry.
    // Just draws a plain cube with color-indexed face colors.

    DrawList = OSAlloc( DRAW_SIZE );
    ASSERT(DrawList);

    GDInitGDLObj( &DrawDLO, DrawList, DRAW_SIZE );
    GDSetCurrent( &DrawDLO );
    
    // face 1

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 0 ); GDColor1x8( 0 );
    GDPosition1x8( 1 ); GDColor1x8( 0 );
    GDPosition1x8( 2 ); GDColor1x8( 0 );
    GDPosition1x8( 3 ); GDColor1x8( 0 );
    GDEnd();
    
    // face 2

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 4 ); GDColor1x8( 1 );
    GDPosition1x8( 5 ); GDColor1x8( 1 );
    GDPosition1x8( 6 ); GDColor1x8( 1 );
    GDPosition1x8( 7 ); GDColor1x8( 1 );
    GDEnd();
    
    // face 3

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 2 ); GDColor1x8( 2 );
    GDPosition1x8( 6 ); GDColor1x8( 2 );
    GDPosition1x8( 5 ); GDColor1x8( 2 );
    GDPosition1x8( 3 ); GDColor1x8( 2 );
    GDEnd();
    
    // face 4

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 1 ); GDColor1x8( 3 );
    GDPosition1x8( 0 ); GDColor1x8( 3 );
    GDPosition1x8( 4 ); GDColor1x8( 3 );
    GDPosition1x8( 7 ); GDColor1x8( 3 );
    GDEnd();
    
    // face 5

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 5 ); GDColor1x8( 4 );
    GDPosition1x8( 4 ); GDColor1x8( 4 );
    GDPosition1x8( 0 ); GDColor1x8( 4 );
    GDPosition1x8( 3 ); GDColor1x8( 4 );
    GDEnd();
    
    // face 6

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 6 ); GDColor1x8( 5 );
    GDPosition1x8( 2 ); GDColor1x8( 5 );
    GDPosition1x8( 1 ); GDColor1x8( 5 );
    GDPosition1x8( 7 ); GDColor1x8( 5 );
    GDEnd();
    
    // pad & flush
    GDPadCurr32();
    GDFlushCurrToMem(); // not strictly needed for PC-side, but not a bad idea
    
    GDSetCurrent(NULL); // bug-prevention
}
