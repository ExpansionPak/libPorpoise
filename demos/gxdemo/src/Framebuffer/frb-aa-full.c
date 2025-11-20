/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     frb-aa-full.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Framebuffer/frb-aa-full.c $
    
    13    7/03/01 11:06a Hirose
    added viewport setting which DEMO library doesn't set appropriately for
    full-frame AA.
    
    12    5/09/01 1:41p Carl
    Got rid of warning due to API change.
    
    11    5/05/01 10:05p Hirose
    removed obsolete flags
    
    10    11/02/00 2:22p Carl
    Added code for Rev. B flipper.
    
    9     10/25/00 9:40p Hirose
    fixed flags. MAC -> MACOS
    
    8     8/08/00 4:23p Hirose
    change due to the update of DEMOSwapBuffers
    
    7     7/21/00 1:44p Carl
    Redone due to DEMOInit.c changes.
    
    6     7/19/00 3:40p Carl
    Fixed comments.
    
    5     7/01/00 1:10p Alligator
    
    4     6/16/00 7:04p Carl
    Minor code cleanup.
    
    3     6/05/00 7:10p Carl
    Revamped.  Now changes frustum instead of viewport.
    
    2     5/24/00 3:27a Carl
    New controls, more unified code with other AA demos.
    
    1     5/18/00 11:13p Carl
    Test for 480 line AA mode.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>


#define OCT_DEPTH_MIN  1 // min octahedron recursion depth
#define OCT_DEPTH_MAX  6 // max octahedron recursion depth

// The viewing frustum parameters
#define PROJ_RIGHT   0.050F
#define PROJ_TOP     0.0375F
#define PROJ_ZNEAR   0.1F
#define PROJ_ZFAR    10.0F
    
// Octahedron vertices
static f32 vertNorth[] = { 0.0f, 0.0f, 1.0f };
static f32 vertSouth[] = { 0.0f, 0.0f, -1.0f };
static f32 vertEquator[] = 
{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    0.0f, -1.0f, 0.0f
};

// An alternate sample pattern & filter to try out
static u8  mySamples[12][2] = {
    3,  3, 10,  5,  5, 10,
    7,  2,  2,  7,  9,  9,
    7,  2,  2,  7,  9,  9,
    3,  3, 10,  5,  5, 10,
};

static u8  myFilter[7] = {
    4, 11, 11, 12, 11, 11, 4
};

static u32 Ticks    = 0;  // time counter
static u8  animMode = 1;  // whether animation is going or stopped
static u8  drawMode = 0;  // which pattern to draw
static u8  aaMode   = 0;  // which filters to use

static u32 octDepth = 3;

// The following items are necessary for full-frame AA mode

GXRenderModeObj *rMode;     // pointer to the render mode struct

#define COPY_OVERLAP 2
static u16 copyLines;       // number of scan lines to copy for bottom
static u32 bufferOffset;    // offset into external frame buffer
                            // for copying bottom half of buffer

// Buffer space for 2 lines of video
// This is needed to clear out the overlap lines in f.f. AA mode
static u8 garbage[640*2*VI_DISPLAY_PIX_SZ] ATTRIBUTE_ALIGN(32);

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
 
void        main        ( void );
static void CopyBottom  ( void );
static void CameraInit  ( Mtx v );
static void DrawInit    ( void );
static void DrawTick    ( Mtx v );
static void AnimTick    ( void );
static void PrintIntro  ( void );
static void Tri         (f32 *v0, f32 *v1, f32 *v2, u8 color, u32 depth);

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{
    Mtx         v;   // view matrix
#if (GX_REV == 1)
    Mtx44       p;   // projection matrix
    f32         yOverlap;
#else
    u32         width;
    u32         half_ht;
#endif

    rMode = &GXNtsc480IntAa;

    DEMOInit(rMode);    // Init os, pad, gx, vi
    
    // The overlap for the rendering passes is 2 scan lines.
#if (GX_REV == 1)
    // For Rev. A hardware, we set the overlap by adjusting the frustum.
    // Thus the overlap is converted into frustum space by dividing
    // it by the total number of scanlines, then multiplying by the
    // total frustum height at the near plane.

    yOverlap = ((f32) COPY_OVERLAP / rMode->xfbHeight) * (PROJ_TOP*2.0f);
#else    
    // For Rev. B hardware, we fiddle with the scissor box.
    // Store away some framebuffer values to make it easy to change scissor.
    width = rMode->fbWidth;
    half_ht = (u16) (rMode->xfbHeight / 2);
#endif

    // Calculate copy parameters for bottom half of the screen
    copyLines    = (u16) (rMode->efbHeight - COPY_OVERLAP);
    bufferOffset = VIPadFrameBufferWidth(rMode->fbWidth) * copyLines * 
                   (u32) VI_DISPLAY_PIX_SZ;

    // Y scale is miscalculated in DEMOInit.  Set it to 1.0
    GXSetDispCopyYScale((f32) 1.0); 

    // You typically want to turn on dither in 16-bit mode
    GXSetDither(GX_ENABLE);


    CameraInit(v); // Initialize the camera.  
    DrawInit();    // Define my vertex formats and set array pointers.

    PrintIntro(); // Print demo directions
         
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
        DEMOBeforeRender();
        
        // Viewport setting done by DEMOBeforeRender is not appropriate
        // for full-frame AA mode. Override it by correct setting.
        GXSetViewport(0.0F, 0.0F,
                      (f32)rMode->fbWidth, (f32)rMode->xfbHeight, 
                      0.0F, 1.0F);
        

        // Draw the top half of the screen
#if (GX_REV == 1)
        // (code for rev. A Flipper)
        // Just the regular frustum.  The scissor box is
        // set to only get the top half plus the overlap.
        MTXFrustum(p, PROJ_TOP,  -PROJ_TOP,
                     -PROJ_RIGHT, PROJ_RIGHT,
                      PROJ_ZNEAR, PROJ_ZFAR);
        GXSetProjection(p, GX_PERSPECTIVE);
#else    
        // (code for rev. B Flipper)
        // Set scissor box and offset to top half
        GXSetScissor(0, 0, width, half_ht+2);
        GXSetScissorBoxOffset(0, 0);
#endif
        DrawTick(v);        // Draw the model.

        // Copy out (and clear) the top half
        GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);

        // Draw the bottom half of the screen
#if (GX_REV == 1)
        // (code for rev. A Flipper)
        // The frustum is shifted down by the offset
        // (half the frustum height at near plane) minus the overlap.
        MTXFrustum(p, yOverlap, -2.0f*PROJ_TOP+yOverlap,
                     -PROJ_RIGHT, PROJ_RIGHT,
                      PROJ_ZNEAR, PROJ_ZFAR);
        GXSetProjection(p, GX_PERSPECTIVE);
#else
        // (code for rev. B Flipper)
        // Set scissor box and offset for bottom half
        GXSetScissor(0, half_ht-2, width, half_ht+2);
        GXSetScissorBoxOffset(0, (s32) half_ht-2);
#endif
        DrawTick(v);        // Draw the model.

        // Copy out (and clear) the bottom half
        CopyBottom();

        // Wait for retrace and swap buffers.
        GXDrawDone();
        DEMOSwapBuffers();

        DEMOPadRead();
        AnimTick();         // Update animation.
    }

    OSHalt("End of test");
}


/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------------*
    Name:           CopyBottom
    
    Description:    Copies the bottom half of the frame buffer correctly.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CopyBottom( void )
{
    // This first copy copies out the lines we want to display.
    // We want to omit the first two lines from the copy, but one of
    // these lines is used for the vertical filter.  We must adjust
    // the clamp mode to allow for this (turn off top clamping).
    GXSetCopyClamp(GX_CLAMP_BOTTOM); // not TOP
    GXSetDispCopySrc(0, COPY_OVERLAP, rMode->fbWidth, copyLines);
    GXCopyDisp((void *)((u32)DEMOGetCurrentBuffer()+bufferOffset), GX_TRUE);

    // This second copy is to clear out the two lines that we didn't copy.
    // We also must return the clamp mode back to normal.
    GXSetCopyClamp((GXFBClamp)(GX_CLAMP_TOP | GX_CLAMP_BOTTOM));
    GXSetDispCopySrc(0, 0, rMode->fbWidth, COPY_OVERLAP);
    GXCopyDisp((void *)garbage, GX_TRUE);

    // Finally, return the display copy source back to the default.
    // This makes sure the top half will get copied correctly.
    GXSetDispCopySrc(0, 0, rMode->fbWidth, rMode->efbHeight);
}

/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.
                    Initialize the view matrix.
                    
    Arguments:      v      view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit ( Mtx v )
{
    Vec     up      = {0.0F, 0.0F, 1.0F};
    Vec     camLoc  = {0.25F, 3.0F, 0.5F};
    Vec     objPt   = {0.0F, 0.0F, 0.0F};

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
    Mtx44       p;   // projection matrix
    GXColor blue = {0, 0, 255, 0};

    // The clear Z is always specified in 24-bit format
    GXSetCopyClear(blue, GX_MAX_Z24);

    // Need to clear background by specified color
    // since the clear color is different than DEMOInit default
    // The clear operation can be done by dummy display copy
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);

    // Set current vertex descriptor to enable position and color0.
    // Both use 8b index to access their data arrays.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
            
    // Position has 3 elements (x,y,z), each of type f32
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    
    // Color 0 has 4 components (r, g, b, a), each component is 8b.
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    
    // Initialize lighting, texgen, and tev parameters
    GXSetNumChans(1); // default, color = vertex color
    GXSetNumTexGens(0); // no texture in this demo
    GXSetNumTevStages( 1 );
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    // Set the projection matrix.
    // Not needed here for Rev. A, actually.
    MTXFrustum(p, PROJ_TOP,  -PROJ_TOP,
               -PROJ_RIGHT, PROJ_RIGHT,
               PROJ_ZNEAR, PROJ_ZFAR);
    GXSetProjection(p, GX_PERSPECTIVE);
}

/*---------------------------------------------------------------------------*
    Name:           Tri
    
    Description:    Recursively draw a triangle while alternateing colors.
                    
    Arguments:      v0        vertex 0
                    v1        vertex 1
                    v2        vertex 2
                    color     color of triangle
                    depth     recursion depth
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void Tri (f32 *v0, f32 *v1, f32 *v2, u8 color, u32 depth)
{
    f32 vertMid01[3];  // midpoint between v0 and v1
    f32 vertMid12[3];  // midpoint between v1 and v2
    f32 vertMid20[3];  // midpoint between v2 and v0

    // if more recursion depth to go, ...
    if (depth > 0)
    {
        // compute triangle midpoints on unit spheres
        VECAdd((VecPtr) v0, (VecPtr) v1, (VecPtr) vertMid01);
        VECScale((VecPtr) vertMid01, (VecPtr) vertMid01, 0.5f);
        VECNormalize((VecPtr) vertMid01, (VecPtr) vertMid01);

        VECAdd((VecPtr) v1, (VecPtr) v2, (VecPtr) vertMid12);
        VECScale((VecPtr) vertMid12, (VecPtr) vertMid12, 0.5f);
        VECNormalize((VecPtr) vertMid12, (VecPtr) vertMid12);
        
        VECAdd((VecPtr) v2, (VecPtr) v0, (VecPtr) vertMid20);
        VECScale((VecPtr) vertMid20, (VecPtr) vertMid20, 0.5f);
        VECNormalize((VecPtr) vertMid20, (VecPtr) vertMid20);

        // draw three outer triangles in same color as parent
        Tri(v0, vertMid01, vertMid20, color, depth - 1);
        Tri(v1, vertMid12, vertMid01, color, depth - 1);
        Tri(v2, vertMid20, vertMid12, color, depth - 1);
        
        // draw inner triangle in complementary color
        Tri(vertMid01, vertMid12, vertMid20, (u8) ~color, depth - 1);
    }
    else // at bottom of recursion
    {
        // draw the triangle
        GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3);
            // vertex 0
            GXPosition3f32( v0[0], v0[1], v0[2] );
            GXColor4u8( color, color, color, 255 );

            // vertex 1
            GXPosition3f32( v1[0], v1[1], v1[2] );
            GXColor4u8( color, color, color, 255 );

            // vertex 2
            GXPosition3f32( v2[0], v2[1], v2[2] );
            GXColor4u8( color, color, color, 255 );
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model once.
    
    Arguments:      v        view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( Mtx v )
{
    Mtx m;  // Model matrix.
    Mtx mv; // Modelview matrix.
    
    switch(aaMode) 
    {
      case 0:
        GXSetCopyFilter(GX_TRUE, rMode->sample_pattern,
                        GX_TRUE, rMode->vfilter);
        break;
      case 1:
        GXSetCopyFilter(GX_TRUE, mySamples,
                        GX_TRUE, myFilter);
        break;
      case 2:
        GXSetCopyFilter(GX_FALSE, NULL,
                        GX_TRUE, rMode->vfilter);
        break;
      case 3:
        GXSetCopyFilter(GX_FALSE, NULL,
                        GX_FALSE, NULL);
        break;
      default:
        OSHalt("bad aaMode case");
    }
    
    // model has a rotation about z axis
    MTXRotDeg(m, 'z', Ticks);
    MTXConcat(v, m, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    if (drawMode == 0) 
    {
        // draw octahedron to recursion depth of octDepth
        // draw northern hemisphere
        Tri(&vertEquator[3], &vertEquator[0], vertNorth, 0,   octDepth);
        Tri(&vertEquator[6], &vertEquator[3], vertNorth, 255, octDepth);
        Tri(&vertEquator[9], &vertEquator[6], vertNorth, 0,   octDepth);
        Tri(&vertEquator[0], &vertEquator[9], vertNorth, 255, octDepth);
    
        // draw southern hemisphere
        Tri(&vertEquator[0], &vertEquator[3], vertSouth, 255, octDepth);
        Tri(&vertEquator[3], &vertEquator[6], vertSouth, 0,   octDepth);
        Tri(&vertEquator[6], &vertEquator[9], vertSouth, 255, octDepth);
        Tri(&vertEquator[9], &vertEquator[0], vertSouth, 0,   octDepth);
    }
    if (drawMode == 1) 
    {
        int i;
        f32 x, y;
        
        GXBegin(GX_LINES, GX_VTXFMT0, 30*2);
        for(i=0; i<30; i++) 
        {
            x = cosf(i/15.0F * 3.1415926535F);
            y = sinf(i/15.0F * 3.1415926535F);

            // vertex 0
            GXPosition3f32( 0.0F, 0.0F, 0.0F );
            GXColor4u8( 255, 255, 255, 255 );

            // vertex 1
            GXPosition3f32( x, y, 0.0F );
            GXColor4u8( 255, 255, 255, 255 );
        }
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Computes next time step.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( void )
{
    u8  oldaaMode = aaMode;
    u16 down = DEMOPadGetButtonDown(0);

    if (down & PAD_BUTTON_X)
    {
        animMode = ! animMode;
    }

    if (down & PAD_BUTTON_Y)
    {
        drawMode = ! drawMode;
    }
    
    if (DEMOPadGetButton(0) & PAD_BUTTON_A) 
    {
        aaMode |= 1;
    }
    else
    {
        aaMode &= ~1;
    }
    
    if (down & PAD_BUTTON_B) 
    {
        aaMode+=2;
        if (aaMode > 3)
            aaMode = 0;
    }

    if (down & PAD_TRIGGER_R)
    {
    	octDepth++;
        if (octDepth > OCT_DEPTH_MAX)
            octDepth = OCT_DEPTH_MAX;
    }

    if (down & PAD_TRIGGER_L)
    {
    	octDepth--;
        if (octDepth < OCT_DEPTH_MIN)
            octDepth = OCT_DEPTH_MIN;
    }

    if (animMode) 
    {
        Ticks++;
    }

    if (aaMode != oldaaMode)
    {
        switch(aaMode) 
        {
          case 0:  OSReport("Regular filter\n"); break;
          case 1:  OSReport("Modified filter\n"); break;
          case 2:  OSReport("AA off, deflicker on\n"); break;
          case 3:  OSReport("AA off, deflicker off\n"); break;
          default: OSHalt("bad aaMode case\n");
        }
    }
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
    OSReport("frb-aa-full: demonstrate full-frame antialiasing\n");
    OSReport("*************************************************\n");
    OSReport("Button A - hold to show alternate AA mode\n");
    OSReport("Button B - press to toggle AA on/off\n\n");
    OSReport("Button X - press to toggle animation on/off\n");
    OSReport("Button Y - press to toggle drawing mode\n");
    OSReport("Triggers - press to change levels of recursive subdivision\n");
    OSReport("\nTo quit:\n");
    OSReport("     press the start button\n");
#ifdef MACOS
    OSReport("     click on the text output window\n");
    OSReport("     select quit from the menu or hit 'command q'\n");
    OSReport("     select 'don't save'\n");
#endif
    OSReport("\n\n*************************************************\n");
}
