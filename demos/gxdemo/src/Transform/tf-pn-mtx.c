/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tf-pn-mtx.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Transform/tf-pn-mtx.c $
    
    4     5/10/01 5:07a Hirose
    removed obsolete flags
    
    3     3/24/00 5:46p Hirose
    changed to use DEMOPad library
    
    2     3/21/00 3:24p Hirose
    deleted PADInit() call because this function is called once in
    DEMOInit()
    
    1     3/06/00 12:14p Alligator
    move from tests/gx and rename
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     1/25/00 3:00p Carl
    Changed to standardized end of test message
    
    3     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    10    11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    9     10/26/99 1:33p Alligator
    remove __GXFifoInit, it's in GXInit
    
    8     9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    7     9/23/99 3:05p Ryan
    added OSHalt at end of demo
    
    6     9/23/99 2:16p Tian
    Updated for new GXFifo API
    
    5     9/17/99 5:26p Ryan
    added new DEMO calls
    
    4     9/16/99 4:43p Tian
    Temporarily added __GXFifoInit call to initialize FIFO.  will be moved
    into GXInit.
    
    3     9/15/99 2:12p Ryan
    update to fix compiler warnings
    
    2     9/07/99 2:32p Ryan
    added __SINGLEFRAME functionality
    
    1     8/12/99 2:42p Ryan
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main            ( void );

static void CameraInit      ( void );
static void DrawInit        ( void );
static void VertexLightInit ( void );
static void DrawTick        ( void );

static void AnimTick        ( void );

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
Mtx v;
u32 rot;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
           
    DrawInit();         // Define my vertex formats and set array pointers.

    DEMOPadRead();      // Read the joystick for this frame

    // While the quit button is not pressed
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU)) 
    {
        DEMOPadRead();      // Read the joystick for this frame

        // Do animation based on input
        AnimTick(); 
        DEMOBeforeRender();

        DrawTick();     // Draw the model.
        
        DEMODoneRender();
    }

    OSHalt("End of test");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.
                    
    Arguments:      v   view matrix to be passed to ViewInit
                    cameraLocScale  scale for the camera's distance from the 
                                    object - to be passed to ViewInit
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit      ( void )
{
    Mtx44 p;
    Vec camPt = {0.0F, 0.0F, 650.0F};
    Vec up = {0.0F, 1.0F, 0.0F};
    Vec origin = {0.0F, 0.0F, 0.0F};
    
    MTXFrustum(p, 240, -240, -320, 320, 500, 2000);

    GXSetProjection(p, GX_PERSPECTIVE);

    MTXLookAt(v, &camPt, &up, &origin); 
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Calls the correct initialization function for the current 
                    model.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{
    CameraInit();   // Initialize the camera.
    VertexLightInit();
}

/*---------------------------------------------------------------------------*/
static void VertexLightInit ( void )
{
    GXLightObj MyLight;
    GXColor color = {255, 255, 255, 255};
    
    GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, color);
    GXLoadLightObjImm(&MyLight, GX_LIGHT0);

    GXSetChanMatColor(GX_COLOR0, color);

    GXSetChanCtrl(  GX_COLOR0,
                    GX_TRUE, // enable channel
                    GX_SRC_REG, // amb source
                    GX_SRC_REG,     // mat source
                    GX_LIGHT0,  // light mask
                    GX_DF_CLAMP,// diffuse function
                    GX_AF_NONE);

    GXSetNumChans(1);
    GXSetNumTexGens( 0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the current model once.  
                    
    Arguments:      v       view matrix
                    m       model matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( void )
{   
    Mtx rx, ry, rz, s, mv, t;


    MTXScale(s, 70, 70, 70);
    MTXRotDeg(rx, 'X', (float)rot);
    MTXRotDeg(ry, 'Y', (float)rot);
    MTXRotDeg(rz, 'Z', (float)rot);

    MTXTrans(t, -240, 150, 0);
    MTXConcat(rx, s, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    MTXTrans(t, -80, 150, 0);
    MTXConcat(ry, s, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX1);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX1);

    MTXTrans(t, 80, 150, 0);
    MTXConcat(rz, s, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX2);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX2);

    MTXTrans(t, 240, 150, 0);
    MTXConcat(rx, s, mv);
    MTXConcat(ry, mv, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX3);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX3);

    MTXTrans(t, -240, -150, 0);
    MTXConcat(rx, s, mv);
    MTXConcat(rz, mv, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX4);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX4);

    MTXTrans(t, -80, -150, 0);
    MTXConcat(ry, s, mv);
    MTXConcat(rz, mv, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX5);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX5);

    MTXTrans(t, 80, -150, 0);
    MTXConcat(rz, s, mv);
    MTXConcat(ry, mv, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX6);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX6);

    MTXTrans(t, 240, -150, 0);
    MTXConcat(rz, s, mv);
    MTXConcat(rx, mv, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX7);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX7);

    MTXTrans(t, -80, 0, 0);
    MTXConcat(rz, s, mv);
    MTXConcat(rx, mv, mv);
    MTXConcat(ry, mv, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX8);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX8);

    MTXTrans(t, 80, 0, 0);
    MTXConcat(rx, s, mv);
    MTXConcat(ry, mv, mv);
    MTXConcat(rz, mv, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX9);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX9);

    GXSetCurrentMtx(GX_PNMTX0);
    GXDrawCube();

    GXSetCurrentMtx(GX_PNMTX1);
    GXDrawCube();

    GXSetCurrentMtx(GX_PNMTX2);
    GXDrawCube();

    GXSetCurrentMtx(GX_PNMTX3);
    GXDrawCube();

    GXSetCurrentMtx(GX_PNMTX4);
    GXDrawCube();

    GXSetCurrentMtx(GX_PNMTX5);
    GXDrawCube();

    GXSetCurrentMtx(GX_PNMTX6);
    GXDrawCube();

    GXSetCurrentMtx(GX_PNMTX7);
    GXDrawCube();

    GXSetCurrentMtx(GX_PNMTX8);
    GXDrawCube();

    GXSetCurrentMtx(GX_PNMTX9);
    GXDrawCube();

}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Animates the camera and object based on the joystick's 
                    state.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick ( void )
{
    rot ++;
    if(rot > 719)
        rot = 0;
}

/*---------------------------------------------------------------------------*/


