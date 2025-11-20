/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     smp-onetri.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

    $Log: /Dolphin/build/demos/videmo/src/smp-onetri_PAL.c $
    
    4     12/17/01 6:08p Hashida
    Changed to use GXGetYScaleFactor.
    
    3     01/11/13 15:41 Hashida
    Reverted the change.
    
    1     11/04/01 10:37a Hashida
    initial revision.
    
    6     10/18/01 10:31p Hashida
    Modified so that you can switch NTSC/PAL with A button.
    
    5     2/14/01 11:09a Hirose
    Changed to accept start button only to quit. Deleted obsolete ifdefs
    
    4     10/25/00 8:59p Hirose
    A flag fix MAC -> MACOS
    
    3     5/19/00 3:20a Hirose
    made objects brighter
    
    2     3/23/00 8:01p Hirose
    fixed instruction message
    
    1     3/16/00 7:24p Alligator
    rename to match other demos conventions
    
    1     3/06/00 12:11p Alligator
    move from gxdemos to gxdemos/Simple
    
    4     1/19/00 6:15p Alligator
      
    4     6/07/99 8:32p Alligator
    removed cast to void *
    
    3     6/07/99 4:35p Alligator
    fix VI function name
    
    2     6/07/99 3:44p Alligator
    added heap and framebuffer allocation
    
    1     6/04/99 11:25p Alligator
    First Immediate mode demo
    

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>

/*---------------------------------------------------------------------------*
  Model Data
 *---------------------------------------------------------------------------*/
#define STRUT_LN        130     // long side of strut
#define STRUT_SD        4       // short side of strut
#define JOINT_SD        10      // joint is a cube

/*---------------------------------------------------------------------------*
   The macro ATTRIBUTE_ALIGN provides a convenient way to align initialized 
   arrays.  Alignment of vertex arrays to 32B IS NOT required, but may result 
   in a slight performance improvement.
 *---------------------------------------------------------------------------*/
s16 Verts_s16[] ATTRIBUTE_ALIGN(32) = 
{
//      x             y             z       
    -STRUT_SD,     STRUT_SD,    -STRUT_SD,   // 0
     STRUT_SD,     STRUT_SD,    -STRUT_SD,   // 1
     STRUT_SD,     STRUT_SD,     STRUT_SD,   // 2
    -STRUT_SD,     STRUT_SD,     STRUT_SD,   // 3
     STRUT_SD,    -STRUT_SD,    -STRUT_SD,   // 4
     STRUT_SD,    -STRUT_SD,     STRUT_SD,   // 5
     STRUT_SD,     STRUT_LN,    -STRUT_SD,   // 6
     STRUT_SD,     STRUT_LN,     STRUT_SD,   // 7
    -STRUT_SD,     STRUT_LN,     STRUT_SD,   // 8
    -STRUT_SD,     STRUT_SD,    -STRUT_LN,   // 9
     STRUT_SD,     STRUT_SD,    -STRUT_LN,   // 10
     STRUT_SD,    -STRUT_SD,    -STRUT_LN,   // 11
     STRUT_LN,     STRUT_SD,    -STRUT_SD,   // 12
     STRUT_LN,     STRUT_SD,     STRUT_SD,   // 13
     STRUT_LN,    -STRUT_SD,     STRUT_SD,   // 14
    -JOINT_SD,     JOINT_SD,    -JOINT_SD,   // 15
     JOINT_SD,     JOINT_SD,    -JOINT_SD,   // 16
     JOINT_SD,     JOINT_SD,     JOINT_SD,   // 17
    -JOINT_SD,     JOINT_SD,     JOINT_SD,   // 18
     JOINT_SD,    -JOINT_SD,    -JOINT_SD,   // 19
     JOINT_SD,    -JOINT_SD,     JOINT_SD,   // 20
    -JOINT_SD,    -JOINT_SD,     JOINT_SD    // 21
};

u8 Colors_rgba8[] ATTRIBUTE_ALIGN(32) = 
{
//   r,   g,   b,   a
    42,  42,  50, 255,   // 0
    80,  80,  80, 255,   // 1
   114, 114, 110, 255    // 2
};

GXRenderModeObj Pal574IntDfScale = 
{
    VI_TVMODE_PAL_INT,      // viDisplayMode
    640,             // fbWidth
    480,             // efbHeight
    574,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 574)/2,        // viYOrigin
    640,             // viWidth
    574,             // viHeight
    VI_XFBMODE_DF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
     6,  6,  6,  6,  6,  6,  // pix 0, 3 sample points, 1/12 units, 4 bits each
     6,  6,  6,  6,  6,  6,  // pix 1
     6,  6,  6,  6,  6,  6,  // pix 2
     6,  6,  6,  6,  6,  6,  // pix 3

    // vertical filter[7], 1/64 units, 6 bits each
     8,         // line n-1
     8,         // line n-1
    10,         // line n
    12,         // line n
    10,         // line n
     8,         // line n+1
     8          // line n+1
};

GXRenderModeObj Pal480IntDfNoScale = 
{
    VI_TVMODE_PAL_INT,      // viDisplayMode
    640,             // fbWidth
    480,             // efbHeight
    480,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 480)/2,        // viYOrigin
    640,             // viWidth
    480,             // viHeight
    VI_XFBMODE_DF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
     6,  6,  6,  6,  6,  6,  // pix 0, 3 sample points, 1/12 units, 4 bits each
     6,  6,  6,  6,  6,  6,  // pix 1
     6,  6,  6,  6,  6,  6,  // pix 2
     6,  6,  6,  6,  6,  6,  // pix 3

    // vertical filter[7], 1/64 units, 6 bits each
     8,         // line n-1
     8,         // line n-1
    10,         // line n
    12,         // line n
    10,         // line n
     8,         // line n+1
     8          // line n+1
};



enum {
    MODE_PAL_NOSCALE = 0,
    MODE_PAL_SCALE,
    MODE_EURGB60,
    MODE_MAX
};

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
 
void        main            ( void );
static void CameraInit      ( Mtx v );
static void DrawInit        ( void );
static void DrawTick        ( Mtx v );
static void AnimTick        ( Mtx v );
static void PrintIntro      ( void );

GXRenderModeObj*            CurrentMode;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

static void ChangeMode(GXRenderModeObj* mode)
{
    GXSetDispCopyYScale(GXGetYScaleFactor(mode->efbHeight, mode->xfbHeight));
    VIConfigure(mode);
    VIFlush();
    VIWaitForRetrace();
    VIWaitForRetrace();
}

void main ( void )
{
    Mtx         v;   // view matrix
    PADStatus   pad[PAD_MAX_CONTROLLERS]; // game pad state
    u32         oldButtons;
    u32         changedButtons;
    u32         mode;

    pad[0].button = 0;
    oldButtons = 0;
    
    OSReport("\n");
    OSReport("======================================\n");
    OSReport("PAL version of smp-onetri\n");
    OSReport("Press A button to change mode\n");
    OSReport("======================================\n");
    OSReport("\n");

    DEMOInit(&Pal574IntDfScale);    // Init os, pad, gx, vi
    mode = MODE_PAL_SCALE;
    OSReport("PAL (Y scaling)\n");

    CameraInit(v); // Initialize the camera.  
    DrawInit();    // Define my vertex formats and set array pointers.

    PrintIntro(); // Print demo directions
         
    while(!(pad[0].button & PAD_BUTTON_MENU))
    {   
        DEMOBeforeRender();
        DrawTick(v);        // Draw the model.
        DEMODoneRender();
        AnimTick(v);        // Update animation.
        PADRead(pad);

        changedButtons = (oldButtons ^ pad[0].button) & pad[0].button;

        if (changedButtons & PAD_BUTTON_A)
        {
            mode++;
            if (mode == MODE_MAX)
            {
                mode = 0;
            }
            
            switch (mode)
            {
              case MODE_PAL_NOSCALE:
                CurrentMode = &Pal480IntDfNoScale;
                ChangeMode(CurrentMode);
                OSReport("PAL (black bars)\n");
                break;
                
              case MODE_PAL_SCALE:
                CurrentMode = &Pal574IntDfScale;
                ChangeMode(CurrentMode);
                OSReport("PAL (Y scaling)\n");
                break;
                
              case MODE_EURGB60:
                CurrentMode = &GXEurgb60Hz480IntDf;
                ChangeMode(CurrentMode);
                OSReport("EU RGB 60Hz\n");
                break;
            }
        }

        oldButtons = pad[0].button;
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
    Vec     up      = {0.20F, 0.97F, 0.0F};
    Vec     camLoc  = {90.0F, 110.0F, 13.0F};
    Vec     objPt   = {-110.0F, -70.0F, -190.0F};
    f32     left    = 24.0F;
    f32     top     = 32.0F;
    f32     znear   = 50.0F;
    f32     zfar    = 2000.0F;
    
    MTXFrustum(p, left, -left, -top, top, znear, zfar);
    GXSetProjection(p, GX_PERSPECTIVE);
    
    MTXLookAt(v, &camLoc, &up, &objPt);    
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

    GXSetCopyClear(black, 0x00FFFFFF);

    // Set current vertex descriptor to enable position and color0.
    // Both use 8b index to access their data arrays.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    
    // Position has 3 elements (x,y,z), each of type s16,
    // no fractional bits (integers)
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    
    // Color 0 has 4 components (r, g, b, a), each component is 8b.
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    
    // stride = 3 elements (x,y,z) each of type s16
    GXSetArray(GX_VA_POS, Verts_s16, 3*sizeof(s16));
    // stride = 4 elements (r,g,b,a) each of type u8
    GXSetArray(GX_VA_CLR0, Colors_rgba8, 4*sizeof(u8));

	// Initialize lighting, texgen, and tev parameters
	GXSetNumChans(1); // default, color = vertex color
	GXSetNumTexGens(0); // no texture in this demo
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

/*---------------------------------------------------------------------------*
    Name:           Vertex
    
    Description:    Create my vertex format
                    
    Arguments:      v        8-bit position index
                    c        8-bit color index
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void Vertex( u8 v, u8 c )
{
    GXPosition1x8(v);
    GXColor1x8(c);
}

/*---------------------------------------------------------------------------*
    Name:           DrawFsQuad
    
    Description:    Draw a flat-shaded quad.
                    
    Arguments:      v0    8-bit position index 0
                    v1    8-bit position index 1
                    v2    8-bit position index 2
                    v3    8-bit position index 3
                    c     8-bit color index
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void DrawFsQuad(
    u8 v0,
    u8 v1,
    u8 v2,
    u8 v3,
    u8 c ) 
{
    Vertex(v0, c);
    Vertex(v1, c);
    Vertex(v2, c);
    Vertex(v3, c);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model once.  Replicates a simple strut model
                    many times in the x, y, z directions to create a dense
                    3D grid.  GXInit makes GX_PNMTX0 the default matrix.
                    
    Arguments:      v        view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( Mtx v )
{
    f32  x;  // Translation in x.
    f32  y;  // Translation in y.
    f32  z;  // Translation in z.
    Mtx  m;  // Model matrix.
    Mtx  mv; // Modelview matrix.
    
    GXSetNumTexGens( 0 );
    GXSetNumTevStages( 1 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
    
    MTXIdentity(m);
    
    for(x = -10*STRUT_LN; x < 2*STRUT_LN; x += STRUT_LN) 
    {
        for(y = -10*STRUT_LN; y < STRUT_LN; y += STRUT_LN) 
        {
            for(z = STRUT_LN; z > -10*STRUT_LN; z -= STRUT_LN) 
            {
                MTXRowCol(m, 0, 3) = x;
                MTXRowCol(m, 1, 3) = y;
                MTXRowCol(m, 2, 3) = z;
                MTXConcat(v, m, mv); 
                GXLoadPosMtxImm(mv, GX_PNMTX0);
        
                GXBegin(GX_QUADS, GX_VTXFMT0, 36); //4 vtx/qd x 9 qd = 36 vtx
                    DrawFsQuad(8, 7, 2, 3, 0);
                    DrawFsQuad(1, 2, 7, 6, 1);
                    DrawFsQuad(1, 0, 9, 10, 2);
                    DrawFsQuad(4, 1, 10, 11, 1);
                    DrawFsQuad(1, 12, 13, 2, 2);
                    DrawFsQuad(2, 13, 14, 5, 0);
                    DrawFsQuad(18, 15, 16, 17, 2);
                    DrawFsQuad(20, 17, 16, 19, 1);
                    DrawFsQuad(20, 21, 18, 17, 0); 
                GXEnd();        
            }
        }
    }
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Moves viewpoint through the grid.  Loops animation so 
                    that it appears viewpoint is continously moving forward.
                    
    Arguments:      v        view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( Mtx v )
{
    static u32  ticks = 0; // Counter.
    Mtx         fwd;       // Forward stepping translation matrix.
    Mtx         back;      // Loop back translation matrix.
    
    u32    animSteps    = 100;
    f32    animLoopBack = (f32)STRUT_LN;
    f32    animStepFwd  = animLoopBack / animSteps;    
    
    MTXTrans(fwd, 0, 0, animStepFwd);
    MTXTrans(back, 0, 0, -animLoopBack);

    MTXConcat(v, fwd, v);
    if((ticks % animSteps) == 0)
        MTXConcat(v, back, v);

    ticks++;
}

/*---------------------------------------------------------------------------*
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n****************************************\n");
    OSReport("to quit:\n");
    OSReport("     hit the start button\n");
#ifdef MACOS
    OSReport("     click on the text output window\n");
    OSReport("     select quit from the menu or hit 'command q'\n");
    OSReport("     select 'don't save'\n");
#endif // MACOS
    OSReport("****************************************\n");
}
