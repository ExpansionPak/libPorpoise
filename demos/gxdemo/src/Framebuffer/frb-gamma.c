/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     frb-gamma.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Framebuffer/frb-gamma.c $    
    
    8     5/05/01 10:05p Hirose
    removed obsolete flags
    
    7     10/31/00 6:24p Hirose
    deleted unnecessary part
    
    6     7/07/00 5:57p Dante
    PC Compatibility
    
    5     6/20/00 6:06p Hirose
    added lit octahedron pattern
    
    4     6/10/00 1:27a Hirose
    added one more pattern / pixel mode select
    
    3     6/08/00 7:42p Hirose
    added two more test patterns
    
    2     6/07/00 11:57p Hirose
    implemented colorbar pattern test
    
    1     6/07/00 5:03p Hirose
    initial check in
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    frb-gamma
        gamma correction mode demo
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define MAX_Z           0x00ffffff // max value of Z buffer

#define NUM_PATTERNS    5
#define NUM_COLORBARS   12 
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
    u32         gammaMode;
    u32         pixMode;
    u32         colorID;
    u16         screen_width;
    u16         screen_height;
    u8*         vfilter;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main                ( void );
static void DrawInit            ( MySceneCtrlObj* sc );
static void DrawTick            ( MySceneCtrlObj* sc );
static void AnimTick            ( MySceneCtrlObj* sc );
static void DrawColorBars       ( MySceneCtrlObj* sc );
static void DrawGammaTestImg    ( MySceneCtrlObj* sc );
static void DrawSampleImg       ( MySceneCtrlObj* sc );
static void DrawDonuts          ( MySceneCtrlObj* sc );
static void DrawLitOcta         ( MySceneCtrlObj* sc );
static void PrintIntro          ( void );

/*---------------------------------------------------------------------------*
  Model and texture data
 *---------------------------------------------------------------------------*/
#define NUM_COLORS  13

#define ID_BLACK    0
#define ID_WHITE    1
#define ID_GRAY     8

static GXColor MyColorArray[] ATTRIBUTE_ALIGN(32) = 
{
    { 0x00, 0x00, 0x00, 0x00 }, // Black
    { 0xFF, 0xFF, 0xFF, 0xFF }, // White
    { 0x00, 0xFF, 0xFF, 0xFF }, // Cyan
    { 0x00, 0xFF, 0x00, 0xFF }, // Green
    { 0xFF, 0xFF, 0x00, 0xFF }, // Yellow
    { 0xFF, 0x00, 0x00, 0xFF }, // Red
    { 0xFF, 0x00, 0xFF, 0xFF }, // Magenta
    { 0x00, 0x00, 0xFF, 0xFF }, // Blue
    { 0x80, 0x80, 0x80, 0xFF }, // Gray
    { 0xFF, 0x80, 0x80, 0xFF }, //
    { 0x80, 0xFF, 0x80, 0xFF }, //
    { 0x80, 0x80, 0xFF, 0xFF }, //
    { 0x40, 0x40, 0x40, 0xFF }  //
};

/*---------------------------------------------------------------------------*
   Gamma correction mode data / pixel mode data
 *---------------------------------------------------------------------------*/
static GXGamma GammaTable[3] =
{
    GX_GM_1_0, GX_GM_1_7, GX_GM_2_2
};

static char* GammaMsg[3] =
{
    "1.0", "1.7", "2.2"
};

static GXPixelFmt PixModeTable[3] =
{
    GX_PF_RGB8_Z24, GX_PF_RGBA6_Z24, GX_PF_RGBA6_Z24
};

static GXBool DitherModeTable[3] =
{
    GX_DISABLE, GX_DISABLE, GX_ENABLE
};

static char* PixModeMsg[3] =
{
    "    RGB8", "    RGBA6", "RGBA6/Dither"
};

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

    OSHalt("End of test");
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
    sc->vfilter       = rmp->vfilter;

    // Background color
    GXSetCopyClear(MyColorArray[ID_BLACK], MAX_Z);

    // Culling mode
    GXSetCullMode(GX_CULL_NONE);

    // Vertex Attribute (VTXFMT0 is used by DEMOPuts library)
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // load tpl file
    TEXGetPalette(&MyTplObj, "gxTests/frb-00.tpl");


    // Default scene control parameter settings

    sc->patternNo = 0;

    // gamma mode & pixel mode
    sc->gammaMode = 0;
    sc->pixMode   = 0;
    sc->colorID   = 1;
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

    // Set pixel mode and gamma mode
    GXSetPixelFmt(PixModeTable[sc->pixMode], GX_ZC_LINEAR);
    GXSetDither(DitherModeTable[sc->pixMode]);
    GXSetDispCopyGamma(GammaTable[sc->gammaMode]);

    // Initialize screen space projection
    MTXOrtho(proj, 0, sc->screen_height, 0, sc->screen_width, 0.0F, 10000.0F);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);

    // Draw a pattern
    switch(sc->patternNo)
    {
      case 0:
        DrawColorBars(sc);
        break;
      case 1:
        DrawGammaTestImg(sc);
        break;
      case 2:
        DrawSampleImg(sc);
        break;
      case 3:
        DrawDonuts(sc);
        break;
      case 4:
      default:
        DrawLitOcta(sc);
        break;
    }

    // Caption
    DEMOInitCaption(DM_FT_OPQ, (s16)(sc->screen_width/2), (s16)(sc->screen_height/2));
    DEMOPrintf(16, 12, 0, "Gamma = %s\n", GammaMsg[sc->gammaMode]);
    DEMOPuts(200, 12, 0, PixModeMsg[sc->pixMode]);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    // Change the pattern
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_A )
    {
        sc->patternNo = ++sc->patternNo % NUM_PATTERNS;
    }

    // Gamma mode
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_B )
    {
        sc->gammaMode = ++sc->gammaMode % 3;
    }

    // Pixel mode (8bit/6bit)
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_X )
    {
        sc->pixMode = ++sc->pixMode % 3;
    }
    
    // Color ID (used in DrawLitOcta)
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_Y )
    {
        sc->colorID = ++sc->colorID % NUM_COLORS;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawColorBars
    
    Description:    Draw color bars
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawColorBars( MySceneCtrlObj* sc )
{
    u32     i;
    u8      c0, c1;
    Mtx     mv;

    // Turn de-flicker filter on
    GXSetCopyFilter(GX_FALSE, NULL, GX_TRUE, sc->vfilter);

    // Transform matrix
    MTXIdentity(mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    // Lighting off
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX,
                  GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    
    // Set TEV operation to use vertex color
    GXSetNumTexGens(0);
	GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // Vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);

    // Array pointer
    GXSetArray(GX_VA_CLR0, MyColorArray, sizeof(GXColor));

    // Draw each color bar
    GXBegin(GX_QUADS, GX_VTXFMT1, NUM_COLORBARS*4);
    for ( i = 0 ; i < NUM_COLORBARS ; ++i )
    {
        c0 = (u8)((i % 2) ? i : 0);
        c1 = (u8)((i % 2) ? 0 : i);
    
        GXPosition3s16(64,  (s16)(i*32+32), 0);
        GXColor1x8(c0);
        GXPosition3s16(576, (s16)(i*32+32), 0);
        GXColor1x8(c1);
        GXPosition3s16(576, (s16)(i*32+64), 0);
        GXColor1x8(c1);
        GXPosition3s16(64,  (s16)(i*32+64), 0);
        GXColor1x8(c0);
   }
   GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawGammaTestImg
    
    Description:    Draw a gamma test pattern
    
    Arguments:      sc  : pointer to the structure of scene control parameters
 
    Returns:        none
 *---------------------------------------------------------------------------*/
// Configuration data
struct
{
    u32     x;
    u32     y;
    GXColor color0;
    GXColor color1; 
}
GammaImgConfig[4] =
{
    { 160,  64, { 255, 255, 255, 255 }, { 128, 128, 128, 255 } },
    { 352,  64, { 255,   0,   0, 255 }, { 128,   0,   0, 255 } },
    { 160, 256, {   0, 255,   0, 255 }, {   0, 128,   0, 255 } },
    { 352, 256, {   0,   0, 255, 255 }, {   0,   0, 128, 255 } }
};

static void DrawGammaTestImg( MySceneCtrlObj* sc )
{
    u32         i;
    GXTexObj    tx0, tx1;
    Mtx         mv;

    // Turn de-flicker filter off
    GXSetCopyFilter(GX_FALSE, NULL, GX_FALSE, sc->vfilter);

    // Lighting off (uses fixed color)
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    
    // Set TEV operation to use one color/texture each
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    // Set up textures
    TEXGetGXTexObjFromPalette(MyTplObj, &tx0, 0);  // Stripe pattern
    TEXGetGXTexObjFromPalette(MyTplObj, &tx1, 1);  // Reference pattern
    GXInitTexObjLOD(&tx0, GX_NEAR, GX_NEAR, 0, 0, 0, 0, 0, GX_ANISO_1);
    GXInitTexObjLOD(&tx1, GX_NEAR, GX_NEAR, 0, 0, 0, 0, 0, GX_ANISO_1);

    // Vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GXSetCurrentMtx(GX_PNMTX0);

    // Draw four patterns
    for ( i = 0 ; i < 4 ; ++i )
    {
        MTXTrans(mv, GammaImgConfig[i].x, GammaImgConfig[i].y, 0.0F);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
    
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
        GXLoadTexObj(&tx0, GX_TEXMAP0);
        GXSetChanMatColor(GX_COLOR0A0, GammaImgConfig[i].color0);
        
        GXBegin(GX_QUADS, GX_VTXFMT1, 4);
            GXPosition3s16(  0,   0, 0);
            GXTexCoord2s16(0x0000, 0x0000);
            GXPosition3s16(128,   0, 0);
            GXTexCoord2s16(0x0100, 0x0000);
            GXPosition3s16(128, 128, 0);
            GXTexCoord2s16(0x0100, 0x0100);
            GXPosition3s16(  0, 128, 0);
            GXTexCoord2s16(0x0000, 0x0100);
        GXEnd();
        
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
        GXLoadTexObj(&tx1, GX_TEXMAP0);
        GXSetChanMatColor(GX_COLOR0A0, GammaImgConfig[i].color1);
        
        GXBegin(GX_QUADS, GX_VTXFMT1, 4);
            GXPosition3s16(  0,   0, 0);
            GXTexCoord2s16(0x0000, 0x0000);
            GXPosition3s16(128,   0, 0);
            GXTexCoord2s16(0x0100, 0x0000);
            GXPosition3s16(128, 128, 0);
            GXTexCoord2s16(0x0100, 0x0100);
            GXPosition3s16(  0, 128, 0);
            GXTexCoord2s16(0x0000, 0x0100);
        GXEnd();
    }

    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);

}

/*---------------------------------------------------------------------------*
    Name:           DrawSampleImg
    
    Description:    Draw a sample image
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawSampleImg( MySceneCtrlObj* sc )
{
    GXTexObj    tx;
    Mtx         mv;

    // De-flicker filter on
    GXSetCopyFilter(GX_FALSE, NULL, GX_TRUE, sc->vfilter);

    // Transform matrix
    MTXIdentity(mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    // No color channel
    GXSetNumChans(0);
    
    // Set TEV operation to use vertex color
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

    // Set up texture
    TEXGetGXTexObjFromPalette(MyTplObj, &tx, 2);  // Sample image
    GXLoadTexObj(&tx, GX_TEXMAP0);

    // Vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // Draw a large quad
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        GXPosition3s16( 64,  96, 0);
        GXTexCoord2s16(0x0000, 0x0000);
        GXPosition3s16(576,  96, 0);
        GXTexCoord2s16(0x0100, 0x0000);
        GXPosition3s16(576, 352, 0);
        GXTexCoord2s16(0x0100, 0x0100);
        GXPosition3s16( 64, 352, 0);
        GXTexCoord2s16(0x0000, 0x0100);
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawDonuts
    
    Description:    Draw doughnuts
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawDonuts( MySceneCtrlObj* sc )
{
    static u32  rot = 0;
    GXLightObj  lo;
    u32         i;
    Mtx         m0, m1, ms;
	GXColor		color1 = { 64, 64, 64, 64 };
	GXColor		color2 = { 192, 192, 192, 192 };

    // De-flicker filter on
    GXSetCopyFilter(GX_FALSE, NULL, GX_TRUE, sc->vfilter);

    // Z compare on
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

    // Lighting on
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR0A0, color1);
    
    GXInitLightColor(&lo, color2);
    GXInitLightPos(&lo, 10000.0F, -10000.0F, 5000.0F);
    GXLoadLightObjImm(&lo, GX_LIGHT0);
    
    // Set TEV operation to use vertex color
    GXSetNumTexGens(0);
	GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // Transform matrix
    GXSetCurrentMtx(GX_PNMTX0);

    // Draw each icosahedron
    MTXScale(ms, 48, 48, 48);
    for ( i = 0 ; i < NUM_DONUTS ; ++i )
    {
        MTXRotDeg(m0, 'y', (rot+i*60));
        MTXConcat(ms, m0, m1);
        MTXTrans(m0, ((i%4)+1)*128, (i/4)*112+128, -256);
        MTXConcat(m0, m1, m1);
        GXLoadPosMtxImm(m1, GX_PNMTX0);
        
        GXSetChanMatColor(GX_COLOR0A0, MyColorArray[i+1]);
        GXDrawTorus(0.3F, 12, 16);
    }

    // Rotation counter
    rot = ++rot % 360;
}

/*---------------------------------------------------------------------------*
    Name:           DrawLitOcta
    
    Description:    Draw a lit octahedron
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawLitOcta( MySceneCtrlObj* sc )
{
    static u32  rot = 0;
    GXLightObj  lo;
    Mtx         mv, mr, ms, m1;
	GXColor		color1 = { 64, 64, 64, 64 };
	GXColor		color2 = { 160, 160, 160, 160 };

    // De-flicker filter on
    GXSetCopyFilter(GX_FALSE, NULL, GX_TRUE, sc->vfilter);

    // Z compare on
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

    // Lighting on
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR0A0, color1);
    GXSetChanMatColor(GX_COLOR0A0, MyColorArray[sc->colorID]);
    
    GXInitLightColor(&lo, color2);
    GXInitLightPos(&lo, 10000.0F, -10000.0F, 5000.0F);
    GXLoadLightObjImm(&lo, GX_LIGHT0);
    
    // Set TEV operation to use vertex color
    GXSetNumTexGens(0);
	GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // Transform matrix
    GXSetCurrentMtx(GX_PNMTX0);

    // Draw an octahedron
    MTXTrans(m1, 320, 240, -512);
    MTXRotDeg(mr, 'y', (f32)rot/2);
    MTXConcat(m1, mr, mv);
    MTXScale(ms, 192, 192, 192);
    MTXConcat(mv, ms, m1);
    
    GXLoadPosMtxImm(m1, GX_PNMTX0);
    GXLoadNrmMtxImm(m1, GX_PNMTX0);
    
    GXDrawOctahedron();

    // Rotation counter
    rot = ++rot % 720;
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
    OSReport("frb-gamma: gamma correction mode demo\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("A Button : Change the pattern\n");
    OSReport("B Button : Change gamma correction mode\n");
    OSReport("X Button : Change the pixel format\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
