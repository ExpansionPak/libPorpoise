/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     smp-texexample.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Simple/smp-texexample.c $
    
    8     2/14/01 11:09a Hirose
    Changed to accept start button only to quit. Deleted obsolete ifdefs
    
    7     10/25/00 8:59p Hirose
    A flag fix MAC -> MACOS
    
    6     7/21/00 2:47p Hirose
    removed MAC only instruction from other build environments
    
    5     7/21/00 2:44p Hirose
    fixed header info.
    
    4     3/29/00 7:09p Carl
    Removed redundant init code; cleaned up comments.
    
    3     3/23/00 8:01p Hirose
    fixed instruction message
    
    2     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    1     3/16/00 7:24p Alligator
    rename to match other demos conventions
    
    1     3/06/00 12:11p Alligator
    move from gxdemos to gxdemos/Simple
    
    4     1/19/00 6:15p Alligator
    
    3     1/18/00 7:00p Alligator
    updated to work with new GXInit defaults
    
    2     1/13/00 8:53p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    14    11/15/99 4:49p Yasu
    Change datafile name
    
    13    11/12/99 4:59p Yasu
    Add GXSetNumTexGens
    
    12    11/03/99 7:39p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    11    9/30/99 10:33p Yasu
    Renamed some functions and enums
    
    10    9/30/99 2:13p Ryan
    sweep to remove gxmodels libs
    
    9     9/17/99 5:26p Ryan
    added new DEMO calls
    
    8     9/15/99 3:02p Ryan
    update to fix compiler warnings
    
    7     7/30/99 1:39p Ryan
    changed demo so that it loads a tpl file for textures
    
    6     7/23/99 2:51p Ryan
    changed dolphinDemo.h to demo.h
    
    5     7/23/99 12:15p Ryan
    included dolphinDemo.h
    
    4     7/20/99 6:03p Alligator
    demo library
    
    3     7/06/99 2:25p Alligator
    modified to use use new GXInitTexObj api
    
    2     7/02/99 2:56p Alligator
    fixed matrix transpose bug
    
    1     6/17/99 3:01a Alligator
    texture example, from GX.doc

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>

#define BALL64_TEX_ID        8

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
    u8            i;       // loop variable
    TEXPalettePtr tpl = 0; // texture palette 
    
    pad[0].button = 0;

    DEMOInit(NULL);    // Init os, pad, gx, vi

    CameraInit(v);
    GXLoadPosMtxImm(v, GX_PNMTX0);

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
    
    while(!(pad[0].button & PAD_BUTTON_MENU))
    {
        DEMOBeforeRender();
        // Draw a triangle
        GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3);
        for (i = 0; i < 3; i++) {
            GXPosition1x8(i);
            GXColor1x8(i);
            // Add texture coordinate
            GXTexCoord1x8(i);
        }
        GXEnd();
        DEMODoneRender();
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

