/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     G2D-test.c (Test of 2D API by Paul Donnelly, Nov. 1999)

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/G2D/G2D-test.c $
    
    6     5/11/01 9:20p Hirose
    removed obsolete flags.
    set gamma to 1.7 due to default gamma value change.
    
    5     10/24/00 9:41a Hirose
    added explicit NULL initialization of TEXPalletePtr
    
    4     7/01/00 1:32p Alligator
    added PrintIntro
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/23/00 2:10a Hirose
    updated to use DEMOPad library / deleted all tab codes
    
    1     3/06/00 12:09p Alligator
    move from tests/gx and rename
    
    10    2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    9     2/01/00 4:17p Hirose
    merged Paul's source modified at RD3
    
    8     1/28/00 3:16p Hirose
    added single frame test (temporary version)
    
    7     1/25/00 2:49p Carl
    Changed to standardized end of test message
    
    6     1/13/00 8:53p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    5     99/12/15 8:41p Paul
    New Tiles
    
    4     99/12/14 5:13p Paul
    
    3     99/12/14 8:34a Paul
    
    2     99/12/12 10:07p Paul
    
    1     99/12/09 12:28p Paul

 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>
#include <stdio.h>

#include "G2D-test.h"

/*---------------------------------------------------------------------------*
  Defines
 *---------------------------------------------------------------------------*/

// Size of sort buffer is based on 1200 16x16 blocks fitting on a
// 640x480 screen , plus some extra for when we are at an angle,
// and for lots of switching between materials
#define SORT_BUFFER         12000

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main                ( void );

static void CameraInit          ( void );
static void CameraUpdate        ( void );
static void DrawInit            ( void );
static void DrawTick            ( void );

static void MoveClouds          ( void );

static void SetupTransforms     ( void );
static void DrawSprites         ( void );

static void SendParticlePoint   ( Vec *vPoint, u8 colorIndex );
static void SendParticleLine    ( Vec *vPoint1, Vec *vDelta, u8 colorIndex );

static void PrintIntro          ( void );

 /*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

static u8 *map;

static GXTexObj toSpritesRGBA8_1;

static G2DSprite sprShip =
{
    0, 0,
    32, 32,
    &toSpritesRGBA8_1,
};
static G2DSprite sprShadow =
{
    64, 0,
    32, 32,
    &toSpritesRGBA8_1,
};

static TEXPalettePtr tpl = NULL;


/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);

    DrawInit();         // Define my vertex formats and set array pointers.
    AnimInit();

    PrintIntro();
    DEMOPadRead();      // Read the joystick for this frame

    // While the quit button is not pressed
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU)) 
    {
        DEMOPadRead();  // Read the joystick for this frame

        AnimTick();     // Do animation based on input

        DEMOBeforeRender();

        DrawTick();     // Draw the model.

        DEMODoneRender();
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Calls the correct initialization function for the current
                    model.

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{
    TEXGetPalette(&tpl, "gxTests/G2D-00.tpl");

    // Load Sprite 
    GXInitTexObjLOD( &toSpritesRGBA8_1,
        GX_LINEAR, GX_LINEAR, 0.0F, 0.0F, 0.0F,
        GX_FALSE, GX_FALSE, GX_ANISO_1 );
    TEXGetGXTexObjFromPalette(tpl, &toSpritesRGBA8_1, 2);

    G2DInitSprite( &sprShip );
    G2DInitSprite( &sprShadow );

    // Set gamma to 1.7 since this database has been assuming PC displays
    GXSetDispCopyGamma( GX_GM_1_7 );

    InitLevel1( &tpl );
}


/*---------------------------------------------------------------------------*
    Name:           MoveClouds

    Description:    Moves cloud layer

    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void MoveClouds( void )
{
    f32 rWorldWidth, rWorldHeight;
    static f32 rCloudX = 0.0F;
    static f32 rCloudY = 0.0F;
    G2DPosOri poCloudCam;

    poCloudCam = poCam;

    poCloudCam.rPosX += rCloudX;
    poCloudCam.rPosY += rCloudY;

    rCloudX += 0.5F;
    rCloudY += 0.25F;

    rWorldWidth = (f32)(lyrBack.nTileWidth << lyrBack.nHS);
    rWorldHeight = (f32)(lyrBack.nTileHeight << lyrBack.nVS);

    if (poCloudCam.rPosX >= rWorldWidth)  { poCloudCam.rPosX -= rWorldWidth; }
    else if (poCloudCam.rPosX < 0)        { poCloudCam.rPosX += rWorldWidth; }
    if (poCloudCam.rPosY >= rWorldHeight) { poCloudCam.rPosY -= rWorldHeight; }
    else if (poCloudCam.rPosY < 0)        { poCloudCam.rPosY += rWorldHeight; }

    G2DSetCamera( &poCloudCam );
}

#define SIXTEEN_OVER_PI 5.0929581789406507446042804279205F
G2DPosOri poShadow;

/*---------------------------------------------------------------------------*
    Name:           DrawTick

    Description:    Draw the current model once.  

    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( void )
{
    s8 aSortBuffer[SORT_BUFFER];
    map = lyrBack.map;

    if (nMode == 2) // Map editor mode
    {
        RenderEditorMode( aSortBuffer );
    }
    else
    {
        s32 nFrame = (32 - (s32)(rAng * SIXTEEN_OVER_PI)) & 0x1f;

        G2DDrawLayer( &lyrBack, aSortBuffer );

        poShadow = poShip;
        poShadow.rPosX += 9.0F;
        poShadow.rPosY += 9.0F;
        G2DDrawSprite( &sprShadow, &poShadow );
        
        sprShip.nTlcT = (u16)((nFrame & 0xf)<<5);
        sprShip.nTlcS = (u16)((nFrame & 0x10)<<1);
        G2DInitSprite( &sprShip );
        G2DDrawSprite( &sprShip,   &poShip );
        MoveClouds();
        G2DDrawLayer( &lyrFront, aSortBuffer );
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
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("G2D-test: test game that uses 2D library\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("  Stick X/Y    : move forward/back\n");
    OSReport("  B button     : brake\n");
    OSReport("  L/R triggers : move left/right\n");
    OSReport("************************************************\n\n");
}

