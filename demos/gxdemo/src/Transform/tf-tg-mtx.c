/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tf-tg-mtx.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Transform/tf-tg-mtx.c $
    
    6     5/10/01 5:07a Hirose
    removed obsolete flags
    
    5     7/07/00 5:57p Dante
    PC Compatibility
    
    4     3/24/00 5:46p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/21/00 3:24p Hirose
    deleted PADInit() call because this function is called once in
    DEMOInit()
    
    1     3/06/00 12:14p Alligator
    move from tests/gx and rename
    
    6     3/02/00 2:25p Carl
    Removed center cube for Q=0 cases.
    
    5     2/26/00 11:45p Hirose
    removed hardcoded magic number in JoyReadButtons
    
    4     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    3     1/25/00 3:00p Carl
    Changed to standardized end of test message
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    12    11/15/99 4:49p Yasu
    Change datafile name
    
    11    11/09/99 2:20p Alligator
    added number of tex gens
    
    10    11/08/99 9:10a Alligator
    fixed normal matrix tex gen to use 3x4 matrix
    
    9     11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    8     9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    7     9/23/99 2:52p Ryan
    added OSHalt at end of demo
    
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

static void SetTexGenMethod ( GXTexMtx mtxIdx );

static void SetNoTexGenMtx          ( void );
static void SetProjectionTexGenMtx  ( void );
static void SetTexCoordTexGenMtx    ( void );
static void SetReflectionTexGenMtx  ( void );

#define SIDE 35
#define NORM (sqrt(3.0))/3.0

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
Mtx v;
u32 rot;
u32 rot2;
float trans;

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
        DEMOPadRead();  // Read the joystick for this frame

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
    TEXPalettePtr tpl = 0;
    u32           i;

    CameraInit();   // Initialize the camera.

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetArray(GX_VA_TEX0, FloatTex, 8);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
    GXSetArray(GX_VA_NRM, FloatNorm, 12);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc (GX_VA_POS, GX_INDEX16);
    GXSetArray(GX_VA_POS, FloatVert, 12);

    TEXGetPalette(&tpl, "gxTests/tf-02.tpl");

    TEXGetGXTexObjFromPalette(tpl, &to1, 0);
    TEXGetGXTexObjFromPalette(tpl, &to2, 1);

    GXLoadTexObj(&to1, GX_TEXMAP0);

    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

    for(i = 0; i < 24; i++)
    {
        FloatNorm[i] *= NORM;
    }

    OSReport("\n\nNo TexGen\n\n");
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

    switch(CurrentControl)
    {
        case 0:
            SetNoTexGenMtx();
            break;
        case 1:
            SetProjectionTexGenMtx();
            break;
        case 2:
            SetTexCoordTexGenMtx();
            break;
        case 3:
            SetReflectionTexGenMtx();
            break;
    }

    SetTexGenMethod(GX_TEXMTX0);
    DrawSmoothCube(-240, 150);

    SetTexGenMethod(GX_TEXMTX1);
    DrawSmoothCube(-80, 150);

    SetTexGenMethod(GX_TEXMTX2);
    DrawSmoothCube(80, 150);

    SetTexGenMethod(GX_TEXMTX3);
    DrawSmoothCube(240, 150);

    SetTexGenMethod(GX_TEXMTX4);
    DrawSmoothCube(-240, -150);

    SetTexGenMethod(GX_TEXMTX5);
    DrawSmoothCube(-80, -150);

    SetTexGenMethod(GX_TEXMTX6);
    DrawSmoothCube(80, -150);

    SetTexGenMethod(GX_TEXMTX7);
    DrawSmoothCube(240, -150);
    
    SetTexGenMethod(GX_TEXMTX8);
    DrawSmoothCube(-160, 0);

    SetTexGenMethod(GX_TEXMTX9);
    DrawSmoothCube(160, 0);

    // Don't draw cube with identity matrix for projected or
    // normal texcoords since this results in Q=0 on 4 cube faces
    if (CurrentControl != 1 && CurrentControl != 3)
    {
        SetTexGenMethod(GX_IDENTITY);
        DrawSmoothCube(0, 0);
    }
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
        if(CurrentControl > 3)
            CurrentControl = 0;

        switch(CurrentControl)
        {
            case 0:
                GXLoadTexObj(&to1, GX_TEXMAP0);
                OSReport("\n\nNo TexGen\n\n");
                break;

            case 1:
                OSReport("\n\nTexture Projection (position * 3x4 matrix)\n\n");
                break;

            case 2:
                OSReport("\n\nTexture rotation and translation (TexCoord * 2x4 matrix)\n\n");
                break;

            case 3:
                GXLoadTexObj(&to2, GX_TEXMAP0);
                OSReport("\n\nReflection mapping (normal * 3x4 matrix)\n\n");
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

    rot2 ++;
    if(rot2 > 1439)
        rot2 = 0;

    trans += .01F;
    if(trans > 2.0F)
        trans = 0.0F;
}

/*---------------------------------------------------------------------------*/
static void DrawSmoothCube ( float tx, float ty )
{
    Mtx ry, rz, mv, t;

    MTXRotDeg(ry, 'Y', (float)rot);
    MTXRotDeg(rz, 'Z', (float)rot);
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
    GXTexCoord1x16(texCoordIndex);
}

/*---------------------------------------------------------------------------*/
static void SetTexGenMethod ( GXTexMtx mtxIdx )
{
    switch(CurrentControl)
    {   
        case 0:
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, mtxIdx);
            break;

        case 1:
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, mtxIdx);
            break;

        case 2:
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, mtxIdx);
            break;

        case 3:
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, mtxIdx);
            break;

    }
}

/*---------------------------------------------------------------------------*/
static void SetNoTexGenMtx ( void )
{
    Mtx m;

    MTXIdentity(m);

    GXLoadTexMtxImm(m, GX_TEXMTX0, GX_MTX2x4);
    GXLoadTexMtxImm(m, GX_TEXMTX1, GX_MTX2x4);
    GXLoadTexMtxImm(m, GX_TEXMTX2, GX_MTX2x4);
    GXLoadTexMtxImm(m, GX_TEXMTX3, GX_MTX2x4);
    GXLoadTexMtxImm(m, GX_TEXMTX4, GX_MTX2x4);
    GXLoadTexMtxImm(m, GX_TEXMTX5, GX_MTX2x4);
    GXLoadTexMtxImm(m, GX_TEXMTX6, GX_MTX2x4);
    GXLoadTexMtxImm(m, GX_TEXMTX7, GX_MTX2x4);
    GXLoadTexMtxImm(m, GX_TEXMTX8, GX_MTX2x4);
    GXLoadTexMtxImm(m, GX_TEXMTX9, GX_MTX2x4);
}

/*---------------------------------------------------------------------------*/
static void SetProjectionTexGenMtx ( void )
{
    //assume all lights are at the origin pointing down the -z axis

    Mtx proj;
    Mtx ry, rz, mv;//, t;

    MTXRotDeg(ry, 'Y', (float)rot);
    MTXRotDeg(rz, 'Z', (float)rot);

    MTXConcat(rz, ry, mv);
    MTXConcat(v, mv, mv);

    MTXLightFrustum(proj, -5, 5, -5, 5, 50, 0.5F, 0.5F, 0.5F, 0.5F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX0, GX_MTX3x4);

    MTXLightFrustum(proj, -5, 5, -5, 5, 20, 0.5F, 0.5F, 0.5F, 0.5F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX1, GX_MTX3x4);

    MTXLightFrustum(proj, -5, 5, -5, 5, 30, 0.5F, 0.5F, 0.7F, 0.3F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX2, GX_MTX3x4);

    MTXLightFrustum(proj, -5, 5, -5, 5, 25, 1.0F, 1.0F, 0.5F, 0.5F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX3, GX_MTX3x4);

    MTXLightFrustum(proj, -10, 10, -5, 5, 5, 0.5F, 0.5F, 0.5F, 0.5F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX4, GX_MTX3x4);

    MTXLightFrustum(proj, -5, 5, -20, 20, 5, 0.5F, 0.5F, 0.5F, 0.5F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX5, GX_MTX3x4);

    MTXLightFrustum(proj, -5, 5, -20, 20, 25, 0.5F, 0.5F, 0.5F, 0.5F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX6, GX_MTX3x4);

    MTXLightFrustum(proj, -5, 5, -20, 20, 8, 0.5F, 0.5F, 0.5F, 0.5F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX7, GX_MTX3x4);

    MTXLightFrustum(proj, -5, 5, -20, 20, 27, 1.5F, 1.5F, 0.5F, 0.5F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX8, GX_MTX3x4);

    MTXLightFrustum(proj, -20, 20, -20, 20, 29, 0.5F, 0.5F, 0.75F, 0.75F);
    MTXConcat(proj, mv, proj);
    GXLoadTexMtxImm(proj, GX_TEXMTX9, GX_MTX3x4);


}

/*---------------------------------------------------------------------------*/
static void SetTexCoordTexGenMtx ( void )
{
    Mtx rz, tx, ty, mt;

    MTXRotDeg(rz, 'Z', ((float)rot2) / 4.0F);
    MTXTrans(tx, trans, 0.0F, 0.0F);
    MTXTrans(ty, 0.0F, trans, 0.0F);

    GXLoadTexMtxImm(rz, GX_TEXMTX0, GX_MTX2x4);
    GXLoadTexMtxImm(tx, GX_TEXMTX1, GX_MTX2x4);
    GXLoadTexMtxImm(ty, GX_TEXMTX2, GX_MTX2x4);

    MTXConcat(ty, rz, mt);
    GXLoadTexMtxImm(mt, GX_TEXMTX3, GX_MTX2x4);
    MTXConcat(tx, rz, mt);
    GXLoadTexMtxImm(mt, GX_TEXMTX4, GX_MTX2x4);
    MTXConcat(ty, tx, mt);
    GXLoadTexMtxImm(mt, GX_TEXMTX5, GX_MTX2x4);
    MTXConcat(tx, rz, mt);
    MTXConcat(ty, mt, mt);
    GXLoadTexMtxImm(mt, GX_TEXMTX6, GX_MTX2x4);

    MTXTrans(tx, trans/2.0F, 0.0F, 0.0F);
    MTXTrans(ty, 0.0F, trans * 2.0F, 0.0F);

    GXLoadTexMtxImm(tx, GX_TEXMTX7, GX_MTX2x4);
    GXLoadTexMtxImm(ty, GX_TEXMTX8, GX_MTX2x4);
    MTXConcat(tx, ty, mt);
    GXLoadTexMtxImm(mt, GX_TEXMTX9, GX_MTX2x4);
}

/*---------------------------------------------------------------------------*/
static void SetReflectionTexGenMtx ( void )
{
    Mtx ry, rz, mv, t, s;

    MTXRotDeg(ry, 'Y', (float)rot);
    MTXRotDeg(rz, 'Z', (float)rot);
    MTXScale(s, -0.5F, -0.5F, 0.0F);
    MTXTrans(t, 0.5F, 0.5F, 1.0F);

    MTXConcat(rz, ry, mv);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    MTXConcat(s, mv, mv);
    MTXConcat(t, mv, mv);

    GXLoadTexMtxImm(mv, GX_TEXMTX0, GX_MTX3x4);
    GXLoadTexMtxImm(mv, GX_TEXMTX1, GX_MTX3x4);
    GXLoadTexMtxImm(mv, GX_TEXMTX2, GX_MTX3x4);
    GXLoadTexMtxImm(mv, GX_TEXMTX3, GX_MTX3x4);
    GXLoadTexMtxImm(mv, GX_TEXMTX4, GX_MTX3x4);
    GXLoadTexMtxImm(mv, GX_TEXMTX5, GX_MTX3x4);
    GXLoadTexMtxImm(mv, GX_TEXMTX6, GX_MTX3x4);
    GXLoadTexMtxImm(mv, GX_TEXMTX7, GX_MTX3x4);
    GXLoadTexMtxImm(mv, GX_TEXMTX8, GX_MTX3x4);
    GXLoadTexMtxImm(mv, GX_TEXMTX9, GX_MTX3x4);
}

/*===========================================================================*/