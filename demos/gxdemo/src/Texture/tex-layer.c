/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-layer.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-layer.c $
    
    4     5/11/01 1:00a Hirose
    removed obsolete flags. fixed some verify messages.
    
    3     3/24/00 4:30p Hirose
    changed to use DEMOPad library
    
    2     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    5     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    4     2/26/00 11:28p Hirose
    removed hardcoded magic number in JoyReadButtons
    
    3     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    7     11/15/99 4:49p Yasu
    Change datafile name
    
    6     11/09/99 7:48p Hirose
    added GXSetNumTexGens
    
    5     11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    4     10/29/99 3:46p Hirose
    replaced GXSetTevStages(GXTevStageID) by GXSetNumTevStages(u8)
    
    3     10/22/99 4:39p Yasu
    Add GXSetTevStages and GXSetTevOrder
    
    2     9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    1     9/29/99 1:49p Ryan
    
    6     9/17/99 5:26p Ryan
    added new DEMO calls
    
    5     9/15/99 2:24p Ryan
    Update to fix compiler warnings
    
    4     9/01/99 12:25p Ryan
    Added __SINGLEFRAME stuff
    
    2     8/26/99 11:57a Ryan
    
    1     8/23/99 3:44p Ryan
    
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
static void DrawTick        ( void );

static void AnimTick        ( void );

static void DrawSmoothCube  ( float tx, float ty );
static void SendVertex      ( u16 posIndex, u16 texCoordIndex );

#define SIDE 105
#define NORM (sqrt(3.0))/3.0

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
Mtx v;
u32 rot;

u8 CurrentControl;
u8 DoRotation = 1;

GXTexObj to1, to2;

float FloatVert[] ATTRIBUTE_ALIGN(32) = 
                    {   -SIDE, SIDE, -SIDE,
                        -SIDE, SIDE, SIDE,
                        -SIDE, -SIDE, SIDE,
                        -SIDE, -SIDE, -SIDE,
                        SIDE, SIDE, -SIDE,
                        SIDE, -SIDE, -SIDE,
                        SIDE, -SIDE, SIDE,
                        SIDE, SIDE, SIDE
                    };

float FloatNorm[] ATTRIBUTE_ALIGN(32) = 
                    {   -1, 1, -1,
                        -1, 1, 1,
                        -1, -1, 1,
                        -1, -1, -1,
                        1, 1, -1,
                        1, -1, -1,
                        1, -1, 1,
                        1, 1, 1
                    };

float FloatTex[] ATTRIBUTE_ALIGN(32) =  
                    {   0.0F, 0.0F, 
                        1.0F, 0.0F,
                        1.0F, 1.0F, 
                        0.0F, 1.0F
                    };

u8  ColorRGBA8[] ATTRIBUTE_ALIGN(32) = {255, 255, 255, 255}; //GX_RGBA8

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
           
    DrawInit();         // Define my vertex formats and set array pointers.

    DEMOPadRead();          // Read the joystick for this frame

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
    OSHalt("End of demo.\n");
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
    Mtx rz;
    TEXPalettePtr tpl = 0;
    u32 i;

    CameraInit();   // Initialize the camera.

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetArray(GX_VA_TEX0, FloatTex, 8);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX1, GX_INDEX16);
    GXSetArray(GX_VA_TEX1, FloatTex, 8);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
    GXSetArray(GX_VA_NRM, FloatNorm, 12);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc (GX_VA_POS, GX_INDEX16);
    GXSetArray(GX_VA_POS, FloatVert, 12);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetArray(GX_VA_CLR0, ColorRGBA8, 4);

    TEXGetPalette(&tpl, "gxTests/tex-05.tpl");

    TEXGetGXTexObjFromPalette(tpl, &to1, 1);
    TEXGetGXTexObjFromPalette(tpl, &to2, 0);

    GXLoadTexObj(&to1, GX_TEXMAP0);
    GXLoadTexObj(&to2, GX_TEXMAP1);

    GXSetNumTevStages(2);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOp(GX_TEVSTAGE1, GX_MODULATE);

    for(i = 0; i < 24; i++)
    {
        FloatNorm[i] *= NORM;
    }

    GXSetNumTexGens(2);
    MTXScale(rz, 3.0F, 3.0F, 3.0F);
    GXLoadTexMtxImm(rz, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
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
    DrawSmoothCube(0, 0);
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
    u16 buttons = DEMOPadGetButtonDown(0);

    if(buttons & PAD_BUTTON_X)
    {
        CurrentControl ++;
        if(CurrentControl > 2)
            CurrentControl = 0;

        switch(CurrentControl)
        {
            case 0:
                GXSetTevOp(GX_TEVSTAGE1, GX_MODULATE);
                OSReport("\n\nTev Mode - modulate\n\n");
                break;

            case 1:
                GXSetTevOp(GX_TEVSTAGE1, GX_DECAL);
                OSReport("\n\nTev Mode - decal\n\n");
                break;

            case 2:
                GXSetTevOp(GX_TEVSTAGE1, GX_PASSCLR);
                OSReport("\n\nTexture 2 off\n\n");
                break;
        }
    }

    if(buttons & PAD_BUTTON_B)
    {
        if(DoRotation)
            DoRotation = 0;
        else
            DoRotation = 1;
    }
    
    if(DoRotation)
    {
        rot ++;
        if(rot > 1439)
            rot = 0;
    }
}

/*---------------------------------------------------------------------------*/
static void DrawSmoothCube ( float tx, float ty )
{
    Mtx ry, rz, mv, t;

    MTXRotDeg(ry, 'Y', (float)rot);
    MTXRotDeg(rz, 'Z', (float)rot / 2.0F);
    MTXTrans(t, tx, ty, 0);

    MTXConcat(rz, ry, mv);
    MTXConcat(t, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);

        SendVertex(0, 0);
        SendVertex(1, 1);
        SendVertex(2, 2);
        SendVertex(3, 3);

        SendVertex(4, 0);
        SendVertex(5, 1);
        SendVertex(6, 2);
        SendVertex(7, 3);       
                         
        SendVertex(2, 0);
        SendVertex(6, 1);
        SendVertex(5, 2);
        SendVertex(3, 3);
        
        SendVertex(1, 0);
        SendVertex(0, 1);
        SendVertex(4, 2);
        SendVertex(7, 3);   
        
        SendVertex(5, 0);
        SendVertex(4, 1);
        SendVertex(0, 2);
        SendVertex(3, 3);
        
        SendVertex(6, 0);
        SendVertex(2, 1);
        SendVertex(1, 2);
        SendVertex(7, 3);   

    GXEnd();
}

/*---------------------------------------------------------------------------*/
static void SendVertex ( u16 posIndex, u16 texCoordIndex )
{
    GXPosition1x16(posIndex);
    GXNormal1x16(posIndex);
    GXColor1x16(0);
    GXTexCoord1x16(texCoordIndex);
    GXTexCoord1x16(texCoordIndex);
}

/*============================================================================*/
