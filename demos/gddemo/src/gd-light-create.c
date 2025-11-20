/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gd-light-create.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-light-create.c $
    
    1     9/21/01 4:06p Hirose
    Initial check in.
  
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   gd-light
     Displaylist demo with lighting commands
     [Displaylist creation function for both off-line/runtime]
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include "gd-light.h"

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
static void SendPartOfTorus ( u32 start, u32 end );

/*---------------------------------------------------------------------------*
   Data
 *---------------------------------------------------------------------------*/
static GXColor AmbColor0  = { 0x20, 0x20, 0x20, 0x20 };
static GXColor AmbColor1  = { 0x00, 0x00, 0x00, 0x00 };
static GXColor LightColor = { 0xC0, 0xC0, 0xC0, 0xC0 };
static GXColor MatColors[8] =
{
    { 0x20, 0xA0, 0xA0, 0xFF },
    { 0xA0, 0xA0, 0xA0, 0xA0 },
    { 0xA0, 0x80, 0x40, 0xFF },
    { 0x80, 0x80, 0x20, 0x60 },
    { 0x30, 0x30, 0x30, 0xFF },
    { 0xFF, 0xFF, 0xFF, 0xFF },
    { 0xFF, 0xC0, 0xC0, 0xFF },
    { 0x30, 0x30, 0x30, 0x30 }
};

static GXVtxDescList VcdSetting[] =
{
    { GX_VA_POS, GX_INDEX16 },
    { GX_VA_NRM, GX_INDEX16 },
    { GX_VA_NULL, GX_NONE },    // NULL Terminator
};


/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           CreateModelDL
    
    Description:    Creates the display list used by the program.
                    GD library is used for creating the display list.
                    
    Arguments:      dlPtr : pointer to memory for the display list
                    plPtr : pointer to memory for patch list
    
    Returns:        actual size of the display list
 *---------------------------------------------------------------------------*/
u32 CreateModelDL ( void* dlPtr, u32* plPtr )
{
    GDLObj  dlObj;
    u32     size;

    GDInitGDLObj(&dlObj, dlPtr, MODELDL_SIZE_MAX);
    GDSetCurrent(&dlObj);

    // Lighting channel setting
    GDSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
    GDSetChanCtrl(GX_COLOR1, GX_ENABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT1, GX_DF_NONE, GX_AF_SPEC);
    GDSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GDSetChanCtrl(GX_ALPHA1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

    // Ambient color
    GDSetChanAmbColor(GX_COLOR0A0, AmbColor0);
    GDSetChanAmbColor(GX_COLOR1A1, AmbColor1);


    // Light setting
    GDSetLightColor(GX_LIGHT0, LightColor);
    GDSetLightColor(GX_LIGHT1, LightColor);
    // LIGHT0(Pos) will be patched in run-time. Put dummy for now.
    plPtr[MODELDL_PATCH_LIGHT0POS] = GDGetCurrOffset();
    GDSetLightPos(GX_LIGHT0, 0.0F, 0.0F, 0.0F);
    // LIGHT1(Dir) will be patched in run-time. Put dummy for now.
    plPtr[MODELDL_PATCH_LIGHT1DIR] = GDGetCurrOffset();
    GDSetSpecularDir(GX_LIGHT1, 0.0F, 0.0F, 1.0F);


    // Vertex descriptor
    GDSetVtxDescv(VcdSetting);


    // 1st./5th. parts (material + body)
    GDSetLightShininess(GX_LIGHT1, 256.0F);
    GDSetChanMatColor(GX_COLOR0A0, MatColors[0]);
    GDSetChanMatColor(GX_COLOR1A1, MatColors[1]);
    SendPartOfTorus( 0,  8);
    SendPartOfTorus(32, 40);

    // 2nd./6th. parts (material + body)
    GDSetLightShininess(GX_LIGHT1, 48.0F);
    GDSetChanMatColor(GX_COLOR0A0, MatColors[2]);
    GDSetChanMatColor(GX_COLOR1A1, MatColors[3]);
    SendPartOfTorus( 8, 16);
    SendPartOfTorus(40, 48);

    // 3rd./7th. parts (material + body)
    GDSetLightShininess(GX_LIGHT1, 128.0F);
    GDSetChanMatColor(GX_COLOR0A0, MatColors[4]);
    GDSetChanMatColor(GX_COLOR1A1, MatColors[5]);
    SendPartOfTorus(16, 24);
    SendPartOfTorus(48, 56);

    // 4th./8th. parts (material + body)
    GDSetLightShininess(GX_LIGHT1, 16.0F);
    GDSetChanMatColor(GX_COLOR0A0, MatColors[6]);
    GDSetChanMatColor(GX_COLOR1A1, MatColors[7]);
    SendPartOfTorus(24, 32);
    SendPartOfTorus(56, 64);


    // close the DL (padding + flush)
    GDPadCurr32();
    GDFlushCurrToMem();
    
    // get actual DL size
    size = GDGetCurrOffset();    

    // release GD object
    GDSetCurrent(NULL);
    
    return size;
}

/*---------------------------------------------------------------------------*
    Draw a part of the torus model
 *---------------------------------------------------------------------------*/
static void SendPartOfTorus ( u32 start, u32 end )
{
    u32     i, j, k;
    u16     idx;

    // send vertex data by using GD functions.
    for ( i = start ; i < end ; i++ )
    {
        GDBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)((MODEL_N0+1)*2));
            for ( j = 0 ; j <= MODEL_N0 ; j++ )
            {
                k = (j % MODEL_N0) * MODEL_N1;
               
                idx = (u16)(k + ((i+1) % MODEL_N1));
                // Position
                GDPosition1x16(idx);
                // Normal
                GDNormal1x16(idx);

                idx = (u16)(k + (i % MODEL_N1));
                // Position
                GDPosition1x16(idx);
                // Normal
                GDNormal1x16(idx);
            }
        GDEnd();
    }
}


/*============================================================================*/
