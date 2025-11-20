/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     DL-tex.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/DisplayList/DL-tex.c $
    
    3     5/14/01 4:16p Carl
    Changed variable name to make code a bit more clear.
    
    2     4/25/01 8:03p Carl
    Defined a couple more macros to make source look cleaner.
    
    1     2/15/01 11:57a Carl
    Demo showing how to put textures into display lists.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/

// Defines for setting texture registers...

//
//  tx_setmode0 register
//
#define TX_SETMODE0_WRAP_S_SHIFT        0
#define TX_SETMODE0_WRAP_T_SHIFT        2
#define TX_SETMODE0_MAG_FILTER_SHIFT    4
#define TX_SETMODE0_MIN_FILTER_SHIFT    5
#define TX_SETMODE0_DIAGLOD_SHIFT       8
#define TX_SETMODE0_LODBIAS_SHIFT       9
#define TX_SETMODE0_MAXANISO_SHIFT      19
#define TX_SETMODE0_LODCLAMP_SHIFT      21
#define TX_SETMODE0_RID_SHIFT           24
#define TX_SETMODE0(wrap_s, wrap_t, mag_filter, min_filter, diaglod, lodbias, maxaniso, lodclamp, rid) \
        ((((u32)(wrap_s)) << TX_SETMODE0_WRAP_S_SHIFT) | \
         (((u32)(wrap_t)) << TX_SETMODE0_WRAP_T_SHIFT) | \
         (((u32)(mag_filter)) << TX_SETMODE0_MAG_FILTER_SHIFT) | \
         (((u32)(min_filter)) << TX_SETMODE0_MIN_FILTER_SHIFT) | \
         (((u32)(diaglod)) << TX_SETMODE0_DIAGLOD_SHIFT) | \
         (((u32)(lodbias)) << TX_SETMODE0_LODBIAS_SHIFT) | \
         (((u32)(maxaniso)) << TX_SETMODE0_MAXANISO_SHIFT) | \
         (((u32)(lodclamp)) << TX_SETMODE0_LODCLAMP_SHIFT) | \
         (((u32)(rid)) << TX_SETMODE0_RID_SHIFT))

//
//  tx_setmode1 register
//
#define TX_SETMODE1_MINLOD_SHIFT        0
#define TX_SETMODE1_MAXLOD_SHIFT        8
#define TX_SETMODE1_RID_SHIFT           24
#define TX_SETMODE1(minlod, maxlod, rid) \
        ((((u32)(minlod)) << TX_SETMODE1_MINLOD_SHIFT) | \
         (((u32)(maxlod)) << TX_SETMODE1_MAXLOD_SHIFT) | \
         (((u32)(rid)) << TX_SETMODE1_RID_SHIFT))

//
//  tx_setimage0 register
//
#define TX_SETIMAGE0_WIDTH_SHIFT        0
#define TX_SETIMAGE0_HEIGHT_SHIFT       10
#define TX_SETIMAGE0_FORMAT_SHIFT       20
#define TX_SETIMAGE0_RID_SHIFT          24
#define TX_SETIMAGE0(width, height, format, rid) \
        ((((u32)(width)) << TX_SETIMAGE0_WIDTH_SHIFT) | \
         (((u32)(height)) << TX_SETIMAGE0_HEIGHT_SHIFT) | \
         (((u32)(format)) << TX_SETIMAGE0_FORMAT_SHIFT) | \
         (((u32)(rid)) << TX_SETIMAGE0_RID_SHIFT))

//
//  tx_setimage1 register
//
#define TX_SETIMAGE1_TMEM_OFFSET_SHIFT  0
#define TX_SETIMAGE1_CACHE_WIDTH_SHIFT  15
#define TX_SETIMAGE1_CACHE_HEIGHT_SHIFT 18
#define TX_SETIMAGE1_IMAGE_TYPE_SHIFT   21
#define TX_SETIMAGE1_RID_SHIFT          24
#define TX_SETIMAGE1(tmem_offset, cache_width, cache_height, image_type, rid) \
        ((((u32)(tmem_offset)) << TX_SETIMAGE1_TMEM_OFFSET_SHIFT) | \
         (((u32)(cache_width)) << TX_SETIMAGE1_CACHE_WIDTH_SHIFT) | \
         (((u32)(cache_height)) << TX_SETIMAGE1_CACHE_HEIGHT_SHIFT) | \
         (((u32)(image_type)) << TX_SETIMAGE1_IMAGE_TYPE_SHIFT) | \
         (((u32)(rid)) << TX_SETIMAGE1_RID_SHIFT))

//
//  tx_setimage2 register
//
#define TX_SETIMAGE2_TMEM_OFFSET_SHIFT  0
#define TX_SETIMAGE2_CACHE_WIDTH_SHIFT  15
#define TX_SETIMAGE2_CACHE_HEIGHT_SHIFT 18
#define TX_SETIMAGE2_RID_SHIFT          24
#define TX_SETIMAGE2(tmem_offset, cache_width, cache_height, rid) \
        ((((u32)(tmem_offset)) << TX_SETIMAGE2_TMEM_OFFSET_SHIFT) | \
         (((u32)(cache_width)) << TX_SETIMAGE2_CACHE_WIDTH_SHIFT) | \
         (((u32)(cache_height)) << TX_SETIMAGE2_CACHE_HEIGHT_SHIFT) | \
         (((u32)(rid)) << TX_SETIMAGE2_RID_SHIFT))

//
//  tx_setimage3 register
//
#define TX_SETIMAGE3_IMAGE_BASE_SHIFT   0
#define TX_SETIMAGE3_RID_SHIFT          24
#define TX_SETIMAGE3(image_base, rid) \
        ((((u32)(image_base)) << TX_SETIMAGE3_IMAGE_BASE_SHIFT) | \
         (((u32)(rid)) << TX_SETIMAGE3_RID_SHIFT))

//
//  tx_settlut register
//
#define TX_SETTLUT_TMEM_OFFSET_SHIFT    0
#define TX_SETTLUT_FORMAT_SHIFT         10
#define TX_SETTLUT_RID_SHIFT            24
#define TX_SETTLUT(tmem_offset, format, rid) \
        ((((u32)(tmem_offset)) << TX_SETTLUT_TMEM_OFFSET_SHIFT) | \
         (((u32)(format)) << TX_SETTLUT_FORMAT_SHIFT) | \
         (((u32)(rid)) << TX_SETTLUT_RID_SHIFT))

//
//  su_ts0 struct
//
#define SU_TS0_SSIZE_SHIFT              0
#define SU_TS0_BS_SHIFT                 16
#define SU_TS0_WS_SHIFT                 17
#define SU_TS0_LF_SHIFT                 18
#define SU_TS0_PF_SHIFT                 19
#define SU_TS0_RID_SHIFT                24
#define SU_TS0(ssize, bs, ws, lf, pf, rid) \
        ((((u32)(ssize)) << SU_TS0_SSIZE_SHIFT) | \
         (((u32)(bs)) << SU_TS0_BS_SHIFT) | \
         (((u32)(ws)) << SU_TS0_WS_SHIFT) | \
         (((u32)(lf)) << SU_TS0_LF_SHIFT) | \
         (((u32)(pf)) << SU_TS0_PF_SHIFT) | \
         (((u32)(rid)) << SU_TS0_RID_SHIFT))

//
//  su_ts1 struct
//
#define SU_TS1_TSIZE_SHIFT              0
#define SU_TS1_BT_SHIFT                 16
#define SU_TS1_WT_SHIFT                 17
#define SU_TS1_RID_SHIFT                24
#define SU_TS1(tsize, bt, wt, rid) \
        ((((u32)(tsize)) << SU_TS1_TSIZE_SHIFT) | \
         (((u32)(bt)) << SU_TS1_BT_SHIFT) | \
         (((u32)(wt)) << SU_TS1_WT_SHIFT) | \
         (((u32)(rid)) << SU_TS1_RID_SHIFT))


// These register ID's can also be found in GX run-time arrays.
// However, we need these #define's here for compile-time access.
//
// The following are defined per texture ID
//
#define TX_SETMODE0_I0_ID       0x80
#define TX_SETMODE0_I1_ID       0x81
#define TX_SETMODE0_I2_ID       0x82
#define TX_SETMODE0_I3_ID       0x83
#define TX_SETMODE0_I4_ID       0xa0
#define TX_SETMODE0_I5_ID       0xa1
#define TX_SETMODE0_I6_ID       0xa2
#define TX_SETMODE0_I7_ID       0xa3
                                  
#define TX_SETMODE1_I0_ID       0x84
#define TX_SETMODE1_I1_ID       0x85
#define TX_SETMODE1_I2_ID       0x86
#define TX_SETMODE1_I3_ID       0x87
#define TX_SETMODE1_I4_ID       0xa4
#define TX_SETMODE1_I5_ID       0xa5
#define TX_SETMODE1_I6_ID       0xa6
#define TX_SETMODE1_I7_ID       0xa7
                                  
#define TX_SETIMAGE0_I0_ID      0x88
#define TX_SETIMAGE0_I1_ID      0x89
#define TX_SETIMAGE0_I2_ID      0x8a
#define TX_SETIMAGE0_I3_ID      0x8b
#define TX_SETIMAGE0_I4_ID      0xa8
#define TX_SETIMAGE0_I5_ID      0xa9
#define TX_SETIMAGE0_I6_ID      0xaa
#define TX_SETIMAGE0_I7_ID      0xab
                                  
#define TX_SETIMAGE1_I0_ID      0x8c
#define TX_SETIMAGE1_I1_ID      0x8d
#define TX_SETIMAGE1_I2_ID      0x8e
#define TX_SETIMAGE1_I3_ID      0x8f
#define TX_SETIMAGE1_I4_ID      0xac
#define TX_SETIMAGE1_I5_ID      0xad
#define TX_SETIMAGE1_I6_ID      0xae
#define TX_SETIMAGE1_I7_ID      0xaf
                                  
#define TX_SETIMAGE2_I0_ID      0x90
#define TX_SETIMAGE2_I1_ID      0x91
#define TX_SETIMAGE2_I2_ID      0x92
#define TX_SETIMAGE2_I3_ID      0x93
#define TX_SETIMAGE2_I4_ID      0xb0
#define TX_SETIMAGE2_I5_ID      0xb1
#define TX_SETIMAGE2_I6_ID      0xb2
#define TX_SETIMAGE2_I7_ID      0xb3
                                  
#define TX_SETIMAGE3_I0_ID      0x94
#define TX_SETIMAGE3_I1_ID      0x95
#define TX_SETIMAGE3_I2_ID      0x96
#define TX_SETIMAGE3_I3_ID      0x97
#define TX_SETIMAGE3_I4_ID      0xb4
#define TX_SETIMAGE3_I5_ID      0xb5
#define TX_SETIMAGE3_I6_ID      0xb6
#define TX_SETIMAGE3_I7_ID      0xb7
                                  
#define TX_SETTLUT_I0_ID        0x98
#define TX_SETTLUT_I1_ID        0x99
#define TX_SETTLUT_I2_ID        0x9a
#define TX_SETTLUT_I3_ID        0x9b
#define TX_SETTLUT_I4_ID        0xb8
#define TX_SETTLUT_I5_ID        0xb9
#define TX_SETTLUT_I6_ID        0xba
#define TX_SETTLUT_I7_ID        0xbb

// The following are defined per texcoord

#define SU_SSIZE0_ID            0x30
#define SU_TSIZE0_ID            0x31
#define SU_SSIZE1_ID            0x32
#define SU_TSIZE1_ID            0x33
#define SU_SSIZE2_ID            0x34
#define SU_TSIZE2_ID            0x35
#define SU_SSIZE3_ID            0x36
#define SU_TSIZE3_ID            0x37
#define SU_SSIZE4_ID            0x38
#define SU_TSIZE4_ID            0x39
#define SU_SSIZE5_ID            0x3a
#define SU_TSIZE5_ID            0x3b
#define SU_SSIZE6_ID            0x3c
#define SU_TSIZE6_ID            0x3d
#define SU_SSIZE7_ID            0x3e
#define SU_TSIZE7_ID            0x3f

// Break a word into its constituent bytes.
//
#define B3(word) (u8)(((word) >> 24) & 0xff)
#define B2(word) (u8)(((word) >> 16) & 0xff)
#define B1(word) (u8)(((word) >>  8) & 0xff)
#define B0(word) (u8)(((word) >>  0) & 0xff)

// Break a word or half-word into a comma-seperated byte sequence
//
#define WTB(word) B3(word), B2(word), B1(word), B0(word)
#define HTB(half) B1(half), B0(half)

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main            ( void );

static void CameraInit      ( void );
static void DrawInit        ( void );
static void DrawTick        ( void );

static void AnimTick        ( void );

static void ParameterInit   ( void );

static void SendVertex      ( u16 posIndex, u16 texCoordIndex );

#define SIDE 30
#define NORM (sqrt(3.0))/3.0

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

// Display lists *************************************************************

// Commands to initialize (most of) texture ID 0 and texcoord 0 scale:

u8 InitList[] ATTRIBUTE_ALIGN(32) = 
{
    GX_LOAD_BP_REG,
    //               wps wpt mgf mnf dgl lbi mxa lcl RID
    WTB(TX_SETMODE0(  1,  1,  1,  6,  0,  0,  0,  0, TX_SETMODE0_I0_ID)),
    
    GX_LOAD_BP_REG,
    //                 mnl   mxl  RID
    WTB(TX_SETMODE1(  0<<4, 7<<4, TX_SETMODE1_I0_ID)),

    // TX_SETIMAGE0 (width, height, format) is loaded in the DrawList

    GX_LOAD_BP_REG,
    //                tmem       cw ch type RID
    WTB(TX_SETIMAGE1( 0x00000>>5, 3, 3, 0,  TX_SETIMAGE1_I0_ID)),

    GX_LOAD_BP_REG,
    //                tmem       cw ch  RID
    WTB(TX_SETIMAGE2( 0x80000>>5, 3, 3, TX_SETIMAGE2_I0_ID)),

    // TX_SETIMAGE3 is loaded in the DrawList

    GX_LOAD_BP_REG,
    //              tmem(high)  fmt RID
    WTB(TX_SETTLUT( 0x40000>>5, 1,  TX_SETTLUT_I0_ID)),

    // 0x400000 corresponds to GX_TLUT0's location
    // We don't actually use the CI texture in this demo...
    // We'd have to modify the texture cache locations, and we're too lazy.

    GX_LOAD_BP_REG,
    //          ssz-1 bs ws lf pf RID
    WTB(SU_TS0(  127, 0, 0, 0, 0, SU_SSIZE0_ID)),

    GX_LOAD_BP_REG,
    //          tsz-1 bt wt RID
    WTB(SU_TS1(  127, 0, 0, SU_TSIZE0_ID)),

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // pad to 32B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    // pad to 32B
};

#define INITSIZE sizeof(InitList)

// Commands to select textures and draw the actual geometry:

u8 DrawList[] ATTRIBUTE_ALIGN(32) = 
{
    // face 1

    GX_LOAD_BP_REG,
    //                w-1  h-1  format       RID
    WTB(TX_SETIMAGE0( 127, 127, GX_TF_RGBA8, TX_SETIMAGE0_I0_ID)),

    GX_LOAD_BP_REG,
    //                mem  RID  (will fill in "mem" at run time)
    WTB(TX_SETIMAGE3( 0x0, TX_SETIMAGE3_I0_ID)),

    GX_DRAW_QUADS, HTB(4),
    0, 0, 0,
    1, 1, 1,
    2, 2, 2,
    3, 3, 3,
    
    // face 2

    GX_LOAD_BP_REG,
    //                w-1  h-1  format       RID
    WTB(TX_SETIMAGE0( 127, 127, GX_TF_RGB5A3, TX_SETIMAGE0_I0_ID)),

    GX_LOAD_BP_REG,
    //                mem  RID  (will fill in "mem" at run time)
    WTB(TX_SETIMAGE3( 0x0, TX_SETIMAGE3_I0_ID)),

    GX_DRAW_QUADS, HTB(4),
    4, 4, 0,
    5, 5, 1,
    6, 6, 2,
    7, 7, 3,
          
    // face 3

    GX_LOAD_BP_REG,
    //                w-1  h-1  format       RID
    WTB(TX_SETIMAGE0( 127, 127, GX_TF_CMPR, TX_SETIMAGE0_I0_ID)),

    GX_LOAD_BP_REG,
    //                mem  RID  (will fill in "mem" at run time)
    WTB(TX_SETIMAGE3( 0x0, TX_SETIMAGE3_I0_ID)),

    GX_DRAW_QUADS, HTB(4),
    2, 2, 0,
    6, 6, 1,
    5, 5, 2,
    3, 3, 3,
    
    // face 4

    GX_LOAD_BP_REG,
    //                w-1  h-1  format       RID
    WTB(TX_SETIMAGE0( 127, 127, GX_TF_RGBA8, TX_SETIMAGE0_I0_ID)),

    GX_LOAD_BP_REG,
    //                mem  RID  (will fill in "mem" at run time)
    WTB(TX_SETIMAGE3( 0x0, TX_SETIMAGE3_I0_ID)),

    GX_DRAW_QUADS, HTB(4),
    1, 1, 0,
    0, 0, 1,
    4, 4, 2,
    7, 7, 3,
    
    // face 5

    GX_LOAD_BP_REG,
    //                w-1  h-1  format       RID
    WTB(TX_SETIMAGE0( 127, 127, GX_TF_I4, TX_SETIMAGE0_I0_ID)),

    GX_LOAD_BP_REG,
    //                mem  RID  (will fill in "mem" at run time)
    WTB(TX_SETIMAGE3( 0x0, TX_SETIMAGE3_I0_ID)),

    GX_DRAW_QUADS, HTB(4),
    5, 5, 0,
    4, 4, 1,
    0, 0, 2,
    3, 3, 3,
    
    // face 6

    GX_LOAD_BP_REG,
    //                w-1  h-1  format       RID
    WTB(TX_SETIMAGE0( 127, 127, GX_TF_IA8, TX_SETIMAGE0_I0_ID)),

    GX_LOAD_BP_REG,
    //                mem  RID  (will fill in "mem" at run time)
    WTB(TX_SETIMAGE3( 0x0, TX_SETIMAGE3_I0_ID)),

    GX_DRAW_QUADS, HTB(4),
    6, 6, 0,
    2, 2, 1,
    1, 1, 2,
    7, 7, 3,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // pad to 32B
};

#define DRAWSIZE sizeof(DrawList)

// For size of one face, take off padding, divide by # of faces

#define FACESIZE ((DRAWSIZE - 10)/6)

// This array tells us the offsets for where to set the memory addresses
// for the textures in the DrawList.  We don't know this information until
// runtime, so we need to know which locations to fix up.

u32 DLOffsets[6] =
{
    6+FACESIZE*0,
    6+FACESIZE*1,
    6+FACESIZE*2,
    6+FACESIZE*3,
    6+FACESIZE*4,
    6+FACESIZE*5,
};

// Actual primitive data follows

float FloatVert[] ATTRIBUTE_ALIGN(32) = 
{
    -SIDE,  SIDE, -SIDE,
    -SIDE,  SIDE,  SIDE,
    -SIDE, -SIDE,  SIDE,
    -SIDE, -SIDE, -SIDE,
     SIDE,  SIDE, -SIDE,
     SIDE, -SIDE, -SIDE,
     SIDE, -SIDE,  SIDE,
     SIDE,  SIDE,  SIDE,
};

float FloatNorm[] ATTRIBUTE_ALIGN(32) = 
{
    -1,  1, -1,
    -1,  1,  1,
    -1, -1,  1,
    -1, -1, -1,
     1,  1, -1,
     1, -1, -1,
     1, -1,  1,
     1,  1,  1,
};

float FloatTex[] ATTRIBUTE_ALIGN(32) =  
{
    0.0F, 0.0F, 
    1.0F, 0.0F,
    1.0F, 1.0F, 
    0.0F, 1.0F,
};

Mtx v;          // view matrix
u32 rot;        // current cube rotation

TEXPalettePtr tpl = 0;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
           
    DrawInit();         // Define my vertex formats and set array pointers.

    ParameterInit();

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
    
    MTXFrustum(p, 112, -112, -160, 160, 500, 2000);

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
    u32 i;
    TEXDescriptorPtr tdescp;
    GXTexObj  to0;
    GXTlutObj tlo1;
    u32       data;
    u32       reg;
    static u8 ftt[]={0, 1, 2, 5, 10, 13};  // face-to-texture table, from below
    GXLightObj MyLight;
    GXColor White = {255, 255, 255, 255};

    CameraInit();   // Initialize the camera.

    GXClearVtxDesc();

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    GXSetArray(GX_VA_TEX0, FloatTex, 8);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
    GXSetArray(GX_VA_NRM, FloatNorm, 12);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc (GX_VA_POS, GX_INDEX8);
    GXSetArray(GX_VA_POS, FloatVert, 12);

    TEXGetPalette(&tpl, "gxTests/tex-06.tpl");

    // In tex-06.tpl:
    //
    // index width height format ws wt mnf mgf mnl mxl lb mm face
    //   0    128   128   RGBA8  r  r  l/l  l   0   7   0 Y   1
    //   1    128   128   RGB5A3 r  r  l/l  l   0   7   0 Y   2
    //   2    128   128   CMPR   r  r  l/l  l   0   7   0 Y   3
    //   3    128   128   CI8    r  r  l/l  l   0   7   0 N
    //   4    128   128   RGB565 r  r  l/l  l   0   7   0 Y  
    //   5    128   128   RGBA8  r  r  l/l  l   0   7   0 Y   4
    //   6    128   128   RGBA8  r  r  l/l  l   0   7   0 Y
    //   7    128   128   RGB565 r  r  l/l  l   0   7   0 Y
    //   8    128   128   CMPR   r  r  l/l  l   0   7   0 Y
    //   9    128   128   RGB5A3 r  r  l/l  l   0   7   0 Y
    //  10    128   128   I4     r  r  l/l  l   0   7   0 Y   5
    //  11    128   128   IA4    r  r  l/l  l   0   7   0 Y
    //  12    128   128   I8     r  r  l/l  l   0   7   0 Y
    //  13    128   128   IA8    r  r  l/l  l   0   7   0 Y   6

    // Set up a light
    GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, White);
    GXLoadLightObjImm(&MyLight, GX_LIGHT0);
    GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);

    // Set up TEV
    GXSetNumChans(1);
    GXSetNumTevStages(1);
    GXSetTevOp   (GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    // Fix up normals
    for(i = 0; i < 24; i++)
    {
        FloatNorm[i] *= NORM;
    }

    // Set up texcoord gen
    MTXIdentity(rz);
    GXLoadTexMtxImm(rz, GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);

    // Set ALL texture coordinates to scale manually.
    // This way, GX won't try to do any automatic scaling.
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD2, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD3, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD4, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD5, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD6, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD7, GX_ENABLE, 1, 1);

    // Set up the relevant textures
    //
    // We need to insert their memory addresses into the DrawList.
    //
    for(i = 0; i < 6; i++)
    {
        tdescp = TEXGet(tpl, ftt[i]);
        if(tdescp->CLUTHeader)
        {
            TEXGetGXTexObjFromPaletteCI(tpl, &to0, &tlo1, GX_TLUT0, ftt[i]);
            GXLoadTlut(&tlo1, 0);
        }
        else
        {
            TEXGetGXTexObjFromPalette(tpl, &to0, ftt[i]);
        }
        data = (u32) GXGetTexObjData(&to0);
        data = data >> 5; // data is already a physical address
        reg  = TX_SETIMAGE3( data, TX_SETIMAGE3_I0_ID);
        *(u32 *)(&DrawList[DLOffsets[i]]) = reg;
    }

    // Since we changed it, we need to flush it
    DCFlushRange((void *)DrawList, DRAWSIZE);
    
    // Set up TEX0
    GXCallDisplayList(InitList, INITSIZE);
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
    Mtx ry, rz, mv, t;
    f32 tx, ty;
    
    tx = 0.0f;
    ty = 0.0f;

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

    GXCallDisplayList(DrawList, DRAWSIZE);
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
    u16 buttons = DEMOPadGetButton(0);

    // Just simple controls right now...

    if(buttons & PAD_BUTTON_A)
    {
        // suspend animation
    } else {
    
        rot ++;
        if(rot > 1439)
            rot = 0;
    }
}

/*---------------------------------------------------------------------------*
    Name:           ParameterInit
    
    Description:    Initialize parameters for single frame display              
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void ParameterInit ( void )
{
    rot = 45;
}

/****************************************************************************/


