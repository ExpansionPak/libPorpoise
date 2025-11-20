/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     smp-onetri_dl.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Simple/smp-onetri_dl.c $
    
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
    
    3     1/18/00 7:00p Alligator
    updated to work with new GXInit defaults
    
    2     1/13/00 8:53p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    15    11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    14    11/12/99 4:59p Yasu
    Add GXSetNumTexGens
    
    13    9/30/99 10:33p Yasu
    Renamed some functions and enums
    
    12    9/17/99 5:26p Ryan
    added new DEMO calls
    
    11    9/15/99 2:42p Ryan
    update to fix compiler warnings
    
    10    9/02/99 3:18p Ryan
    Made Frame Buffer Api changes
    
    9     7/28/99 4:07p Alligator
    update header files and emulator for hw changes
    
    8     7/23/99 2:51p Ryan
    changed dolphinDemo.h to demo.h
    
    7     7/23/99 12:15p Ryan
    included dolphinDemo.h
    
    6     7/20/99 6:03p Alligator
    demo library
    
    5     7/02/99 2:56p Alligator
    fixed matrix transpose bug
    
    4     6/15/99 11:44a Ryan
    Reversed polygon order from CCW to CW
    Added PrintIntro function
    
    2     6/08/99 3:52p Alligator
    change pragma again
    
    1     6/08/99 12:19p Alligator
    simple display list demo
    
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
   Display list pointers must be 32B aligned.  Attribute arrays ARE NOT
   required to be 32B aligned, but doing so may result in a slight 
   performance improvement.
   The macro ATTRIBUTE_ALIGN provides a convenient way to align initialized 
   arrays.
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   Connectivity display list.  Normally a display list would be created by a
   an external tool and loaded from ROM. We define this one in ASCII to make
   the format readable.
 *---------------------------------------------------------------------------*/
u8 OneTriDL[] ATTRIBUTE_ALIGN(32) = 
{
    (GX_DRAW_QUADS | GX_VTXFMT0),      // command, primitive type | vat idx
    0, 36,                             // number of verts, 16b
    8,  0,  7, 0,  2, 0,  3, 0,        // quad 0
    1,  1,  2, 1,  7, 1,  6, 1,        // quad 1
    1,  2,  0, 2,  9, 2, 10, 2,        // quad 2
    4,  1,  1, 1, 10, 1, 11, 1,        // quad 3
    1,  2, 12, 2, 13, 2,  2, 2,        // quad 4
    2,  0, 13, 0, 14, 0,  5, 0,        // quad 5
    18, 2, 15, 2, 16, 2, 17, 2,        // quad 6
    20, 1, 17, 1, 16, 1, 19, 1,        // quad 7
    20, 0, 21, 0, 18, 0, 17, 0,        // quad 8
    GX_NOP, GX_NOP, GX_NOP, GX_NOP, GX_NOP, GX_NOP, GX_NOP, // pad
    GX_NOP, GX_NOP, GX_NOP, GX_NOP, GX_NOP, GX_NOP, GX_NOP, // pad
    GX_NOP, GX_NOP, GX_NOP, GX_NOP, GX_NOP, GX_NOP, GX_NOP  // pad to 32B
};

s16 Verts_s16[] ATTRIBUTE_ALIGN(32) = 
{
//      x          y            z       
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


/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
 
void        main            ( void );
static void CameraInit      ( Mtx v );
static void DrawInit        ( void );
static void DrawTick        ( Mtx v );
static void AnimTick        ( Mtx v );
static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
 
void main ( void )
{
    Mtx         v;   // view matrix
    PADStatus   pad[PAD_MAX_CONTROLLERS]; // game pad state

    pad[0].button = 0;
        
    DEMOInit(NULL);    // init os, pad, gx, vi

    CameraInit(v); // Initialize the camera.
    DrawInit();    // Define my vertex formats and set array pointers.

    PrintIntro(); // Print demo directions
         
    while(!(pad[0].button & PAD_BUTTON_MENU)) // any button quits
    {  
		DEMOBeforeRender();
        DrawTick(v);         // Draw the model.
        DEMODoneRender(); 
        AnimTick(v);         // Update animation.
        PADRead(pad);        // read the game pad
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
    
    Description:    Initializes the vertex attribute format, and sets
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
    
    // stride = 3 elements (x,y,z) of type s16
    GXSetArray(GX_VA_POS, Verts_s16, 3*sizeof(s16));
    // stride = 4 elements (r,g,b,a) of type u8
    GXSetArray(GX_VA_CLR0, Colors_rgba8, 4*sizeof(u8));

	// Initialize lighting, texgen, and tev parameters
	GXSetNumChans(1); // default, color = vertex color
	GXSetNumTexGens(0); // no texture in this demo
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model once.  Replicates a simple strut model
                    many times in the x, y, z directions to create a dense
                    3D grid. GXInit makes GX_PNMTX0 the default matrix.
                    
    Arguments:      v        view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick(Mtx v)
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
                GXCallDisplayList(OneTriDL, 3*32);
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
