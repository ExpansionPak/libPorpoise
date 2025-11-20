/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tev-swap.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Tev/tev-swap.c $    
    
    3     5/13/01 2:48p Hirose
    removed obsolete flags
    
    2     11/01/00 6:00p Hirose
    
    1     10/31/00 7:24p Hirose
    beta version check in
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tev-swap
        TEV swap mode table capability demo
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#if ( GX_REV == 1 ) // GX revision 1
#define NUM_MENU        1
#else               // GX revision 2 or later
#define NUM_MENU        5
#endif

#define NUM_PATTERNS    2
#define NUM_DONUTS      12

#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
   Structure definitions
 *---------------------------------------------------------------------------*/
// for entire scene control
typedef struct
{
    u32         patternNo;
    u32         cursor;
    u32         tblIdx;
    BOOL        blend;
    u16         screen_width;
    u16         screen_height;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main                ( void );
static void DrawInit            ( MySceneCtrlObj* sc );
static void DrawTick            ( MySceneCtrlObj* sc );
static void AnimTick            ( MySceneCtrlObj* sc );
static void DrawSampleTex       ( MySceneCtrlObj* sc );
static void DrawDonuts          ( MySceneCtrlObj* sc );
static void LoadTevSwapTable    ( void );
static void PrintIntro          ( void );

/*---------------------------------------------------------------------------*
  Model and texture data
 *---------------------------------------------------------------------------*/
#define NUM_COLORS  12
static GXColor MyColorArray[] ATTRIBUTE_ALIGN(32) = 
{
    { 0xFF, 0xFF, 0xFF, 0xFF }, // White
    { 0x00, 0xFF, 0xFF, 0x80 }, // Cyan
    { 0x00, 0xFF, 0x00, 0xC0 }, // Green
    { 0xFF, 0xFF, 0x00, 0xA0 }, // Yellow
    { 0xFF, 0x00, 0x00, 0xE0 }, // Red
    { 0xFF, 0x00, 0xFF, 0x60 }, // Magenta
    { 0x00, 0x00, 0xFF, 0x80 }, // Blue
    { 0x80, 0x80, 0x80, 0x40 }, // Gray
    { 0xFF, 0x80, 0x80, 0xC0 }, //
    { 0x80, 0xFF, 0x80, 0x80 }, //
    { 0x80, 0x80, 0xFF, 0x20 }, //
    { 0x00, 0x00, 0x00, 0xFF }  //
};

/*---------------------------------------------------------------------------*
   Swap mode table
 *---------------------------------------------------------------------------*/
static u8 MySwapModeTable[4][4] =
{
    { 0, 1, 2, 3 }, // RGBA
    { 0, 0, 0, 3 }, // RRRA
    { 1, 1, 1, 3 }, // GGGA
    { 2, 2, 2, 3 }, // BBBA
};

static char* MySwCompMsg[4] =
{
    "RED  ", "GREEN", "BLUE ", "ALPHA"
};


#if ( GX_REV == 1 ) // GX revision 1
static GXTevColorArg MySwCompTable[4] =
{
    GX_CC_TEXC, GX_CC_TEXRRR, GX_CC_TEXGGG, GX_CC_TEXBBB
};

#else // ( GX_REV >= 2 ) : GX revision 2 or later
static GXTevSwapSel MySwSelTable[4] =
{
    GX_TEV_SWAP0, GX_TEV_SWAP1, GX_TEV_SWAP2, GX_TEV_SWAP3
};

static GXTevColorChan MyChCompTable[4] =
{
    GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA
};

#endif

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;          // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;    // for TPL file

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);       // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl); // Initialize vertex formats, array pointers
                          // and default scene settings.

    PrintIntro();    // Print demo directions
   
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);    // Draw the model.
        DEMODoneRender();
        DEMOPadRead();           // Read controller
        AnimTick(&SceneCtrl);    // Do animation
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format and sets up
                    the array pointer for the indexed data.
                    This function also initializes scene control parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{
    GXRenderModeObj *rmp;

    // Get framebuffer size of current rendering mode
    rmp = DEMOGetRenderModeObj();
    sc->screen_width  = rmp->fbWidth;
    sc->screen_height = rmp->efbHeight;

    // Vertex Attribute (VTXFMT0 is used by DEMOPuts library)
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // load tpl file
    TEXGetPalette(&MyTplObj, "gxTests/tex-02/rgba8_1.tpl");

    // Default scene control parameter settings
    sc->patternNo = 0;
    sc->cursor    = 0;
    sc->tblIdx    = 0;
    sc->blend     = FALSE;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx44  proj;

    // Initialize screen space projection
    MTXOrtho(proj, 0, sc->screen_height, 0, sc->screen_width, 0.0F, 10000.0F);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);

    // Load TEV swap mode table setting into the hardware
    LoadTevSwapTable();

    // Alpha blending mode
    if ( sc->blend )
    {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    }
    else
    {
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
    }

    // Draw models
    if ( sc->patternNo == 0 )
    {
        DrawSampleTex(sc);
    }
    else
    {
        DrawDonuts(sc);
    }

    // Caption
    DEMOInitCaption(DM_FT_OPQ, (s16)(sc->screen_width/2), (s16)(sc->screen_height/2));
    DEMOPrintf(24, 16, 0, "Swap Mode Table : %d", sc->tblIdx);
    DEMOPrintf(24, 24, 0, "%s -> RED",   MySwCompMsg[MySwapModeTable[sc->tblIdx][0]]);
    DEMOPrintf(24, 32, 0, "%s -> GREEN", MySwCompMsg[MySwapModeTable[sc->tblIdx][1]]);
    DEMOPrintf(24, 40, 0, "%s -> BLUE",  MySwCompMsg[MySwapModeTable[sc->tblIdx][2]]);
    DEMOPrintf(24, 48, 0, "%s -> ALPHA", MySwCompMsg[MySwapModeTable[sc->tblIdx][3]]);
    // Cursor
    DEMOPrintf(12, (s16)(sc->cursor*8+16), 0, "%c", 0x7F);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    // Move cursor
    if ( DEMOPadGetDirsNew(0) & DEMO_STICK_UP )
    {
        sc->cursor += NUM_MENU - 1;
    }
    if ( DEMOPadGetDirsNew(0) & DEMO_STICK_DOWN )
    {
        sc->cursor++;
    }
    sc->cursor %= NUM_MENU;


    // Change swap mode parameters
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_A )
    {
        switch(sc->cursor)
        {
          case 0 :
            sc->tblIdx = ( sc->tblIdx + 1 ) % 4;
            break;
          case 1 :
          case 2 :
          case 3 :
          case 4 :
            MySwapModeTable[sc->tblIdx][sc->cursor - 1]++;
            MySwapModeTable[sc->tblIdx][sc->cursor - 1] %= 4;
            break;
        }
    }

    // Change the display pattern
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_B )
    {
        sc->patternNo = ++sc->patternNo % NUM_PATTERNS;
    }
    
    // Alpha blend mode
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_Y )
    {
        sc->blend = sc->blend ? FALSE : TRUE;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawSampleTex
    
    Description:    Draw a sample texture
                    which shows TEX component swap results
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawSampleTex( MySceneCtrlObj* sc )
{
    GXTexObj    tx;
    Mtx         mv;

    // Transform matrix
    MTXIdentity(mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    // No color channel
    GXSetNumChans(0);
    
    // Texture coord generation
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    // Set up texture
    TEXGetGXTexObjFromPalette(MyTplObj, &tx, 0);
    GXLoadTexObj(&tx, GX_TEXMAP0);

    // Set TEV operation for one texture
	GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

    // TEV swap mode setting
    // Note that these features don't work on the emulator
#if ( GX_REV == 1 )

    // GX revision 1 supports only RRR/GGG/BBB type swap as an
    // argument of GXSetTevColorIn
    GXSetTevColorIn( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO,
                     GX_CC_ZERO, MySwCompTable[sc->tblIdx] );
#else // ( GX_REV >= 2 )

    // GX revision 2 or later support swap mode table select
    GXSetTevSwapMode(
        GX_TEVSTAGE0,
        MySwSelTable[sc->tblIdx],   // ras_sel (actually not used here)
        MySwSelTable[sc->tblIdx] ); // tex_sel
#endif

    // Vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // Draw a large quad
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        GXPosition3s16( 64,  120, 0);
        GXTexCoord2s16(0x0000, 0x0000);
        GXPosition3s16(576,  120, 0);
        GXTexCoord2s16(0x0100, 0x0000);
        GXPosition3s16(576, 376, 0);
        GXTexCoord2s16(0x0100, 0x0100);
        GXPosition3s16( 64, 376, 0);
        GXTexCoord2s16(0x0000, 0x0100);
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawDonuts
    
    Description:    Draw lit colored doughnuts
                    that show RAS component swap results
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawDonuts( MySceneCtrlObj* sc )
{
#if ( GX_REV == 1 )
    #pragma unused(sc)
#endif

    static u32  rot = 0;
    GXLightObj  lo;
    u32         i;
    Mtx         m0, m1, ms;
    GXColor		ambCol = { 64, 64, 64, 64 };
    GXColor		litCol = { 192, 192, 192, 192 };

    // Lighting on
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR0A0, ambCol);
    
    GXInitLightColor(&lo, litCol);
    GXInitLightPos(&lo, 10000.0F, -10000.0F, 5000.0F);
    GXLoadLightObjImm(&lo, GX_LIGHT0);
    
    // No texgen
    GXSetNumTexGens(0);

    // Set TEV operation to use vertex color
	GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // TEV swap mode
    // Note that this feature doesn't work on the emulator
#if ( GX_REV != 1 )

    // Swap mode for RAS component is available on GX revision 2 or later.
    GXSetTevSwapMode(
        GX_TEVSTAGE0,
        MySwSelTable[sc->tblIdx],   // ras_sel
        MySwSelTable[sc->tblIdx] ); // tex_sel (actually not used here)
#endif


    // Transform matrix
    GXSetCurrentMtx(GX_PNMTX0);

    // Draw each torus
    MTXScale(ms, 40, -40, 40);
    for ( i = 0 ; i < NUM_DONUTS ; ++i )
    {
        MTXRotDeg(m0, 'y', (rot+i*60));
        MTXConcat(ms, m0, m1);
        MTXTrans(m0, ((i%4)+1)*128, (i/4)*96+160, -256);
        MTXConcat(m0, m1, m1);
        GXLoadPosMtxImm(m1, GX_PNMTX0);
        
        GXSetChanMatColor(GX_COLOR0A0, MyColorArray[i]);
        GXDrawTorus(0.3F, 12, 16);
    }

    // Rotation counter
    rot = ++rot % 360;
}

/*---------------------------------------------------------------------------*
    Name:           LoadTevSwapTable
    
    Description:    Load TEV swap mode table setting into hardware
                    
    Arguments:      sc : scene control object
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void LoadTevSwapTable( void )
{
#if ( GX_REV != 1 ) // GX Revision 2 or later
    u32  i;
    
    for ( i = 0 ; i < 4 ; ++i )
    {
        GXSetTevSwapModeTable(
            MySwSelTable[i],
            MyChCompTable[MySwapModeTable[i][0]],
            MyChCompTable[MySwapModeTable[i][1]],
            MyChCompTable[MySwapModeTable[i][2]],
            MyChCompTable[MySwapModeTable[i][3]] );
    }
    
#endif
}

/*---------------------------------------------------------------------------*
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("tev-swap: TEV swap mode table demo\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start/pause button\n");
    OSReport("\n");
    OSReport("Stick Up/Down : Move the cursor\n");
    OSReport("A Button      : Change the value\n");
    OSReport("B Button      : Change the test pattern\n");
    OSReport("Y Button      : Turn on/off alpha blending\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
