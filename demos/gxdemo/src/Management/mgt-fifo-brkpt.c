/*---------------------------------------------------------------------------*
  Project:  How to use dual-fifos without really trying
  File:     mgt-fifo-brkpt.c

  Copyright 1998-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Management/mgt-fifo-brkpt.c $
    
    4     4/11/01 4:53p John
    Renamed and moved demo:
    Simple/smp-dualfifo --> Management/mgt-fifo-brkpt (EPPC only)
    
    3     2/14/01 11:09a Hirose
    Changed to accept start button only to quit. Deleted obsolete ifdefs
    
    2     10/16/00 4:18p Tian
    Added an actual call to FifoInit and fixed it to NOT use a local
    variable for the new fifo.
    
    1     10/06/00 12:11p Tian
    Initial checkin

  $NoKeywords: $

 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This demo is based on the smp-texexample demo, except we animate to make 
  it clear that this code actually works.

  We use ONE fifo to hold TWO frames' data.  One of the frames is
  being consumed by the GP.  The other is being produced by the CPU.

  We prevent the GP from starting on the second frame prematurely by
  placing a FIFO breakpoint right where the CPU is about to start
  writing the next frame (see FifoTick).  

  We prevent the CPU from starting on a third frame by synchronizing
  with the GP.  Since we cannot rely on GXDrawDone, we poll with
  GXReadDrawSync (see MyDEMODoneRender).

  There is no need to worry about the CPU trampling over the data the
  GP hasn't read yet - the high watermark interrupt will get triggered
  before that.

  Look at FifoInit to see how we re-allocate the FIFO object, because
  in a real application, the default fifo might not be large enough to
  hold 2 frames worth of command data.
 *---------------------------------------------------------------------------*/

#include <demo.h>

#define BALL64_TEX_ID        8

#define BIG_FIFO_SIZE        (256*1024)
#define FIFO_DRAWDONE_TOKEN  0xBEEF
#define FIFO_DRAWING_TOKEN   0xB00B

static void FifoInit            ( void );
static void FifoTick            ( void );
static void MyDEMODoneRender    ( void );


/*---------------------------------------------------------------------------*
  Model Data
 *---------------------------------------------------------------------------*/

static s8 Vert_s8[] ATTRIBUTE_ALIGN(32) = 
{        
    -100,  100, 0,  // 0
     100,  100, 0,  // 1
    -100, -100, 0   // 2
};        
                                  
static u32 Colors_u32[] ATTRIBUTE_ALIGN(32) = 
{         
//    r g b a
    0xff0000ff, // 0
    0x00ff00ff, // 1
    0x0000ffff  // 2
};                          

//  Array of texture coordinates
static u8 TexCoords_u8[] ATTRIBUTE_ALIGN(32) = 
{        
    0x00, 0x00, // 0
//    s     t        fixed point format is unsigned 8.0
    0x01, 0x00, // 1
    0x00, 0x01  // 2
};

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
                  
static void CameraInit( Mtx v );

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{   
    PADStatus     pad[4];  // game pad state
    GXTexObj      texObj;  // texture object
    Mtx           v;       // view matrix
    Mtx           r;       // rotation matrix
    u8            i;       // loop variable
    u32           deg;     // rotation angle
    TEXPalettePtr tpl = 0; // texture palette 
    
    pad[0].button = 0;

    DEMOInit(NULL);    // Init os, pad, gx, vi
    FifoInit();

    CameraInit(v);

    GXSetNumChans(1);  // Enable light channel; by default = vertex color

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS,  GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    // Add an indexed texture coordinate to the vertex description
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);

    GXSetArray(GX_VA_POS,  Vert_s8, 3*sizeof(s8));
    GXSetArray(GX_VA_CLR0, Colors_u32, 1*sizeof(u32));
    GXSetArray(GX_VA_TEX0, TexCoords_u8, 2*sizeof(u8));

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_S8,    0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    //  Describe the texture coordinate format
    //  fixed point format is unsigned 8.0
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,   GX_U8,    0);
    
    //  Load the texture palette
    TEXGetPalette(&tpl, "gxTextrs.tpl");
    //  Initialize a texture object to contain the correct texture
    TEXGetGXTexObjFromPalette(tpl, &texObj, BALL64_TEX_ID);
    //  Load the texture object; tex0 is used in stage 0
    GXLoadTexObj(&texObj, GX_TEXMAP0);

    //  Set the Texture Environment (Tev) Mode for stage 0
    //  GXInit sets default of 1 TexCoordGen
    //  Default TexCoordGen is texcoord(n) from tex(n) with 2x4 identity mtx
    //  Default number of tev stages is 1
    //  Default stage0 uses texcoord0, texmap0, color0a0
    //  Only need to change the tevop
    GXSetTevOp(GX_TEVSTAGE0, GX_DECAL);

    OSReport("\n\n********************************\n");
    OSReport("to quit:\n");
    OSReport("     hit the start button\n");
#ifdef MACOS
    OSReport("     click on the text output window\n");
    OSReport("     select quit from the menu or hit 'command q'\n");
    OSReport("     select 'don't save'\n");
#endif
    OSReport("********************************\n");

    deg = 0;
         
    while(!(pad[0].button & PAD_BUTTON_MENU))
    {
        FifoTick();
        
        DEMOBeforeRender();
        
        MTXRotDeg( r, 'Y', deg++);
        MTXConcat(v, r, r);
        GXLoadPosMtxImm(r, GX_PNMTX0);

        // Draw a triangle, front and back
        GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3);
        for (i = 0; i < 3; i++) 
        {
            GXPosition1x8(i);
            GXColor1x8(i);
            // Add texture coordinate
            GXTexCoord1x8(i);
        }
        GXEnd();
        GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3);
        for (i = 0; i < 3; i++) 
        {
            GXPosition1x8   ((u8)(2 - i));
            GXColor1x8      ((u8)(2 - i));
            // Add texture coordinate
            GXTexCoord1x8   ((u8)(2 - i));
        }
        GXEnd();
        MyDEMODoneRender();
        PADRead(pad);
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.
                    Initialize the view matrix
                    
    Arguments:      v    view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/

static void CameraInit ( Mtx v )
{
    Mtx44 p;
    Vec   camPt = {0.0F, 0.0F, 800.0F};
    Vec   at    = {0.0F, 0.0F, -100.0F};
    Vec   up    = {0.0F, 1.0F, 0.0F};

    MTXFrustum(p, 240.0F,-240.0F,-320.0F, 320.0F, 500, 2000);
    GXSetProjection(p, GX_PERSPECTIVE);
    MTXLookAt(v, &camPt, &up, &at);        
}

/*---------------------------------------------------------------------------*
    Name:           MyDEMODoneRender()
    
    Description:    rewrite of DEMODoneRender function - performs post-rendering operations.
                    receives a stopwatch activated at the top of the 'main' demoloop.
                    checks stopwatch after GXDrawDone and before VIWaitForRetrace to
                    compute actual frame rendering time in milliseconds.                  
               
    Arguments:      globals:  writes DemoFrameBuffer1,
                              writes DemoFrameBuffer2,
                              writes DemoCurrentBuffer
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void MyDEMODoneRender ( void )
{
    extern void*  DemoFrameBuffer1;   // static global variables from DEMOInit.c
    extern void*  DemoFrameBuffer2;
    extern void*  DemoCurrentBuffer;


    // copy out the other framebuffer since GP is a frame behind
    if( DemoCurrentBuffer == DemoFrameBuffer1 )
        GXCopyDisp(DemoFrameBuffer2, GX_TRUE);
    else
        GXCopyDisp(DemoFrameBuffer1, GX_TRUE);

    GXSetDrawSync( FIFO_DRAWDONE_TOKEN );
    
    // wait until GP is finished by polling for the drawdone token in current GP Fifo
    while( (GXReadDrawSync()) != FIFO_DRAWDONE_TOKEN )
    {        
    }

    //============================

    // Wait for vertical retrace and set the next frame buffer
    // Display the buffer which was just filled by GXCopyDisplay
    DEMOSwapBuffers();

}

/*---------------------------------------------------------------------------*
    Name:           FifoTick
    
    Description:    changes the GP breakpoint so CPU can calculate next frame
                    while GP processes last frame. should be called before any 
                    commands are sent to GP (at the top of the main loop) 
                    since this function assumes that both the CPU and GP are 
                    not processing.
                    
    Arguments:      none
    
    Returns:        none
*----------------------------------------------------------------------------*/
static void FifoTick( void )
{
    void *readPtr, *writePtr;

    // Set a breakpoint at the current point in the CPU 
    // and disable the previous one to let the GP start processing
    GXFlush();
    GXGetFifoPtrs( GXGetCPUFifo(), &readPtr, &writePtr );
    GXEnableBreakPt( writePtr );

    GXSetDrawSync( FIFO_DRAWING_TOKEN );
}



/*---------------------------------------------------------------------------*
    Name:           FifoInit
    
    Description:    Resizes the default FIFO from DEMOInit.
                    
    Arguments:      none
    
    Returns:        none
*----------------------------------------------------------------------------*/
static void FifoInit( void )
{
    GXFifoObj *fifo;
    GXFifoObj *fifoCPUCurrent;

    // get the default fifo and free it later
    GXSetDrawDone();
    fifoCPUCurrent = GXGetCPUFifo();

    fifo = OSAlloc(sizeof (GXFifoObj));

    // allocate new fifo
    GXInitFifoBase( fifo, OSAlloc(BIG_FIFO_SIZE), BIG_FIFO_SIZE );

    // set the CPU and GP fifo to this new one
    GXSetCPUFifo( fifo );
    GXSetGPFifo( fifo );        

    // so that the first fifo will fire off immediately
    GXSetDrawSync( FIFO_DRAWDONE_TOKEN );

    // free the default fifo
    OSFree( GXGetFifoBase(fifoCPUCurrent) );
}
