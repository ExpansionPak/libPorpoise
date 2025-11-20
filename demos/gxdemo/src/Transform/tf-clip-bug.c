/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tf-clip-bug.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Transform/tf-clip-bug.c $
    
    3     5/10/01 5:07a Hirose
    removed obsolete flags
    
    2     11/16/00 3:30p Carl
    Added ability to select different FB/ZB modes.
    
    1     11/15/00 6:02p Carl
    Demo to illustrate transform clipping bug.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>

/*---------------------------------------------------------------------------*
  Model Data
 *---------------------------------------------------------------------------*/

// Notes on vertex numbering:
//
// Outermost numbers represent quad corners
// Inner numbers are for fractional positions
//
//  3                       0
//  |  .  :  .  ;  .  :  .  |
// 16 14 12 10  8  6  4  2  0
//
//
//
//
// 17 15 13 11  9  7  5  3  1
//  |  .  :  .  ;  .  :  .  |
//  2                       1


// indices for verts for drawing order at tessellation = 1, 2, 4, 8

static u8 ivert[4][32] =
{
    { 0,  1, 17, 16 },
    { 0,  1,  9,  8,  8,  9, 17, 16 },
    { 0,  1,  5,  4,  4,  5,  9,  8,  8,  9, 13, 12, 12, 13, 17, 16 },
    { 0,  1,  3,  2,  2,  3,  5,  4,  4,  5,  7,  6,  6,  7,  9, 8,
      8,  9, 11, 10, 10, 11, 13, 12, 12, 13, 15, 14, 14, 15, 17, 16 }
};

#define IHALF 18

// verts

static s16 Vert_s16[] ATTRIBUTE_ALIGN(32) = 
{        
   -10,  100,  100,  // 0 0   0         0 left wall   0
   -10, -100,  100,  // 1 1   1         1             1
   -10,  100,   75,  //                 3 4           2
   -10, -100,   75,  //                 2 5           3
   -10,  100,   50,  //       3 4         7 8         4 
   -10, -100,   50,  //       2 5         6 9         5
   -10,  100,   25,  //                    11 12      6
   -10, -100,   25,  //                    10 13      7
   -10,  100,    0,  //   3 4   7 8           15 16   8 
   -10, -100,    0,  //   2 5   6 9           14 17   9
   -10,  100,  -25,  //                       20 19   10
   -10, -100,  -25,  //                       21 18   11
   -10,  100,  -50,  //          11 12     24 23      12
   -10, -100,  -50,  //          10 13     25 22      13
   -10,  100,  -75,  //                 28 27         14
   -10, -100,  -75,  //                 29 26         15
   -10,  100, -100,  // 3   7       15  31            16
   -10, -100, -100,  // 2   6       14  30            17
                                                        
    10,  100,  100,  // 0 0   0         0 right wall  18
    10, -100,  100,  // 1 1   1         1             19
    10,  100,   75,  //                 3 4           20
    10, -100,   75,  //                 2 5           21
    10,  100,   50,  //       3 4         7 8         22
    10, -100,   50,  //       2 5         6 9         23
    10,  100,   25,  //                    11 12      24
    10, -100,   25,  //                    10 13      25
    10,  100,    0,  //   3 4   7 8           15 16   26
    10, -100,    0,  //   2 5   6 9           14 17   27
    10,  100,  -25,  //                       20 19   28
    10, -100,  -25,  //                       21 18   29
    10,  100,  -50,  //          11 12     24 23      30
    10, -100,  -50,  //          10 13     25 22      31
    10,  100,  -75,  //                 28 27         32
    10, -100,  -75,  //                 29 26         33
    10,  100, -100,  // 3   7       15  31            34 
    10, -100, -100,  // 2   6       14  30            35
};        
                                  
// colors

static u32 Colors_u32[] ATTRIBUTE_ALIGN(32) = 
{         
//    r g b a
    0xc0c0c0ff, // 0        0
    0xa0a0c0ff, // 1        1
    0xb4b4c0ff, // 1/8 0-3  2
    0x9c9cc0ff, // 1/8 1-2  3
    0xa8a8c0ff, // 1/4 0-3  4
    0x9898c0ff, // 1/4 1-2  5
    0x9c9cc0ff, // 3/8 0-3  6
    0x9494c0ff, // 3/8 1-2  7
    0x9090c0ff, // 1/2 0-3  8
    0x9090c0ff, // 1/2 1-2  9
    0x8484c0ff, // 5/8 0-3  10
    0x8c8cc0ff, // 5/8 1-2  11
    0x7878c0ff, // 3/4 0-3  12
    0x8888c0ff, // 3/4 1-2  13
    0x6c6cc0ff, // 7/8 0-3  14
    0x8484c0ff, // 7/8 1-2  15
    0x8080c0ff, // 2        16
    0x6060c0ff, // 3        17

    0x2020c0ff, // 0        18
    0x4040c0ff, // 1        19
    0x2c2cc0ff, // 1/8 0-3  20
    0x4444c0ff, // 1/8 1-2  21
    0x3838c0ff, // 1/4 0-3  22
    0x4848c0ff, // 1/4 1-2  23
    0x4444c0ff, // 3/8 0-3  24
    0x4c4cc0ff, // 3/8 1-2  25
    0x5050c0ff, // 1/2 0-3  26
    0x5050c0ff, // 1/2 1-2  27
    0x5c5cc0ff, // 5/8 0-3  28
    0x5454c0ff, // 5/8 1-2  29
    0x6868c0ff, // 3/4 0-3  30
    0x5858c0ff, // 3/4 1-2  31
    0x7474c0ff, // 7/8 0-3  32
    0x5c5cc0ff, // 7/8 1-2  33
    0x6060c0ff, // 2        34
    0x8080c0ff, // 3        35
};                          

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{   
    u16 button, down;   // button return codes

    Mtx44   pm;         // projection matrix
    Mtx     cm;         // camera matrix
    Mtx     vm;         // view matrix
    Mtx     sm;         // scale matrix
    Mtx     ym;         // yet another view matrix
          
    s16      i;         // loop variable
    s16      j;         // loop variable
    u8    tess = 0;     // tessellation factor
    u8    mode = 0;     // AA/zbuffer mode

    Vec   vTmp = {0.0F, 0.0F, 0.0F};     // camera position
    Vec   at   = {0.0F, 0.0F, -6000.0F}; // look at position
    Vec   up   = {0.0F, 1.0F, 0.0F};     // camera up vector
    f32   fovy = 90.0F; // FOV in Y direction
    f32   near = 5.0f;  // near plane distance

    char  nums[100];    // number string
    GXColor green = { 0, 255, 0, 255 };
    GXColor red   = { 255, 0, 0, 255 };
    s16   spacing = 5;  // interplanar distance for Z tags
    s16   xp, yp, zp;   // Z tag position
    f32   fi;           // fractional i for distance calculation

    //----------------------------------------------------------------------

    // We use GXNtsc240Int since that makes it easy to change
    // between 16-bit and 24-bit framebuffer modes.

    DEMOInit(&GXNtsc240Int);    // Init os, pad, gx, vi

    GXSetCullMode(GX_CULL_NONE);

    GXSetArray(GX_VA_POS,  Vert_s16, 3*sizeof(s16));
    GXSetArray(GX_VA_CLR0, Colors_u32, 1*sizeof(u32));

    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS,  GX_POS_XYZ,  GX_S16,   0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    DEMOLoadFont( GX_TEXMAP0, GX_TEXMTX0, DMTF_POINTSAMPLE );

    MTXIdentity(sm);

    OSReport("\n\n********************************\n");
    OSReport("Running...\n\n");
    OSReport("Main stick   - move camera\n");
    OSReport("Sub stick    - move camera in Z\n");
    OSReport("L trigger    - hold to adjust FOV with A/B buttons\n");
    OSReport("R trigger    - hold to adjust tessellation with X/Y buttons\n");
    OSReport("A/B buttons  - adjust near plane or FOV\n");
    OSReport("X/Y buttons  - adjust Z tag interplanar dist or tessellation\n");
    OSReport("Start button - toggle 16/24-bit mode\n");
    OSReport("********************************\n");
    while(1)
    {
        DEMOPadRead();

        down   = DEMOPadGetButtonDown(0);
        button = DEMOPadGetButton(0);

        vTmp.x -= DEMOPadGetStickX(0) / 100.0f;
        vTmp.y -= DEMOPadGetStickY(0) / 100.0f;
        vTmp.z += DEMOPadGetSubStickY(0) / 100.0f;

        if (button & PAD_TRIGGER_R)
        {
            if (down & PAD_BUTTON_X)
            {
                if (tess > 0)
                    tess--;
            }
            if (down & PAD_BUTTON_Y)
            {
                if (tess < 3)
                    tess++;
            }
        } else {
            if (down & PAD_BUTTON_X)
            {
                spacing += 1;
                if (spacing > 10)
                    spacing = 10;
            }
            if (down & PAD_BUTTON_Y)
            {
                spacing  -= 1;
                if (spacing < 1)
                    spacing = 1;
            }
        }
        
        
        if (button & PAD_TRIGGER_L)
        {
            if (button & PAD_BUTTON_A)
            {
                fovy += 1.0F;
                if (fovy > 179.0F)
                    fovy = 179.0F;
            }
            if (button & PAD_BUTTON_B)
            {
                fovy -= 1.0F;
                if (fovy < 1.0F)
                    fovy = 1.0F;
            }
        } else {
            if (button & PAD_BUTTON_A)
            {
                near += 0.1F;
                if (near > 10.0F)
                    near = 10.0F;
            }
            if (button & PAD_BUTTON_B)
            {
                near -= 0.1F;
                if (near < 0.1F)
                    near = 0.1F;
            }
        }
        
        if (down & PAD_BUTTON_MENU)
        {
            mode++;
            if (mode == 5)
                mode = 0;
        }


        if (mode) 
        {
            GXSetPixelFmt( GX_PF_RGB565_Z16, (GXZFmt16)(mode - 1) );
        } else {
            GXSetPixelFmt( GX_PF_RGB8_Z24, GX_ZC_LINEAR );
        }

        DEMOBeforeRender();

        // Cancel out changes done by DEMOInitCaption (below)
        GXSetZMode( GX_ENABLE, GX_LEQUAL, GX_TRUE);
        GXSetNumChans(1);

        // Set up projection, matrices
        MTXPerspective(pm, fovy, 4.0F/3.0F, near, 10000);
        GXSetProjection(pm, GX_PERSPECTIVE);

        // Set up camera/view matrices
        MTXLookAt(cm, &vTmp, &up, &at);

        // If you want any viewing transforms, put them here
        MTXCopy(cm, vm);
        GXLoadPosMtxImm(vm, GX_PNMTX0);

        // Set up for drawing walls
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS,  GX_INDEX8);
        GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);

        // Use vertex colors
        GXSetChanCtrl(
            GX_COLOR0A0,
            GX_DISABLE,
            GX_SRC_REG,
            GX_SRC_VTX,
            GX_LIGHT_NULL,
            GX_DF_NONE,
            GX_AF_NONE );

        GXSetNumTexGens(0);
        GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

        // Draw the walls
        GXBegin(GX_QUADS, GX_VTXFMT1, (u16) ((1<<tess)*4*2));
        for (i = 0; i < (1<<tess); i++)
        {
            for(j=0; j<4; j++)  // left wall
            {
                GXPosition1x8( (u8) ivert[tess][i*4+j]);
                GXColor1x8( (u8) ivert[tess][i*4+j]);
            }
            for(j=0; j<4; j++)  // right wall
            {
                GXPosition1x8( (u8) (ivert[tess][i*4+j]+IHALF));
                GXColor1x8( (u8) (ivert[tess][i*4+j]+IHALF));
            }
        }
        GXEnd();
        
        // Set up for drawing signs
        vm[1][1] = - vm[1][1];

        // Use register colors
        GXSetChanCtrl(
            GX_COLOR0A0,
            GX_DISABLE,
            GX_SRC_REG,
            GX_SRC_REG,
            GX_LIGHT_NULL,
            GX_DF_NONE,
            GX_AF_NONE );

        GXSetNumTexGens(1);
        GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
        GXSetTevOp(GX_TEVSTAGE0, GX_BLEND);

        // Draw the signs
        for (i = 1; i < 60; i++) 
        {
            sprintf( nums, "%d00", i );

            xp = (s16) ((i>9) ? -16 : -12);
            yp = (s16) -8;
            zp = (s16) (-i * 100);

            // scale signs larger as they get further away
            sm[0][0] = i * (5.0F / 12.0F);
            sm[1][1] = i * (5.0F / 12.0F);
            MTXConcat(vm, sm, ym);
            // position signs up and down in vertical FOV
            ym[1][3] = - i * 100 + i * i * (10.0F / 3.0F);
            GXLoadPosMtxImm(ym, GX_PNMTX0);

            // Draw green sign
            GXSetChanMatColor(GX_COLOR0A0, green);
            DEMOPuts( xp, yp, zp, nums );

            // Adjust for red sign
            zp -= spacing;
            fi  = i + spacing / 100.0F;
            ym[1][3] = - fi * 100 + fi * fi * (10.0F / 3.0F);
            GXLoadPosMtxImm(ym, GX_PNMTX0);

            // Draw red sign
            GXSetChanMatColor(GX_COLOR0A0, red);
            DEMOPuts( xp, yp, zp, nums );
        }

        // Draw the control panel
        DEMOInitCaption(DM_FT_OPQ, 640, 480);
        
        sprintf(nums, "camera= [%g, %g, %g]", vTmp.x, vTmp.y, vTmp.z);
        DEMOPuts( 24, 20, 0, nums );
        sprintf(nums, "fovy = %g", fovy);
        DEMOPuts( 24, 28, 0, nums );
        sprintf(nums, "near = %g", near);
        DEMOPuts( 24, 36, 0, nums );
        sprintf(nums, "tess = %d", 1 << tess);
        DEMOPuts( 24, 44, 0, nums );
        sprintf(nums, "Z spacing = %d", spacing);
        DEMOPuts( 24, 52, 0, nums );

        switch (mode)
        {
          case 0: sprintf(nums, "24-bit linear mode"); break;
          case 1: sprintf(nums, "16-bit linear mode"); break;
          case 2: sprintf(nums, "16-bit near mode"); break;
          case 3: sprintf(nums, "16-bit mid mode"); break;
          case 4: sprintf(nums, "16-bit far mode"); break;
        }
        
        DEMOPuts( 24, 60, 0, nums );

        DEMODoneRender();
    }
    OSHalt("End of demo");
}

/*===========================================================================*/