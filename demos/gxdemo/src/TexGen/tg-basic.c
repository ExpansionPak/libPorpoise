/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-basic.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-basic.c $
    
    5     5/12/01 1:32a Hirose
    removed obsolete flags
    
    4     3/24/00 6:56p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/21/00 3:24p Hirose
    deleted PADInit() call because this function is called once in
    DEMOInit()
    
    1     3/06/00 12:12p Alligator
    move from tests/gx and rename
    
    8     3/03/00 3:19p Carl
    Fixed texture projection matrix.
    
    7     2/26/00 10:56p Hirose
    removed hardcoded magic number in JoyReadButtons
    
    6     2/24/00 11:57p Yasu
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    3     1/25/00 2:58p Carl
    Changed to standardized end of test message
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    17    11/15/99 4:49p Yasu
    Change datafile name
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>

#define WATER1_TEX_ID   3

/*---------------------------------------------------------------------------*/
Mtx v, m, mv;
Mtx44 proj;
u32 i;

u8 done;

Vec CamLoc = {0.0F, 0.0F, 8.0F};
float CameraLocScale = 100.0F;
Vec UP = {0.0F, 1.0F, 0.0F};
Vec ObjPt = {0.0F, 0.0F, 0.0F};

u8 CurrentTexGenMethod;
u8 CurrentCombineMethod;

static TEXPalettePtr tpl = 0;

/*---------------------------------------------------------------------------*/
void        main                ( void );
static void InitCamera          ( float scale );
static void DrawInit            ( void );
static void DrawTick            ( void );
static void InitTexGenMethod    ( void );
static void AnimTick            ( void );

/*---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);

    //Initialize camera
    InitCamera(100.0F);

    DrawInit();

    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {
        DEMOPadRead();

        AnimTick();
        DEMOBeforeRender();

        DrawTick();

        DEMODoneRender();
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*/
static void InitCamera ( float scale )
{
    Mtx44 p;

    Vec camPt = CamLoc;

    camPt.x *= CameraLocScale; camPt.y *= CameraLocScale; camPt.z *= CameraLocScale;

    MTXFrustum(p, .24F * scale,-.24F * scale,-.32F * scale, .32F * scale, .5F * scale, 20.0F * scale);

    GXSetProjection(p, GX_PERSPECTIVE);

    MTXLookAt(v, &camPt, &UP, &ObjPt);

}

/*---------------------------------------------------------------------------*/
static void DrawInit    ( void )
{
    GXTexObj to;
    GXLightObj lo;
    GXColor LightColor;

    GXSetCullMode(GX_CULL_NONE);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);

    /*
        Init texture object from gxTextrs.tpl
     */
    TEXGetPalette(&tpl, "gxTextrs.tpl");
    // Initialize a texture object to contain the correct texture
    TEXGetGXTexObjFromPalette(tpl, &to, WATER1_TEX_ID);
    // Load the texture object into hardware
    GXLoadTexObj(&to, GX_TEXMAP0);
    // Set the tev op to be GX_REPLACE
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    MTXLightPerspective(proj, 40.0F, 1.0F, 2.0F, 2.0F, 0.5F, 0.5F);

    GXInitLightAttn(&lo, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
    GXInitLightPos(&lo, 0.0F, 0.0F, 0.0F);
    LightColor.r = LightColor.g = LightColor.b = LightColor.a = 255;
    GXInitLightColor(&lo, LightColor);
    GXLoadLightObjImm(&lo, GX_LIGHT0);

    LightColor.r = LightColor.g = LightColor.b = LightColor.a = 0;
    GXSetNumChans(1);
    GXSetChanAmbColor(GX_COLOR0, LightColor);
    GXSetChanCtrl(GX_COLOR0A0, 1, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);

    InitTexGenMethod();
}

/*---------------------------------------------------------------------------*/
static void DrawTick    ( void )
{
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

        GXPosition3f32(100.0F, 100.0F, 0.0F);
        GXNormal3f32(0.0F, 0.0F, 1.0F);
        GXColor4u8(255, 64, 128, 255);
        GXTexCoord2f32(1.0F, 0.0F);

        GXPosition3f32(100.0F, -100.0F, 0.0F);
        GXNormal3f32(0.0F, 0.0F, 1.0F);
        GXColor4u8(255, 255, 128, 255);
        GXTexCoord2f32(1.0F, 1.0F);

        GXPosition3f32(-100.0F, -100.0F, 0.0F);
        GXNormal3f32(0.0F, 0.0F, 1.0F);
        GXColor4u8(64, 255, 128, 64);
        GXTexCoord2f32(0.0F, 1.0F);

        GXPosition3f32(-100.0F, 100.0F, 0.0F);
        GXNormal3f32(0.0F, 0.0F, 1.0F);
        GXColor4u8(64, 64, 128, 64);
        GXTexCoord2f32(0.0F, 0.0F);

    GXEnd();
}

/*---------------------------------------------------------------------------*/
static void InitTexGenMethod ( void )
{
    switch(CurrentTexGenMethod)
    {
        case 0:
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
            OSReport("TexGen - Texture coord translate in T\n\n");
            break;
        case 1:
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
            OSReport("TexGen - Texture coord rotate around origin\n\n");
            break;
        case 2:
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
            OSReport("TexGen - Texture projection\n\n");
            break;
        case 3:
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_SRTG, GX_TG_COLOR0, GX_TEXMTX0);
            OSReport("TexGen - S = R, T = G\n\n");
            break;
    }
    GXSetNumTexGens(1);
}

/*---------------------------------------------------------------------------*/
static void AnimTick    ( void )
{
    u16 buttons = DEMOPadGetButtonDown(0);

    MTXRotDeg(m, 'Y', i);
    MTXConcat(v, m, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, m);
    MTXTranspose(m, m);
    GXLoadNrmMtxImm(m, GX_PNMTX0);

    if(buttons & PAD_BUTTON_A)
    {
        CurrentTexGenMethod ++;
        if(CurrentTexGenMethod > 3) CurrentTexGenMethod = 0;

        InitTexGenMethod();
    }

    if(buttons & PAD_BUTTON_B)
    {
        CurrentCombineMethod ++;
        if(CurrentCombineMethod > 1) CurrentCombineMethod = 0;

        switch(CurrentCombineMethod)
        {
            case 0:
                GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
                OSReport("TevOp - REPLACE\n\n");
                break;
            case 1:
                GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
                OSReport("TevOp - PASSCLR\n\n");
                break;
        }
    }

    switch(CurrentTexGenMethod)
    {
        case 0:
            MTXTrans(m, 0.0F, ((float)i)/100.0F, 0.0F);
            GXLoadTexMtxImm(m, GX_TEXMTX0, GX_MTX2x4);
            break;
        case 1:
            MTXRotDeg(m, 'Z', i);
            GXLoadTexMtxImm(m, GX_TEXMTX0, GX_MTX2x4);
            break;
        case 2:
            MTXConcat((MtxPtr)proj, mv, mv);
            GXLoadTexMtxImm(mv, GX_TEXMTX0, GX_MTX3x4);
            break;
    }

    i++;
}

/*============================================================================*/

