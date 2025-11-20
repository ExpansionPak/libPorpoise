/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     frb-fld-int.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Framebuffer/frb-fld-int.c $
    
    9     5/05/01 9:24p Hirose
    removed obsoleted flags
    
    8     10/30/00 11:53a Hirose
    removed obsolete CSIM_OUTPUT stuff
    
    7     10/25/00 9:40p Hirose
    fixed flags. MAC -> MACOS
    
    6     6/16/00 7:05p Carl
    Minor code cleanup.
    Also changed field-specific box drawing a bit.
    
    5     5/24/00 3:13a Carl
    Removed OddField references; use DemoDrawField instead.
    
    4     5/19/00 12:17a Hirose
    added pause animation button
    
    3     4/04/00 2:08p Carl
    Added field-specific rectangles and diagonal lines to help
    diagnose interlaced output.
    
    2     3/23/00 2:14p Hirose
    fixed "hit the start button" -> "hit any button"
    
    1     3/06/00 12:09p Alligator
    move from tests/gx and rename
    
    4     2/23/00 7:05p Carl
    Added code for handling odd/even field flags.
    
    3     1/25/00 2:48p Carl
    Changed to standardized end of test message
    
    2     1/20/00 5:56p Danm
    Single frame testing for single field rendering.
    
    1     1/20/00 3:01p Danm
    Created tests for signle field rendering. frb-test03 performs
    interleaved single field rendering and frb-test04 performs double
    strike single field rendering.
    
    1     1/19/00 6:21p Danm
    Create a demo of a rotating octahedron in garish colors.
      


  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>

/*---------------------------------------------------------------------------*
   The macro ATTRIBUTE_ALIGN provides a convenient way to align initialized 
   arrays.  Alignment of vertex arrays to 32B IS NOT required, but may result 
   in a slight performance improvement.
 *---------------------------------------------------------------------------*/
f32 Verts_f32[] ATTRIBUTE_ALIGN(32) = 
{
//      x, y, z       
    0.0f, 0.0f, 1.0f,   // 0:0
    0.0f, 1.0f, 0.0f,   // 0:1
    1.0f, 0.0f, 0.0f,   // 0:2
    0.0f, 0.0f, 1.0f,   // 1:0
    -1.0f, 0.0f, 0.0f,  // 1:1
    0.0f, 1.0f, 0.0f,   // 1:2
    0.0f, 0.0f, 1.0f,   // 2:0
    0.0f, -1.0f, 0.0f,  // 2:1
    -1.0f, 0.0f, 0.0f,  // 2:2
    0.0f, 0.0f, 1.0f,   // 3:0
    1.0f, 0.0f, 0.0f,   // 3:1
    0.0f, -1.0f, 0.0f,  // 3:2
    0.0f, 0.0f, -1.0f,  // 4:0
    1.0f, 0.0f, 0.0f,   // 4:1
    0.0f, 1.0f, 0.0f,   // 4:2
    0.0f, 0.0f, -1.0f,  // 5:0
    0.0f, 1.0f, 0.0f,   // 5:1
    -1.0f, 0.0f, 0.0f,  // 5:2
    0.0f, 0.0f, -1.0f,  // 6:0
    -1.0f, 0.0f, 0.0f,  // 6:1
    0.0f, -1.0f, 0.0f,  // 6:2
    0.0f, 0.0f, -1.0f,  // 7:0
    0.0f, -1.0f, 0.0f,  // 7:1
    1.0f, 0.0f, 0.0f    // 7:2
};

u8 Colors_rgba8[] ATTRIBUTE_ALIGN(32) = 
{
    //  r,   g,  b,  a
    255, 0, 0, 255, // 0:0
    255, 0, 0, 255, // 0:1
    255, 0, 0, 255, // 0:2
    0, 255, 0, 255, // 1:0
    0, 255, 0, 255, // 1:1
    0, 255, 0, 255, // 1:2
    255, 0, 0, 255, // 2:0
    255, 0, 0, 255, // 2:1
    255, 0, 0, 255, // 2:2
    0, 255, 0, 255, // 3:0
    0, 255, 0, 255, // 3:1
    0, 255, 0, 255, // 3:2
    0, 255, 0, 255, // 4:0
    0, 255, 0, 255, // 4:1
    0, 255, 0, 255, // 4:2
    255, 0, 0, 255, // 5:0
    255, 0, 0, 255, // 5:1
    255, 0, 0, 255, // 5:2
    0, 255, 0, 255, // 6:0
    0, 255, 0, 255, // 6:1
    0, 255, 0, 255, // 6:2
    255, 0, 0, 255, // 7:0
    255, 0, 0, 255, // 7:1
    255, 0, 0, 255, // 7:2
    0, 255, 0, 255,    // 24 extra: green
    255, 255, 0, 255,  // 25 extra: yellow
    0, 255, 255, 255,  // 26 extra: cyan
    255, 0, 255, 255,  // 27 extra: magenta
    255, 255, 255, 255 // 28 extra: white
};

#define GREEN   24
#define YELLOW  25
#define CYAN    26
#define MAGENTA 27
#define WHITE   28

typedef struct
{
    u32 ticks;      // time counter
    Mtx v;          // view matrix
    u16 width;      // screen width
    u16 height;     // screen height
} MyState;

MyState mystate;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
 
void        main            ( void );
static void CameraInit      ( MyState *ms );
static void DrawInit        ( void );
static void DrawTick        ( MyState *ms );
static void AnimTick        ( MyState *ms );
static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{
    GXRenderModeObj *rmode;
    
    DEMOInit(&GXNtsc240Int);    // Init os, pad, gx, vi
    rmode = DEMOGetRenderModeObj();
    
    mystate.ticks = 0;
    mystate.width = rmode->fbWidth;
    mystate.height = rmode->viHeight;  // Note: VI Height, not fb height!!!

    CameraInit(&mystate); // Initialize the camera.
    DrawInit();   // Define my vertex formats and set array pointers.

    PrintIntro(); // Print demo directions
         
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
        DEMOBeforeRender();
        DrawTick(&mystate);     // Draw the model.
        DEMODoneRender();
        DEMOPadRead();
        if (!(DEMOPadGetButton(0) & PAD_BUTTON_A))
        {
            AnimTick(&mystate); // Update animation.
        }
    }

    OSHalt("End of test");
}


/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.
                    Initialize the view matrix.
                    
    Arguments:      ms     pointer to general scene state
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit ( MyState *ms )
{
    Mtx44   p;      // projection matrix
    Vec     up      = {0.0F, 1.0F, 0.0F};
    Vec     camLoc  = {0.0F, 0.0F, 500.0F};
    Vec     objPt   = {0.0F, 0.0F, 0.0F};
    f32     left;
    f32     top;    
    f32     znear   = 100.0F;
    f32     zmid;
    f32     zfar    = 1000.0F;
    
    zmid = camLoc.z;
    left = znear * (ms->width / 2.0F) / zmid;
    top  = znear * (ms->height / 2.0F) / zmid;

    // Assuming camera at zmid, the projection maps:
    //     (-width/2, -height/2, 0) to (-1, -1, 0)
    //     ( width/2,  height/2, 0) to ( 1,  1, 0)

    MTXFrustum(p, top, -top, -left, left, znear, zfar);
    GXSetProjection(p, GX_PERSPECTIVE);
    
    MTXLookAt(ms->v, &camLoc, &up, &objPt);    
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format 0, and sets
                    the array pointers and strides for the indexed data.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{ 
    GXColor black = {0, 0, 0, 0};

    GXSetCopyClear(black, 0x00ffffff);

    // Position has 3 elements (x,y,z), each of type f32
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    
    // Color 0 has 4 components (r, g, b, a), each component is 8b.
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    
    // stride = 3 elements (x,y,z) each of type s16
    GXSetArray(GX_VA_POS, Verts_f32, 3*sizeof(f32));
    // stride = 4 elements (r,g,b,a) each of type u8
    GXSetArray(GX_VA_CLR0, Colors_rgba8, 4*sizeof(u8));

    // Initialize lighting, texgen, and tev parameters
    GXSetNumChans(1); // default, color = vertex color
    GXSetNumTexGens(0); // no texture in this demo
    GXSetNumTevStages( 1 );
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

/*---------------------------------------------------------------------------*
    Name:           Vertex
    
    Description:    Create my vertex format
                    
    Arguments:      t        8-bit triangle index
                    v        8-bit vertex index
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void Vertex( u8 t, u8 v )
{
    u8 tv = (u8) (3 * t + v);
    GXPosition1x8(tv);
    GXColor1x8(tv);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model once.
    
    Arguments:      ms     pointer to general scene state
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MyState *ms )
{
    Mtx m;  // Model matrix.
    Mtx mv; // Modelview matrix.
    u8  iTri;   // index of triangle
    u8  iVert;  // index of vertex
    
    // Set current vertex descriptor to enable position and color0.
    // Both use 8b index to access their data arrays.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
            
    // model has a rotation about x axis...
    MTXRotDeg(m, 'x', 20.0F);
    MTXConcat(ms->v, m, mv);

    // and an animated rotation about y axis
    MTXRotDeg(m, 'y', 8 * ms->ticks + 102.5F);
    MTXConcat(mv, m, mv);

    // model needs to be scaled to fill screen better
    MTXScale(m, 200.0F, 200.0F, 200.0F);
    MTXConcat(mv, m, mv);

    GXLoadPosMtxImm(mv, GX_PNMTX0);

    GXBegin(GX_TRIANGLES, GX_VTXFMT0, 24);
    
    // for all triangles of octahedron, ...
    for (iTri = 0; iTri < 8; ++iTri)
    {
        // for all vertices of triangle, ...
        for (iVert = 0; iVert < 3; ++iVert)
        {
            Vertex(iTri, iVert);
        }
    }

    GXEnd();

    // Set current vertex descriptor to enable position and color0.
    // Both use direct data.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
            
    // Translate, scale to map to screen space
    MTXScale(m, 1.0F, -1.0F, 1.0F);
    MTXConcat(ms->v, m, mv);
    MTXTrans(m, -ms->width/2.0F, -ms->height/2.0F, 0.0F);
    MTXConcat(mv, m, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    // Field-specific rectangles:
    // VI_FIELD_BELOW draws cyan rectangles
    // VI_FIELD_ABOVE draws yellow rectangles

    if (VIGetNextField() == VI_FIELD_BELOW)
    {
        GXBegin(GX_QUADS, GX_VTXFMT0, 8);
        GXPosition3f32(50.0F, 50.0F, 0.0F);
        GXColor1x8(CYAN);
        GXPosition3f32(75.0F, 50.0F, 0.0F);
        GXColor1x8(CYAN);
        GXPosition3f32(75.0F, 75.0F, 0.0F);
        GXColor1x8(CYAN);
        GXPosition3f32(50.0F, 75.0F, 0.0F);
        GXColor1x8(CYAN);

        GXPosition3f32(175.0F, 50.0F, 0.0F);
        GXColor1x8(CYAN);
        GXPosition3f32(200.0F, 50.0F, 0.0F);
        GXColor1x8(CYAN);
        GXPosition3f32(200.0F, 75.0F, 0.0F);
        GXColor1x8(CYAN);
        GXPosition3f32(175.0F, 75.0F, 0.0F);
        GXColor1x8(CYAN);
        GXEnd();
    }
    else
    {
        GXBegin(GX_QUADS, GX_VTXFMT0, 8);
        GXPosition3f32(150.0F, 50.0F, 0.0F);
        GXColor1x8(YELLOW);
        GXPosition3f32(175.0F, 50.0F, 0.0F);
        GXColor1x8(YELLOW);
        GXPosition3f32(175.0F, 75.0F, 0.0F);
        GXColor1x8(YELLOW);
        GXPosition3f32(150.0F, 75.0F, 0.0F);
        GXColor1x8(YELLOW);

        GXPosition3f32(75.0F, 50.0F, 0.0F);
        GXColor1x8(YELLOW);
        GXPosition3f32(100.0F, 50.0F, 0.0F);
        GXColor1x8(YELLOW);
        GXPosition3f32(100.0F, 75.0F, 0.0F);
        GXColor1x8(YELLOW);
        GXPosition3f32(75.0F, 75.0F, 0.0F);
        GXColor1x8(YELLOW);
        GXEnd();
    }
    
    // Diagonal lines

    GXBegin(GX_LINES, GX_VTXFMT0, 12);

    // Off by one lines
    GXPosition3f32(100.0F, 44.0F, 0.0F);
    GXColor1x8(MAGENTA);
    GXPosition3f32(150.0F, 45.0F, 0.0F);
    GXColor1x8(MAGENTA);

    GXPosition3f32(100.0F, 80.0F, 0.0F);
    GXColor1x8(MAGENTA);
    GXPosition3f32(150.0F, 81.0F, 0.0F);
    GXColor1x8(MAGENTA);

    // Off by two lines
    GXPosition3f32(100.0F, 38.0F, 0.0F);
    GXColor1x8(GREEN);
    GXPosition3f32(150.0F, 40.0F, 0.0F);
    GXColor1x8(GREEN);

    GXPosition3f32(100.0F, 85.0F, 0.0F);
    GXColor1x8(GREEN);
    GXPosition3f32(150.0F, 87.0F, 0.0F);
    GXColor1x8(GREEN);

    // Crossing lines
    GXPosition3f32(100.0F, 50.0F, 0.0F);
    GXColor1x8(WHITE);
    GXPosition3f32(150.0F, 75.0F, 0.0F);
    GXColor1x8(WHITE);

    GXPosition3f32(150.0F, 50.0F, 0.0F);
    GXColor1x8(WHITE);
    GXPosition3f32(100.0F, 75.0F, 0.0F);
    GXColor1x8(WHITE);

    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Computes next time step.
                    
    Arguments:      ms     pointer to general scene state
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MyState *ms )
{
    ms->ticks++;
}

/*---------------------------------------------------------------------------*
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n*************************************************\n");
    OSReport("A Button : pause animation while pressed down\n");
    OSReport("To quit:\n");
    OSReport("     hit the start/pause button\n");
#ifdef MACOS
    OSReport("     click on the text output window\n");
    OSReport("     select quit from the menu or hit 'command q'\n");
    OSReport("     select 'don't save'\n");
#endif
    OSReport("***************************************************\n");
}
