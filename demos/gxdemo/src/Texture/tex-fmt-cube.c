/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-cube.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-fmt-cube.c $    
    
    4     5/11/01 12:29a Hirose
    removed obsolete flags. fixed some verify messages.
    
    3     3/24/00 4:30p Hirose
    changed to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    7     2/25/00 3:58p Hirose
    updated pad control functions to match actual HW1 gamepad
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    4     1/25/00 2:59p Carl
    Changed to standardized end of test message
    
    3     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    21    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    20    11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    19    10/09/99 12:29a Hirose
    
    18    10/08/99 3:33p Hirose
    removed obsolete data
    
    17    10/05/99 6:25p Hirose
    added single frame test
    changed some structures / deleted C14X2 texture
    
    16    10/04/99 2:05p Tian
    Forced an array into .data section for EPPC build.
    
    15    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    14    9/20/99 11:51a Ryan
    update to use new DEMO calls
    
    13    9/17/99 6:37p Hirose
    changed GXInitTlutObj and GXLoadTlut parameters
    
    12    9/14/99 4:06p Hirose
    checked alignment/coherency
    removed all warnings
    
    11    9/01/99 5:45p Ryan
    
    10    8/29/99 9:41p Hirose
    fixed reversed texture coord
    
    9     8/27/99 3:41p Yasu
    Change the parameter of GXSetZMode() and append GXSetZUpdate()
    
    8     8/17/99 12:02p Alligator
    updated to reflect arbitrary tlut size
    
    7     7/29/99 6:15p Hirose
    Changed to fit to new format set
    
    6     7/23/99 2:55p Ryan
    changed dolphinDemo.h to demo.h
    
    5     7/23/99 12:16p Ryan
    included dolphinDemo.h
    
    4     7/14/99 5:58p Hirose
    
    3     7/13/99 8:32p Hirose
    Supported all texture formats
    
    2     7/13/99 1:21p Hirose
    
    1     7/07/99 3:32p Hirose
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-fmt-cube
        display various format textures ( non-tpl version )
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include "tex-fmt-cube.h"

/*---------------------------------------------------------------------------*
  Macro definitions
 *---------------------------------------------------------------------------*/
#define MAX_SCOORD         0x4000  // for 16b, 2.14 fixed point format
#define MAX_TCOORD         0x4000  // for 16b, 2.14 fixed point format
#define STEP_SCOORD        8
#define STEP_TCOORD        24

#define NON_CI_TEXTURES    8
#define CI_TEXTURES        2
#define MAX_TEXTURES       (NON_CI_TEXTURES + CI_TEXTURES)

#define TEST_TLUT00_NUM_ENTRIES    16
#define TEST_TLUT01_NUM_ENTRIES    256

#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
   Structure definitions
 *---------------------------------------------------------------------------*/
// for camera
typedef struct
{
    Vec    location;
    Vec    up;
    Vec    target;
    f32    left;
    f32    top;
    f32    znear;
    f32    zfar;
} CameraConfig;

typedef struct
{
    CameraConfig  cfg; 
    Mtx           view;
    Mtx44         proj;
} MyCameraObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    Mtx            modelCtrl;
    f32            modelScale; 
    u32            texNumber;
    GXTevMode      tevMode;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main           ( void );
static void DrawInit       ( MySceneCtrlObj* sc );
static void DrawTick       ( MySceneCtrlObj* sc );
static void AnimTick       ( MySceneCtrlObj* sc );
static void DrawBox        ( void );
static void SetCamera      ( MyCameraObj* cam );
static void Message        ( u32 num );
static void PrintIntro     ( void );

/*---------------------------------------------------------------------------*
   Model Data
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   Data arrays for indexed primitives must be 32B aligned.  Normally, memory
   for the arrays would be OSAlloc'd (which returns 32B aligned pointers) and
   the data would be loaded from ROM.  The pragma variable_align provides a
   convenient way to align initialized arrays.
 *---------------------------------------------------------------------------*/
static s16 TopVerts[] ATTRIBUTE_ALIGN(32) = 
{
    -40, -40,  40, // 0
     40, -40,  40, // 1
     40,  40,  40, // 2
    -40,  40,  40, // 3
    -40, -40, -40, // 4
     40, -40, -40, // 5
     40,  40, -40, // 6
    -40,  40, -40  // 7
};

static u8 Colors[] ATTRIBUTE_ALIGN(32) =
{
    0x80, 0x00, 0x00, 0x80,
    0x00, 0x80, 0x00, 0x80,
    0x00, 0x00, 0x80, 0x80,
    0x80, 0x80, 0x00, 0x80,
    0x80, 0x00, 0x80, 0x80,
    0x00, 0x80, 0x80, 0x80
};

/*---------------------------------------------------------------------------*
   Texture settings
 *---------------------------------------------------------------------------*/
static void* TexDataPtr[MAX_TEXTURES] =
{
    TestI4TexImageData,
    TestI8TexImageData,
    TestIA4TexImageData,
    TestIA8TexImageData,
    TestRGB565TexImageData,
    TestRGB5A3TexImageData,
    TestRGBA8TexImageData,
    TestCMPRTexImageData,
    TestC4TexImageData,
    TestC8TexImageData
};

static u32 TexAttr[3*MAX_TEXTURES] =
{
    TEST_I4_TEX_IMAGE_WIDTH,     TEST_I4_TEX_IMAGE_HEIGHT,     TEST_I4_TEX_IMAGE_FORMAT,
    TEST_I8_TEX_IMAGE_WIDTH,     TEST_I8_TEX_IMAGE_HEIGHT,     TEST_I8_TEX_IMAGE_FORMAT,
    TEST_IA4_TEX_IMAGE_WIDTH,    TEST_IA4_TEX_IMAGE_HEIGHT,    TEST_IA4_TEX_IMAGE_FORMAT,
    TEST_IA8_TEX_IMAGE_WIDTH,    TEST_IA8_TEX_IMAGE_HEIGHT,    TEST_IA8_TEX_IMAGE_FORMAT,
    TEST_RGB565_TEX_IMAGE_WIDTH, TEST_RGB565_TEX_IMAGE_HEIGHT, TEST_RGB565_TEX_IMAGE_FORMAT,
    TEST_RGB5A3_TEX_IMAGE_WIDTH, TEST_RGB5A3_TEX_IMAGE_HEIGHT, TEST_RGB5A3_TEX_IMAGE_FORMAT,
    TEST_RGBA8_TEX_IMAGE_WIDTH,  TEST_RGBA8_TEX_IMAGE_HEIGHT,  TEST_RGBA8_TEX_IMAGE_FORMAT,
    TEST_CMPR_TEX_IMAGE_WIDTH,   TEST_CMPR_TEX_IMAGE_HEIGHT,   TEST_CMPR_TEX_IMAGE_FORMAT,
    TEST_C4_TEX_IMAGE_WIDTH,     TEST_C4_TEX_IMAGE_HEIGHT,     TEST_C4_TEX_IMAGE_FORMAT,
    TEST_C8_TEX_IMAGE_WIDTH,     TEST_C8_TEX_IMAGE_HEIGHT,     TEST_C8_TEX_IMAGE_FORMAT
};

static GXTlut TexTlutAttr[CI_TEXTURES] =
{
    GX_TLUT0, GX_TLUT1
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    {   0.0F, 0.0F, 300.0F }, // location
    {   0.0F, 1.0F,   0.0F }, // up
    {   0.0F, 0.0F,   0.0F }, // target
    -32.0F,  // left
    24.0F,   // top
    50.0F,   // near
    2000.0F  // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;    // scene control parameters
static GXTexObj         MyTexObj[MAX_TEXTURES];
static GXTlutObj        MyTlutObj0, MyTlutObj1;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);  // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl); // Initialize vertex formats, array pointers
                          // and default scene settings.

    PrintIntro();    // Print demo directions

    Message(SceneCtrl.texNumber);
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);    // Draw the model.
        DEMODoneRender();
        DEMOPadRead();           // Read controller
        AnimTick(&SceneCtrl);    // Do animation
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format and sets up
                    the array pointer for the indexed data.
                    This function also initializes scene control parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{ 
    u32 i;

    //   Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);   
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 14);

    //   Array Pointers and Strides
    GXInvalidateVtxCache();
    GXSetArray(GX_VA_POS,  TopVerts, 3 * sizeof(s16));
    GXSetArray(GX_VA_CLR0, Colors,   4 * sizeof(u8));

	GXSetNumChans(1);

    // Set texture coordinate generation.
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTexGens(1);
	
    //   Init texture objects, existing in tex-data00.c
    GXInvalidateTexAll();

    for ( i = 0 ; i < NON_CI_TEXTURES ; ++i )
    {
        GXInitTexObj(
            &MyTexObj[i],
            TexDataPtr[i],
            (u16)TexAttr[i*3],
            (u16)TexAttr[i*3+1],
            (GXTexFmt)TexAttr[i*3+2],
            GX_REPEAT, // s
            GX_REPEAT, // t
            GX_FALSE );
    }

    for ( i = NON_CI_TEXTURES ; i < MAX_TEXTURES ; ++i )
    {
        GXInitTexObjCI(
            &MyTexObj[i],
            TexDataPtr[i],
            (u16)TexAttr[i*3],
            (u16)TexAttr[i*3+1],
            (GXCITexFmt)TexAttr[i*3+2],
            GX_REPEAT, // s
            GX_REPEAT, // t
            GX_FALSE,
            (u32)(TexTlutAttr[i - NON_CI_TEXTURES]) );
    }
    
    //  Init TLUTs
    GXInitTlutObj(
        &MyTlutObj0,
        TestTlutData00,
        (GXTlutFmt)TEST_TLUT00_FORMAT,
        TEST_TLUT00_NUM_ENTRIES );
    GXLoadTlut(&MyTlutObj0, GX_TLUT0);

    GXInitTlutObj(
        &MyTlutObj1,
        TestTlutData01,
        (GXTlutFmt)TEST_TLUT01_FORMAT,
        TEST_TLUT01_NUM_ENTRIES );
    GXLoadTlut(&MyTlutObj1, GX_TLUT1);


    // Default scene control parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // model control matrix
    MTXScale(sc->modelCtrl, 1.0F, 1.0F, 1.0F);
    sc->modelScale = 1.0F;
    
    // texture number
    sc->texNumber = 0;
    
    // tev mode
    sc->tevMode = GX_REPLACE;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  ms; // Scaling matrix.
    Mtx  mv; // Modelview matrix.

    // set tev operation (Decal/Replace)
    GXSetTevOp(GX_TEVSTAGE0, sc->tevMode);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    
    // set texture
    GXLoadTexObj(&MyTexObj[sc->texNumber], GX_TEXMAP0);

    // set modelview matrix
    MTXScale(ms, sc->modelScale, sc->modelScale, sc->modelScale);
    MTXConcat(sc->cam.view, ms, mv); 
    MTXConcat(mv, sc->modelCtrl, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    DrawBox();
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    u16  down;
    Mtx  mrx, mry;

    // PAD
    down = DEMOPadGetButtonDown(0);

    // Model rotation calculation
    MTXRotDeg(mry, 'x', -(f32)(DEMOPadGetStickY(0))/32.0F);
    MTXRotDeg(mrx, 'y', (f32)(DEMOPadGetStickX(0))/32.0F);
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);

    // Model scale control
    sc->modelScale += ( DEMOPadGetSubStickY(0) / 32 ) * 0.005F;
    Clamp(sc->modelScale, 0.1F, 2.0F);


    // Texture select
    if ( down & PAD_BUTTON_A )
    {
        sc->texNumber = ( sc->texNumber + 1 ) % MAX_TEXTURES;
        Message(sc->texNumber);
    }

    // Tev mode control
    if ( down & PAD_BUTTON_B )
    {
        sc->tevMode = ( sc->tevMode == GX_REPLACE ) ?
                      GX_DECAL : GX_REPLACE;
    }
}

/*---------------------------------------------------------------------------*
    Name:           VertexT
    
    Description:    Draw a vertex with texture, direct data
                    
    Arguments:      v        8-bit position index
                    s, t     16-bit tex coord
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void VertexT( u8 v, u32 s, u32 t, u8 c )
{
    GXPosition1x8(v);
    GXColor1x8(c);
    GXTexCoord2u16((u16)s, (u16)t);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTexQuad
    
    Description:    Draw a textured quad.  Map extends to corners of the quad.
                    MAX_SCOORD is the value of 1.0 in the fixed point format.

    Arguments:      v0        8-bit position
                    v1        8-bit position
                    v2        8-bit position
                    v3        8-bit position
                    s0        s tex coord at v0
                    t0        t tex coord at v0
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void DrawTexQuad(
    u8  v0,
    u8  v1,
    u8  v2,
    u8  v3,
    u32 s0,
    u32 t0,
    u8  c ) 
{
    VertexT(v0, s0+MAX_SCOORD, t0, c);
    VertexT(v1, s0+MAX_SCOORD, t0+MAX_TCOORD, c);
    VertexT(v2, s0, t0+MAX_TCOORD, c);
    VertexT(v3, s0, t0, c);
}

/*---------------------------------------------------------------------------*
    Name:           DrawBox
    
    Description:    Draw box model.
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawBox( void )
{
    static u32 s = 0;
    static u32 t = 0;
    
    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // draw the box
    GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);
        DrawTexQuad(3, 2, 1, 0, s, t, 0);
        DrawTexQuad(4, 5, 6, 7, s, s, 1);
        DrawTexQuad(4, 7, 3, 0, 0, 0, 2);
        DrawTexQuad(5, 4, 0, 1, t, t, 3);
        DrawTexQuad(6, 5, 1, 2, t, 0, 4);
        DrawTexQuad(7, 6, 2, 3, 0, s, 5);
    GXEnd();

    // translate s, t coordinates
    s += STEP_SCOORD; 
    t += STEP_TCOORD;
    if (s  > MAX_SCOORD) s = 0;
    if (t  > MAX_TCOORD) t = 0;

}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    set view matrix and load projection matrix into hardware
                    
    Arguments:      cam : pointer to the MyCameraObj structure
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetCamera( MyCameraObj* cam )
{
    MTXLookAt(
        cam->view,
        &cam->cfg.location,
        &cam->cfg.up,
        &cam->cfg.target );    

    MTXFrustum(
        cam->proj,
        cam->cfg.top,
        - (cam->cfg.top),
        cam->cfg.left,
        - (cam->cfg.left),
        cam->cfg.znear,
        cam->cfg.zfar );
    GXSetProjection(cam->proj, GX_PERSPECTIVE);
}

/*---------------------------------------------------------------------------*
    Name:           Message
    
    Description:    Output message for debugging
                    
    Arguments:      num : current texture number
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void Message( u32 num )
{
    switch(num)
    {
        case 0  : OSReport("I4 Format\n");            break;
        case 1  : OSReport("I8 Format\n");            break;
        case 2  : OSReport("IA4 Format\n");           break;
        case 3  : OSReport("IA8 Format\n");           break;
        case 4  : OSReport("RGB565 Format\n");        break;
        case 5  : OSReport("RGB5A3 Format\n");        break;
        case 6  : OSReport("RGBA8 Format\n");         break;
        case 7  : OSReport("Compressed Format\n");    break;
        case 8  : OSReport("C4 Format\n");            break;
        case 9  : OSReport("C8 Format\n");            break;
        case 10 : OSReport("C14X2 Format\n");         break;
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
    OSReport("******************************************************\n");
    OSReport("tex-fmt-cube: various format test (non-tpl version)\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick  : rotate the box.\n");
    OSReport("Sub Stick Y : change scale of the box.\n");
    OSReport("A button    : change the texture.\n");
    OSReport("B button    : change tev mode.\n");
    OSReport("******************************************************\n");
    OSReport("\n");
}

/*============================================================================*/
