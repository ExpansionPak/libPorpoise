/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     smp-toy-draw.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Simple/smp-toy-draw.c $    
    
    8     2/15/01 11:30a Hirose
    removed obsolete ifdefs
    
    7     10/28/00 5:56p Hirose
    
    6     10/25/00 8:59p Hirose
    A flag fix MAC -> MACOS
    
    5     8/30/00 6:27p Hirose
    removed MAC only instructions from other targets
    
    4     3/27/00 3:24p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/17/00 2:50p Alligator
    add ifdef BUG_NO_8b_SCALE
    
    1     3/16/00 7:24p Alligator
    rename to match other demos conventions
    
    1     3/06/00 12:11p Alligator
    move from gxdemos to gxdemos/Simple
    
    4     1/18/00 7:00p Alligator
    updated to work with new GXInit defaults
    
    3     1/13/00 8:53p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/10/99 4:01p Carl
    Removed GXSetZUpdate.
    
    23    11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    22    11/15/99 4:49p Yasu
    Change datafile name
    
    21    11/12/99 4:59p Yasu
    Add GXSetNumTexGens
    
    20    11/03/99 7:41p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    19    11/03/99 7:39p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    18    10/06/99 11:13a Alligator
    tex coords cast to u16, not u8
    delete unused init code
    
    17    10/06/99 11:22a Hashida
    Removed MyVIInit, which was not called from anywhere.
    
    16    9/30/99 10:33p Yasu
    Renamed some functions and enums
    
    15    9/30/99 2:13p Ryan
    sweep to remove gxmodels libs
    
    14    9/17/99 5:26p Ryan
    added new DEMO calls
    
    13    9/15/99 3:00p Ryan
    update to fix compiler warnings
    
    12    9/02/99 3:18p Ryan
    Made Frame Buffer Api changes
    
    11    9/01/99 5:45p Ryan
    
    10    8/27/99 3:36p Yasu
    Change the parameter of GXSetZMode() and append GXSetZUpdate()	    
    
    9     8/26/99 2:46p Yasu
    Replace GXSetColorMode() to GXSetBlendMode().
    
    8     7/30/99 1:39p Ryan
    changed demo so that it loads a tpl file for textures
    
    7     7/28/99 4:07p Alligator
    update header files and emulator for hw changes
    
    6     7/23/99 2:51p Ryan
    changed dolphinDemo.h to demo.h
    
    5     7/23/99 12:15p Ryan
    included dolphinDemo.h
    
    4     7/20/99 6:03p Alligator
    demo library
    
    3     7/06/99 2:25p Alligator
    modified to use use new GXInitTexObj api
    
    2     7/02/99 2:55p Alligator
    fixed matrix transpose
    
    1     6/17/99 1:25a Alligator
    simple interactive toy.  demonstrates texture api, multiple vertex
    formats and many primitive types.
    

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include "toy-stick.h"
#include "toy-anim.h"


#define MAX_SCOORD         0x4000  // for 16b, 2.14 fixed point format
#define MAX_TCOORD         0x4000  // for 16b, 2.14 fixed point format
#define STEP_SCOORD        4
#define STEP_TCOORD        5

#define TILTX_SCALE        10.0F
#define TILTZ_SCALE        10.0F
#define BOX_CENTROIDX      70.0F
#define BOX_CENTROIDZ      70.0F

#define WATER1_TEX_ID		3
#define BALL32_TEX_ID		7

/*---------------------------------------------------------------------------*
  Model Data
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   It IS NOT necessary to align attribute arrays to 32B, but doing so may
   result in a slight performance improvement.
   The macro ATTRIBUTE_ALIGN provides a convenient way to align initialized 
   arrays.
 *---------------------------------------------------------------------------*/

static s16 TopVerts[] ATTRIBUTE_ALIGN(32) = 
{
      0,   0,   0, // 0
    140,   0,   0, // 1
      0,   0, 140, // 2
    140,   0, 140, // 3
     20,   0,  20, // 4
    120,   0,  20, // 5
     20,   0, 120, // 6
    120,   0, 120, // 7
     40,   0,  20, // 8
     60,   0,  20, // 9
     80,   0,  20, // 10
    100,   0,  20, // 11
     80,   0,  40, // 12
    100,   0,  40, // 13
     60,   0,  60, // 14
    100,   0,  60, // 15
     40,   0,  80, // 16
     80,   0,  80, // 17
     40,   0, 100, // 18
     60,   0, 100, // 19
     40,   0, 120, // 20
     60,   0, 120, // 21
     80,   0, 120, // 22
    100,   0, 120, // 23
      0, -70,   0, // 24
    140, -70,   0, // 25
      0, -70, 140, // 26
    140, -70, 140, // 27
     20, -10,  20, // 28
     60, -10,  20, // 29
     60, -10,  60, // 30
    100, -10,  60, // 31
    100, -10, 120, // 32
     60, -10,  20, // 33
    100, -10,  20, // 34
    100, -10,  45, // 35
     60, -10, 105, // 36
     60, -10, 120, // 37
    100, -10,  20, // 38
    120, -10,  20, // 39
    120, -10, 120, // 40
     80, -10, 120, // 41
     40, -10, 120, // 42
     20, -10, 120  // 43
};


static u8 TexCoords_u8[] ATTRIBUTE_ALIGN(32) = 
{
//  s,    t  (8.0 fixed point)
    0x00, 0x00,   // 0
    0x01, 0x00,   // 1
    0x01, 0x01,   // 2
    0x00, 0x01    // 3
};

static u8 Colors[] ATTRIBUTE_ALIGN(32) = 
{ 
//   r    g    b    a
    255, 255, 255, 255, // 0
    124, 120, 128, 255, // 1
     60,  60,  64, 255, // 2
    200, 200, 200, 255, // 3
     95,  95, 100, 255, // 4
     54,  54,  58, 255  // 5
};

static GXTexObj WaterTexObj;
static GXTexObj BallTexObj;

static TEXPalettePtr tpl = 0;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
 
void        main            ( void );
static void CameraInit      ( Mtx v );
static void DrawInit        ( void );
static void DrawTick        ( Mtx v );

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{
    Mtx     v;   // view matrix

    DEMOInit(NULL);  // init os, pad, gx, vi

    CameraInit(v); // Initialize the camera.  
    DrawInit();    // Define my vertex formats and set array pointers.

    OSReport("***************************************\n");
    OSReport("to play:\n");
    OSReport("    Use the STICK X/Y to tilt the table and roll the ball.\n");
    OSReport("***************************************\n\n");
#ifdef MACOS
    OSReport("***************************************\n");
    OSReport("to quit:\n");
    OSReport("    hit the START button\n");
    OSReport("    click on the text output window\n");
    OSReport("    select 'quit' from the menu or hit 'command q'\n");
    OSReport("    select 'don't save'\n");
    OSReport("***************************************\n\n");
#endif // MACOS

    while(!StickDone())
    {   
		DEMOBeforeRender();
        DrawTick(v);        // Draw the model.
        DEMODoneRender();
        StickTick();        // Read controller
        AnimTick();         // Update animation.
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.
                    Initialize the view matrix.
                    
    Arguments:      v      view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit ( Mtx v )
{
    Mtx44   p;      // projection matrix
    Vec     up      = {0.0F, 0.0F, 1.0F};
    Vec     camLoc  = {155.0F, 200.0F, -5.0F};
    Vec     objPt   = {70.0F, 0.0F, 70.0F};
    f32     top     = 24.0F;
    f32     bot     = -top;
    f32     right   = 32.0F;
    f32     left    = -right;
    f32     znear   = 50.0F;
    f32     zfar    = 2000.0F;
    
    MTXFrustum(p, top, bot, left, right, znear, zfar);
    GXSetProjection(p, GX_PERSPECTIVE);
    
    MTXLookAt(v, &camLoc, &up, &objPt);    
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes vertex attribute formats, and sets
                    the array pointers and strides for the indexed data. Also
                    sets some default state for this app.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{ 
    GXColor black = {0, 0, 0, 0};

    GXSetCopyClear(black, 0x0FFFFFF);

    // show inside walls of box
    GXSetCullMode(GX_CULL_NONE);

    /*
        Vertex Attribute Format 0
      
     */
    //  Position has 3 elements (x,y,z), each of type s16,
    //  no fractional bits (integers)
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    // Clr0 has 4 components (r, g, b, a), each component is 8b.
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);   
    // Texture coordinate is 16-bit fixed-point, with 14 fractional bits
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 14);

    /*
        Vertex Attribute Format 1
     */
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);   
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);

    /*
        Set Array Pointers and Strides
     */
    // stride = 3 elements (x,y,z) each of type s16
    GXSetArray(GX_VA_POS, TopVerts, 3*sizeof(s16));
    // stride = 2 elements (s,t) each of type u8
    GXSetArray(GX_VA_CLR0, Colors, 4*sizeof(u8));
    // Texture coordinate has 2 components (s,t), each component is 8b
    GXSetArray(GX_VA_TEX0, TexCoords_u8, 2*sizeof(u8));

    /*
        Init texture objects, from gxTextrs.tpl
     */
	TEXGetPalette(&tpl, "gxTextrs.tpl");
	// Initialize a texture object to contain the correct texture
	TEXGetGXTexObjFromPalette(tpl, &WaterTexObj, WATER1_TEX_ID);
	TEXGetGXTexObjFromPalette(tpl, &BallTexObj, BALL32_TEX_ID);
}

/*---------------------------------------------------------------------------*
    Name:           VertexC
    
    Description:    Draw a vertex with color
                    
    Arguments:      v        8-bit position index
                    c        8-bit color index
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void VertexC( u8 v, u8 c )
{
    GXPosition1x8(v);
    GXColor1x8(c);
}

/*---------------------------------------------------------------------------*
    Name:           VertexT
    
    Description:    Draw a vertex with texture, direct data
                    
    Arguments:      v        8-bit position index
                    s, t     16-bit tex coord
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void VertexT( u8 v, u16 s, u16 t )
{
    GXPosition1x8(v);
    GXTexCoord2u16(s, t);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTexQuad
    
    Description:    Draw a textured quad.  Map extends to corners of the quad.
                    MAX_SCOORD is the value of 1.0 in the fixed point format.

    Arguments:      v0        8-bit position
                    v1        8-bit position
                    v2        8-bit position
                    v3        8-bit position
                    s0        16-bit s tex coord at v0
                    t0        16-bit t tex coord at v0
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void DrawTexQuad(
    u8  v0,
    u8  v1,
    u8  v2,
    u8  v3,
    u16 s0,
    u16 t0 ) 
{
    VertexT(v0, s0, t0);
    VertexT(v1, (u16)(s0+MAX_SCOORD), (u16)t0);
    VertexT(v2, (u16)(s0+MAX_SCOORD), (u16)(t0+MAX_TCOORD));
    VertexT(v3, (u16)s0, (u16)(t0+MAX_TCOORD));
}

/*---------------------------------------------------------------------------*
    Name:           DrawFSQuad
    
    Description:    Draw a flat shaded quad.
                    
    Arguments:      v0       8-bit position
                    v1       8-bit position
                    v2       8-bit position
                    v3       8-bit position
                    c        color
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void DrawFSQuad(
    u8 v0,
    u8 v1,
    u8 v2,
    u8 v3,
    u8 c ) 
{
    VertexC(v0, c);
    VertexC(v1, c);
    VertexC(v2, c);
    VertexC(v3, c);
}

/*---------------------------------------------------------------------------*
    Name:           DrawSSQuad
    
    Description:    Draw a smooth shaded quad.
                    
    Arguments:      v0       8-bit position
                    c0       color at v0
                    v1       8-bit position
                    c1       color at v1
                    v2       8-bit position
                    c2       color at v2
                    v3       8-bit position
                    c3       color at v3
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void DrawSSQuad(
    u8 v0, u8 c0,
    u8 v1, u8 c1,
    u8 v2, u8 c2,
    u8 v3, u8 c3 ) 
{
    VertexC(v0, c0);
    VertexC(v1, c1);
    VertexC(v2, c2);
    VertexC(v3, c3);
}

/*---------------------------------------------------------------------------*
    Name:           DrawBall
    
    Description:    Draw texture image of a ball at the given position. 
                    Uses Direct data for x,y,z.  Uses indexed data for
                    texture coordinates.
                    
    Arguments:      x,y,z    position to draw ball
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawBall( f32 x, f32 y, f32 z )
{
    // set Tev to pass texture color and alpha
    GXSetNumChans( 0 );
    GXSetNumTexGens( 1 );
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    // set texture object
    GXLoadTexObj(&BallTexObj, GX_TEXMAP0);
    // enable alpha blending
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, 
                   GX_LO_CLEAR);
    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    
    // draw ball texture
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        GXPosition3f32( x - BALL_SIZEX, y, z - BALL_SIZEZ );
        GXTexCoord1x8(2);
        GXPosition3f32( x + BALL_SIZEX, y, z - BALL_SIZEZ );
        GXTexCoord1x8(3);
        GXPosition3f32( x + BALL_SIZEX, y, z + BALL_SIZEZ );
        GXTexCoord1x8(0);
        GXPosition3f32( x - BALL_SIZEX, y, z + BALL_SIZEZ );
        GXTexCoord1x8(1);
    GXEnd();
    
    // restore default value, disable blending
    GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, 
                   GX_LO_CLEAR);
}

/*---------------------------------------------------------------------------*
    Name:           DrawBottom
    
    Description:    Draw bottom of box model.  Animates a texture along the
                    bottom of the box.  The order the parts are drawn is
                    important since zbuffering is turned off.
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawBottom( void )
{
    static u16 s = 0;
    static u16 t = 0;

    // use texture in Tev
    GXSetNumChans( 0 );
    GXSetNumTexGens( 1 );
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    // set texture object
    GXLoadTexObj(&WaterTexObj, GX_TEXMAP0);
    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // draw water texture on bottom of box
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            DrawTexQuad(25, 27, 26, 24, s, t);
    GXEnd();
    
    // translate s, t coordinates
    s += STEP_SCOORD; 
    t += STEP_TCOORD;
    if (s  > MAX_SCOORD) s = 0;
    if (t  > MAX_TCOORD) t = 0;

    // select Color0 in Tev
    GXSetNumChans( 1 );
    GXSetNumTexGens( 0 );
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // change vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);

    GXBegin(GX_QUADS, GX_VTXFMT0, 44);
        // sides of cube
        DrawFSQuad(2, 26, 24, 0, 5);
        DrawFSQuad(24, 25, 1, 0, 1);
        DrawFSQuad(3, 27, 26, 2, 4);
        DrawFSQuad(27, 3, 1, 25, 2);
        // misc edges
        DrawFSQuad(40, 7, 23, 32, 1);
        DrawFSQuad(41, 23, 21, 37, 1);
        DrawFSQuad(42, 20, 6, 43, 1);
        DrawFSQuad(43, 6, 4, 28, 2);
        // bottom bridge
        DrawSSQuad(12, 1, 13, 1, 19, 3, 18, 3);
        DrawFSQuad(10, 11, 13, 12, 0);
        DrawFSQuad(18, 19, 21, 20, 0);
    GXEnd();

    // bottom bridge, side
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);
        VertexC(37, 2);
        VertexC(21, 2);
        VertexC(36, 2);
        VertexC(19, 2);
        VertexC(35, 2);
        VertexC(13, 2);
        VertexC(34, 2);
        VertexC(11, 2);
    GXEnd();

}

/*---------------------------------------------------------------------------*
    Name:           DrawTop
    
    Description:    Draw top of box model.  The order the parts are drawn is
                    important since zbuffering is turned off.
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTop( void )
{
    // set Tev to use vertex color
    GXSetNumChans( 1 );
    GXSetNumTexGens( 0 );
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
            
    GXBegin(GX_QUADS, GX_VTXFMT0, 8);
        // top, left and right 
        DrawFSQuad(0, 4, 6, 2, 0);
        DrawFSQuad(3, 7, 5, 1, 0);
    GXEnd();

    // top bridge, side
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);
        VertexC(32, 2);
        VertexC(23, 2);
        VertexC(31, 2);
        VertexC(15, 2);
        VertexC(30, 2);
        VertexC(14, 2);
        VertexC(29, 2);
        VertexC( 9, 2);
    GXEnd();

    // top bridge
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);
        VertexC(8, 0);
        VertexC(9, 0);
        VertexC(16, 0);
        VertexC(14, 0);
        VertexC(17, 0);
        VertexC(15, 0);
        VertexC(22, 0);
        VertexC(23, 0);
    GXEnd();

    // Use fans here so no T-junctions
    // top, bottom edge
    //
#ifdef BUG_TRIANGLE_FAN
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
        VertexC( 7, 0);
        VertexC( 3, 0);
        VertexC( 2, 0);
        VertexC( 6, 0);
    GXEnd();
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
        VertexC( 7, 0);
        VertexC(20, 0);
        VertexC(21, 0);
        VertexC(22, 0);
    GXEnd();
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 3);
        VertexC( 7, 0);
        VertexC(22, 0);
        VertexC(23, 0);
    GXEnd();
#else
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 8);
        VertexC( 7, 0);
        VertexC( 3, 0);
        VertexC( 2, 0);
        VertexC( 6, 0);
        VertexC(20, 0);
        VertexC(21, 0);
        VertexC(22, 0);
        VertexC(23, 0);
    GXEnd();
#endif // BUG_TRIANGLE_FAN

    // top, top edge
#ifdef BUG_TRIANGLE_FAN
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
        VertexC(4, 0);
        VertexC(0, 0);
        VertexC(1, 0);
        VertexC(5, 0);
    GXEnd();
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
        VertexC(4, 0);
        VertexC(11, 0);
        VertexC(10, 0);
        VertexC(9, 0);
    GXEnd();
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 3);
        VertexC(4, 0);
        VertexC(10, 0);
        VertexC(8, 0);
    GXEnd();
#else
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 8);
        VertexC(4, 0);
        VertexC(0, 0);
        VertexC(1, 0);
        VertexC(5, 0);
        VertexC(11, 0);
        VertexC(10, 0);
        VertexC(9, 0);
        VertexC(8, 0);
    GXEnd();
#endif // BUG_TRIANGLE_FAN
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the entire model once. Draws the bottom box, ball,
                    and top box.  Translate ball when on slope.  This is
                    done because the collision detection thinks the slope
                    is really straight.  See toy-anim.c.
                    
    Arguments:      v        view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick(Mtx v)
{
    Mtx  m;  // Model matrix.
    Mtx  mv; // Modelview matrix.
    f32  x;  // translate ball when on slope
    Vec  curpos; // current position

    // turn off zbuffering, since viewpoint is controlled
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);

    // tilt the box around its centroid
    MTXTrans(m, BOX_CENTROIDX, 0.0F, BOX_CENTROIDX);
    MTXConcat(v, m, mv); 
    MTXRotDeg(m, 'x', GetAnalogY()*TILTX_SCALE);
    MTXConcat(mv, m, mv); 
    MTXRotDeg(m, 'z', GetAnalogX()*TILTZ_SCALE);
    MTXConcat(mv, m, mv); 
    MTXTrans(m, -BOX_CENTROIDX, 0.0F, -BOX_CENTROIDX);
    MTXConcat(mv, m, mv); 
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    DrawBottom();

    curpos = GetCurPos();
    if (OnSlope() && !DoTeleport()) {
        x = curpos.x - ((curpos.z - SLOPE_ZSTART) * SLOPE_SLOPE);
        DrawBall(x, 0.0F, curpos.z);
        DrawTop();
    } else {
        DrawTop();
        DrawBall(curpos.x, 0.0F, curpos.z);
    }
}

