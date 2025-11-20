/*---------------------------------------------------------------------------*
  Project:  How to use dual fifos to sort between opaque and transparent objects
  File:     mgt-fifo-dual.c

  Copyright 1998-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Management/mgt-fifo-dual.c $
    
    3     4/11/01 6:11p John
    Added call to GXSaveGPFifo.
    Changed code to help developers better understand fifo switching.
    Instead of not drawing into either fifo depending on fifo mode. Now,
    code always sends commands to both fifos regardless of current fifo
    mode.  So, this means if one fifo is not being rendered by GP, CPU may
    have clobbered commands at the read pointer of undrawn fifo because of
    fifo wrapping.  For more info, see AnimTick and DrawTick.
    
    2     4/05/01 12:28p John
    Cleaned up code and added comments.
    
    1     4/04/01 11:41a John
    Initial checkin.  Functional demo.
    
  $NoKeywords: $

 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This demo is a more difficult method to implement dual fifos. 
  Normally, dual fifos are necessary to implement frame-staggered
  rendering, where the CPU is writing commands to one fifo, while the GP is
  reading commands from another fifo from the previous frame.  An example of
  this is provided in mgt-fifo-brkpt using ONE single fifo with a breakpoint.

  However, this example of dual fifos is used not to show how to do 
  frame-staggered rendering, but rather to have a basic way of sorting opaque and
  transparent geometry into two separate fifos.  Because GameCube is a 
  Z-buffer machine, in order to draw transparent objects correctly,
  non-transparent objects need to be rendered first, and then the transparent
  objects must be rendered according to its depth (from back to front).  While this demo does
  not depth-sort its transparent geometry objects, nor does it work per polygon,
  it will show how to use two separate fifos for this simple purpose.

 *---------------------------------------------------------------------------*/

// Local includes
#include <demo.h>
#include <string.h>
#include <stdarg.h>

// Local constants
#define FIFO_SIZE       (256*1024)
#define NUM_TORUS       11

// Local structures
typedef enum
{
    FIFO_0_1 = 0,
    FIFO_1_0,
    FIFO_0,
    FIFO_1,
    FIFO_NONE               // Not a valid option for this demo

} FifoModeEnum;

typedef struct
{
    GXColor color;          // Used to test for transparency as well

    Vec     translation;
    Vec     rotation;

} GeomObj;                              

// Forward references
static void FifoInit        ( );
static void TorusInit       ( );
static void CameraInit      ( Mtx v );
static void DrawInit        ( );
static void FifoCPUSwitch   ( u32 fifoID );
static void FifoGPSwitch    ( u32 fifoID );
static void AnimTick        ( );
static void DrawTick        ( Mtx v );
static void DrawUI          ( );
static void DoneRender      ( );

// Local variables
static GXRenderModeObj *RenderMode = &GXNtsc480Int;

static GXFifoObj       *Fifo[2]    = {NULL, NULL};  // Fifo0 is for opaque objects, Fifo1 for transparent
static s32              FifoCPUID  = -1;            // Current CPU Fifo ID
static s32              FifoGPID   = -1;            // Current GP Fifo ID
static FifoModeEnum     FifoMode   = FIFO_NONE;     // Current Fifo Mode

static f32              StartRotX  = 0.0f;          // starting rotation angle on x-axis for torus
static GXCullMode       CullMode   = GX_CULL_BACK;
static GeomObj          Torus[NUM_TORUS];

static GXColor Colors[] ATTRIBUTE_ALIGN(32) = 
{         
    {   0,   0,   0, 255 }, // 0 black
    {   0,   0, 255, 255 }, // 1 blue
    { 255, 128,   0, 128 }, // 2 orange
};                          


/*---------------------------------------------------------------------------*
    Name:           main
    
    Description:    The main application loop
                    
    Arguments:      none
    
    Returns:        none
*----------------------------------------------------------------------------*/
void 
main ( void )
{   
    Mtx v;  // view matrix
    
    // Init os, pad, gx, vi
    DEMOInit( RenderMode );

    // Create the torus objects
    TorusInit();

    GXSetDispCopyGamma( GX_GM_1_0 );
    GXSetCopyClear( Colors[0], GX_MAX_Z24 );

    // Must be last function before main loop
    FifoInit();

    while( !(DEMOPadGetButton(0) & PAD_BUTTON_MENU) )
    {
        // AnimTick should not send any GX commands
        AnimTick();

        // CameraInit and DrawInit commands should be processed by GP first,
        // so pick the correct fifo to be drawn first depending on fifo modes
        if( FifoMode == FIFO_0 || FifoMode == FIFO_0_1 )
        {
            FifoCPUSwitch( 0 );

            // GP should be idle, so allow GP to process early as possible
            FifoGPSwitch( 0 );
        }
        else
        {
            FifoCPUSwitch( 1 );

            // GP should be idle, so allow GP to process early as possible
            FifoGPSwitch( 1 );
        }

        // Initialize camera and GX state again (DrawUI changes GX state for fonts)
        CameraInit( v );
        DrawInit();

        DEMOBeforeRender();

        // Draw toruses.  Switch fifos appropriately in this functions
        DrawTick( v );

        // Should draw text last, so pick the correct fifo depending on fifo mode
        if( FifoMode == FIFO_0 || FifoMode == FIFO_1_0 )
            FifoCPUSwitch( 0 );
        else
            FifoCPUSwitch( 1 );

        // Draw text onto the screen
        DrawUI();

        // Finish drawing fifos, and copy EFB to XFB.
        DoneRender();
    }

    OSHalt("End of demo");
}


/*---------------------------------------------------------------------------*
    Name:           FifoInit
    
    Description:    Initialize the two new fifos (reuse the default fifo).
                    This function should be called just before the main loop
                    since the top of the main loop will set the CPU and GP
                    fifos.
                    
    Arguments:      none
    
    Returns:        none
*----------------------------------------------------------------------------*/
static void 
FifoInit( )
{
    GXDrawDone();

    // Set Fifo0 to be the default fifo
    Fifo[0] = GXGetGPFifo();

    // Set Fifo1 to a newly allocated fifo
    Fifo[1] = OSAlloc( sizeof(GXFifoObj) );
    GXInitFifoBase( Fifo[1], OSAlloc(FIFO_SIZE), FIFO_SIZE );

    // Default mode is Fifo0 then Fifo1 (draw opaque objects first, then transparent)
    FifoMode = FIFO_0_1;

    // CPU and GP fifos will be updated at the top of the main loop
}


/*---------------------------------------------------------------------------*
    Name:           TorusInit
    
    Description:    Initialize the torus objects.
                    
    Arguments:      none
    
    Returns:        none
*----------------------------------------------------------------------------*/
static void
TorusInit()
{
    u32 i;

    memset( Torus, 0, sizeof(GeomObj) * NUM_TORUS );
    for( i = 0; i < NUM_TORUS; i++ )
    {
        if( i % 2 )
            Torus[i].color = Colors[2];
        else
            Torus[i].color = Colors[1];

        Torus[i].translation.x = -5.0f + i;
        Torus[i].rotation.x = i * 36.0f;
    }
}


/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.
                    Initialize the view matrix.
                    
    Arguments:      v - view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void 
CameraInit( Mtx v )
{
    Vec   camPt = {0.0F, 0.0F, 12.0F};
    Vec   at    = {0.0F, 0.0F, 0.0F};
    Vec   up    = {0.0F, 1.0F, 0.0F};
    Mtx44 p;
    
    // Load the projection matrix
    MTXPerspective( p, 45.0f, 4.0f / 3.0f, 1.0f, 1000.0f );
    GXSetProjection( p, GX_PERSPECTIVE );

    // Create the camera (view) matrix
    MTXLookAt( v, &camPt, &up, &at );
}


/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initialize GX state for torus objects
                    
    Arguments:      none
    
    Returns:        none
*----------------------------------------------------------------------------*/
static void
DrawInit()
{
    // Set the blend mode for transparency
    GXSetBlendMode( GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR );
    GXSetAlphaCompare( GX_GREATER, 0, GX_AOP_AND, GX_GREATER, 0 );
    GXSetZCompLoc( GX_FALSE );

    // Rasterize 1 material color (no light and texture)
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
    GXSetChanCtrl( GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE );
    GXSetNumChans( 1 );
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    GXSetNumTexGens( 0 );
    
    // Clear the VCD (GxDrawTorus will set it manually)
    GXClearVtxDesc();

    // Set the cull mode (controlled by user)
    GXSetCullMode( CullMode );
}


/*---------------------------------------------------------------------------*
    Name:           FifoCPUSwitch
    
    Description:    Switch the CPU fifo if different than current CPU fifo.
                    
    Arguments:      fifoID - 0 for opaque fifo
                             1 for transparent fifo
   
    Returns:        none
*----------------------------------------------------------------------------*/
static void
FifoCPUSwitch( u32 fifoID )
{
    if( FifoCPUID != fifoID )
    {
        GXSaveCPUFifo( GXGetCPUFifo() );
        GXSetCPUFifo( Fifo[fifoID] );
        FifoCPUID = (s32)fifoID;
    }
}


/*---------------------------------------------------------------------------*
    Name:           FifoGPSwitch
    
    Description:    Swith the GP fifo if different than current GP fifo.
                    Must make sure the GP is idle (should check with GXDrawDone,
                    and not draw sync tokens) before switching GP.
                    
    Arguments:      fifoID - 0 for opaque fifo
                             1 for transparent fifo
    
    Returns:        none
*----------------------------------------------------------------------------*/
static void
FifoGPSwitch( u32 fifoID )
{
    // GP should not be processing anything for safety

    if( FifoGPID != fifoID )
    {
        GXSaveGPFifo( GXGetGPFifo() );
        GXSetGPFifo( Fifo[fifoID] );
        FifoGPID = (s32)fifoID;
    }
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Animate the scene depending on user input.
                    
    Arguments:      none
    
    Returns:        none
*----------------------------------------------------------------------------*/
static void
AnimTick()
{
    void *readPtr;
    void *writePtr;

    DEMOPadRead();

    if( DEMOPadGetButtonDown(0) & PAD_BUTTON_A )
    {
        // If current fifo mode is to draw only one fifo only, then we need to 
        // reset the pointers for other fifo since this demo always draws into 
        // both fifos in DrawTick regardless of fifo mode.  So, if we are 
        // switching to another fifo mode, the other (undrawn) fifo will
        // have an unknown commands to at the readPtr because CPU will have 
        // clobbered them.  This is not necessary if your game code does not
        // switch fifo modes like this demo.
        if( FifoMode == FIFO_0 )
        {
            // Set the read and write pointers to the same location in the fifo
            // since AnimTick is at the top of the loop.
            GXGetFifoPtrs( Fifo[1], &readPtr, &writePtr );
            GXInitFifoPtrs( Fifo[1], readPtr, readPtr );
        }
        if( FifoMode == FIFO_1 )
        {
            // Set the read and write pointers to the same location in the fifo
            // since AnimTick is at the top of the loop.
            GXGetFifoPtrs( Fifo[0], &readPtr, &writePtr );
            GXInitFifoPtrs( Fifo[0], readPtr, readPtr );
        }

        FifoMode = (FifoModeEnum)((FifoMode + 1) % 4);
    }

    if( DEMOPadGetButtonDown(0) & PAD_BUTTON_B )
    {
        CullMode = (GXCullMode)((CullMode + 1) % 3);
    }

    if( DEMOPadGetStickY(0) )
    {
        StartRotX += DEMOPadGetStickY(0) / 12.0f;
    }
}


/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the toruses to the appropriate fifos.
                    
    Arguments:      none
    
    Returns:        none
*----------------------------------------------------------------------------*/
static void
DrawTick( Mtx v )
{
    Mtx         t;       // translation matrix
    Mtx         rx;      // rotation matrix
    u32         i;

    for( i = 0; i < NUM_TORUS; i++ )
    {
        if( Torus[i].color.a == 255 )
        {
            // Object is opaque but current fifo is transparent fifo, 
            // so switch to opaque fifo.
            FifoCPUSwitch( 0 );
        }
        else if( Torus[i].color.a != 255 )
        {
            // Object is transparent but current fifo is opaque fifo, 
            // so switch to transparent fifo.
            FifoCPUSwitch( 1 );
        }

        // Change the material color of the torus
        GXSetChanMatColor( GX_COLOR0A0, Torus[i].color );

        // Load in the modelview matrix
        MTXRotDeg( rx, 'X', Torus[i].rotation.x + StartRotX );
        MTXTrans( t, Torus[i].translation.x, Torus[i].translation.y, Torus[i].translation.z );
        MTXConcat( t, rx, rx );
        MTXConcat( v, rx, t );
        GXLoadPosMtxImm( t, GX_PNMTX0 );

        // Draw torus
        GXDrawTorus( 0.25f, 10, 17 );
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawUI
    
    Description:    Draw text into the viewport to display current state.
                    
    Arguments:      none
    
    Returns:        none
*----------------------------------------------------------------------------*/
static void
DrawUI()
{
    s16   textX = 20;
    s16   textY = 430;
    char *text;
    
    // Initialize GX for drawing text
    DEMOInitCaption( DM_FT_OPQ, RenderMode->fbWidth, RenderMode->xfbHeight );

    // So text is not culled out
    GXSetCullMode( GX_CULL_NONE );

    // Print current cull mode
    switch( CullMode )
    {
        case GX_CULL_NONE:
            text = "GX_CULL_NONE";
            break;
        case GX_CULL_FRONT:
            text = "GX_CULL_FRONT";
            break;
        case GX_CULL_BACK:
            text = "GX_CULL_BACK";
            break;
        case GX_CULL_ALL:
            text = "GX_CULL_ALL";
            break;
    }
    DEMOPrintf( textX, textY, 0, "B button: Cull Mode: %s", text );

    // Print current fifo mode
    textY += 9;
    switch( FifoMode )
    {
        case FIFO_0_1:
            text = "Fifo0 (opaque) then Fifo1 (transparent)";
            break;
        case FIFO_1_0:
            text = "Fifo1 (transparent) then Fifo0 (opaque)";
            break;
        case FIFO_0:
            text = "Fifo0 only";
            break;
        case FIFO_1:
            text = "Fifo1 only";
            break;
    }
    DEMOPrintf( textX, textY, 0, "A button: Fifo Mode: %s", text );
}


/*---------------------------------------------------------------------------*
    Name:           DoneRender
    
    Description:    Finishes rendering fifos.  Copies copies embedded frame buffer (EFB)
                    to external frame buffer (XFB).
    
    Arguments:      None
    
    Returns:        None
 *---------------------------------------------------------------------------*/
static void 
DoneRender( )
{
    extern void *DemoCurrentBuffer;

    // Depending on current fifo mode, draw the fifos in proper order

    // Draw Fifo0
    if( FifoMode == FIFO_0_1 || FifoMode == FIFO_0 )
    {
        // Set the CPU and GP first to opaque fifo (immediate-mode)
        FifoCPUSwitch( 0 );
        FifoGPSwitch( 0 );

        // If we will be drawing fifo1, then make sure GP is done before switching fifo
        if( FifoMode == FIFO_0_1 )
            GXDrawDone();
    }

    // Draw Fifo1
    if( FifoMode == FIFO_0_1 || FifoMode == FIFO_1 || FifoMode == FIFO_1_0 )
    {
        // Set the CPU and GP to transparent fifo (immediate-mode)
        FifoCPUSwitch( 1 );
        FifoGPSwitch( 1 );

        // If we will be drawing fifo0, then make sure GP is done before switching fifo
        if( FifoMode == FIFO_1_0 )
            GXDrawDone();
    }

    // Draw Fifo0
    if( FifoMode == FIFO_1_0 )
    {
        // Set the CPU and GP to opaque fifo (immediate-mode)
        FifoCPUSwitch( 0 );
        FifoGPSwitch( 0 );
    }

    // Set Z/Color update to make sure eFB will be cleared at GXCopyDisp.
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetColorUpdate(GX_TRUE);
    
    // Issue display copy command
    GXCopyDisp(DemoCurrentBuffer, GX_TRUE);

    // Wait until everything is drawn and copied into XFB.
    GXDrawDone();

    // Set the next frame buffer
    DEMOSwapBuffers();
}


