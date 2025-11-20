/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-filter.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-filter.c $    
    
    5     5/11/01 12:29a Hirose
    removed obsolete flags. fixed some verify messages.
    
    4     3/24/00 4:30p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    7     2/25/00 3:58p Hirose
    updated pad control functions to match actual HW1 gamepad
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     1/21/00 5:46p Hirose
    removed #ifdef EPPC code
    
    4     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    3     1/13/00 5:55p Alligator
    integrate with ArtX GX library code
    
    2     12/24/99 3:43p Hirose
    fixed a small bug
    
    17    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    16    11/15/99 4:49p Yasu
    Change datafile name
    
    15    11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    14    10/18/99 11:30a Hirose
    added mipmap on/off control
    
    13    10/13/99 4:32p Alligator
    change GXSetViewport, GXSetScissor to use xorig, yorig, wd, ht
    
    12    10/12/99 10:19a Hirose
    a small bug fix
    
    11    10/07/99 10:08a Hirose
    added single frame test
    
    10    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    9     9/20/99 11:51a Ryan
    update to use new DEMO calls
    
    8     9/14/99 4:24p Hirose
    checked alignment/coherency
    removed all warnings
    turned to use GXSetViewport() to split objects
    
    7     9/01/99 5:45p Ryan
    
    6     8/27/99 3:41p Yasu
    Change the parameter of GXSetZMode() and append GXSetZUpdate()
    
    5     8/13/99 4:04p Hirose
    changed Tpl file location
    
    4     8/04/99 8:36p Hirose
    
    3     8/04/99 2:21p Hirose
    Changed to use TPL
    
    2     7/30/99 6:13p Hirose
    
    1     7/29/99 6:28p Hirose
    Created

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-filter
        texture bilinear/trilinear filter test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define NUM_VIEWS          4

#define MAX_SCOORD         0x4000  // for 16b, 2.14 fixed point format
#define MAX_TCOORD         0x4000  // for 16b, 2.14 fixed point format

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

// for viewports
typedef struct
{
    s32    xorig;
    s32    yorig;
    s32    width;
    s32    height;
} MyViewPortObj;

// for entire scene control
typedef struct
{
    MyCameraObj       cam;
    u32               texNumber;
    GXBool            mipMapSw;
    Mtx               modelCtrl;
    f32               modelScale;
    MyViewPortObj     viewPorts[NUM_VIEWS]; 
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main           ( void );
static void DrawInit       ( MySceneCtrlObj* sc );
static void DrawTick       ( MySceneCtrlObj* sc );
static void AnimTick       ( MySceneCtrlObj* sc );
static void DrawCube       ( void );
static void SetCamera      ( MyCameraObj* cam );
static void SetViewPort    ( MyViewPortObj* vp );
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
static s16 CubeVertices[] ATTRIBUTE_ALIGN(32) = 
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

static u16 CubeTexCoords[4][2] = // This is not used as array.
{
    { 0x0000,     0x0000 },
    { MAX_SCOORD, 0x0000 },
    { MAX_SCOORD, MAX_TCOORD },
    { 0x0000,     MAX_TCOORD }
};

static s16 CubeTexSpeed[6][2] =
{
    { 2,  3 },
    { 3,  2 },
    { 0,  0 },
    { 2,  0 },
    { 1,  1 },
    { 0,  3 }
};

static u8 CubeFaces[6][4] =
{
    { 3, 2, 1, 0 },
    { 4, 5, 6, 7 },
    { 4, 7, 3, 0 },
    { 5, 4, 0, 1 },
    { 6, 5, 1, 2 },
    { 7, 6, 2, 3 }
};

/*---------------------------------------------------------------------------*
   Settings for each view
 *---------------------------------------------------------------------------*/
// Viewport origin for each view
static u32 ViewOrigX[NUM_VIEWS] = { 0, 1, 0, 1 };
static u32 ViewOrigY[NUM_VIEWS] = { 0, 0, 1, 1 };

// Mipmap filter for each cube
static GXTexFilter Filters[NUM_VIEWS][2] =
{
    { GX_NEAR_MIP_NEAR, GX_NEAR },
    { GX_NEAR_MIP_LIN,  GX_NEAR },
    { GX_LIN_MIP_NEAR,  GX_LINEAR },
    { GX_LIN_MIP_LIN,   GX_LINEAR }
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    {   0.0F, -500.0F, 0.0F }, // location
    {   0.0F,    0.0F, 1.0F }, // up
    {   0.0F,    0.0F, 0.0F }, // target
    -160.0F,  // left
    120.0F,   // top
    50.0F,    // near
    2000.0F   // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;                // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;          // texture palette
static u32              TexNumMax;                // number of textures

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);  // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl); // Initialize vertex formats, array pointers
                          // and default scene settings.

    PrintIntro();    // Print demo directions
   
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);        // Draw the model.
        DEMODoneRender();
        DEMOPadRead();               // Read controller
        AnimTick(&SceneCtrl);   // Do animation
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
    GXRenderModeObj* rmode;
    u32    swd, sht;
    u32    i;

    //  Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 14);

    //  Array Pointers and Strides
    GXInvalidateVtxCache();
    GXSetArray(GX_VA_POS, CubeVertices, 3 * sizeof(s16));

    //  Load TPL file
    TEXGetPalette(&MyTplObj, "gxTests/tex-01.tpl");
    TexNumMax = MyTplObj->numDescriptors;

    // Set texture coordinate generation.
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTexGens(1);
	
	
    // Default scene control parameter settings
    
    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // viewports
    rmode = DEMOGetRenderModeObj();
    swd   = rmode->fbWidth;    // Screen Width
    sht   = rmode->efbHeight;  // Screen Height
    for ( i = 0 ; i < NUM_VIEWS ; ++i )
    {
        sc->viewPorts[i].xorig  = (s32)(ViewOrigX[i] * (swd / 2));
        sc->viewPorts[i].yorig  = (s32)(ViewOrigY[i] * (sht / 2));
        sc->viewPorts[i].width  = (s32)(swd / 2);
        sc->viewPorts[i].height = (s32)(sht / 2);
    }

    // texture number
    sc->texNumber = 0;
    
    // mipmap switch
    sc->mipMapSw = GX_TRUE;

    // model rotation and scale
    sc->modelScale = 1.0F;
    MTXIdentity(sc->modelCtrl);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    u32              i;
    TEXDescriptorPtr tdp;
    GXTexObj         texObj;
    GXBool           mipMapFlag;
    Mtx              ms, mv;
    f32              minLOD, maxLOD;

    // set tev mode to use texture
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    

    // get texture descriptor from texture palette
    tdp = TEXGet(MyTplObj, sc->texNumber);

    mipMapFlag =
        ( tdp->textureHeader->minLOD == tdp->textureHeader->maxLOD ) ?
        GX_FALSE : GX_TRUE;

    minLOD = tdp->textureHeader->minLOD;
    maxLOD = tdp->textureHeader->maxLOD;

    if ( sc->mipMapSw == GX_FALSE )
    {
        mipMapFlag = GX_FALSE;
        maxLOD     = minLOD;
    }

    GXInitTexObj(
        &texObj,
        tdp->textureHeader->data,
        tdp->textureHeader->width,
        tdp->textureHeader->height,
        (GXTexFmt)tdp->textureHeader->format,
        tdp->textureHeader->wrapS, // s
        tdp->textureHeader->wrapT, // t
        mipMapFlag );     // Mipmap

    // Set modelview matrix
    MTXConcat(sc->cam.view, sc->modelCtrl, mv); 
    MTXScale(ms, sc->modelScale, sc->modelScale, sc->modelScale);
    MTXConcat(mv, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);


    // Draw each view
    for ( i = 0 ; i < NUM_VIEWS ; ++i )
    {
        // Viewport
        SetViewPort(&sc->viewPorts[i]);
        
        // Filter setting
        GXInitTexObjLOD(
            &texObj,
            mipMapFlag ? Filters[i][0] : Filters[i][1],
            Filters[i][1],
            minLOD,
            maxLOD,
            tdp->textureHeader->LODBias,
            GX_FALSE,
            tdp->textureHeader->edgeLODEnable,
            GX_ANISO_1 );

        // Draw a textured cube
        GXLoadTexObj(&texObj, GX_TEXMAP0);
        DrawCube();
    }
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
    f32  dscale;
    Mtx  mrx, mry;
    
    down = DEMOPadGetButtonDown(0);

    // Model Rotation Calculation   
    MTXRotDeg(mry, 'x', -(f32)(DEMOPadGetStickY(0))/32.0F);
    MTXRotDeg(mrx, 'z',  (f32)(DEMOPadGetStickX(0))/32.0F);
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);

    // Model scale control
    dscale = 1.0F + (DEMOPadGetSubStickY(0) / 32 ) * 0.0125F;
    sc->modelScale *= dscale;
    Clamp(sc->modelScale, 0.05F, 4.0F);

    // Change the texture
    if ( down & PAD_BUTTON_A )
    {
        sc->texNumber = ( sc->texNumber + 1 ) % TexNumMax;
    }

    // Turn on/off mipmap switch
    if ( down & PAD_BUTTON_B )
    {
        if ( sc->mipMapSw == GX_TRUE )
        {
            sc->mipMapSw = GX_FALSE;
            OSReport("Mipmap off\n");
        }
        else
        {
            sc->mipMapSw = GX_TRUE;
            OSReport("Mipmap on\n");
        }
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawCube
    
    Description:    Draw a textured cube. 
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawCube( void )
{
    static u32 s = 0;
    static u32 t = 0;
    u16  cs, ct;
    u8   i, j;

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // draw a cube
    GXBegin(GX_QUADS, GX_VTXFMT0, 24);
    for ( i = 0 ; i < 6 ; ++i )
    {
        // Draw a face
        for ( j = 0 ; j < 4 ; ++j )
        {
            cs = (u16)(CubeTexCoords[j][0] + s * CubeTexSpeed[i][0]);
            ct = (u16)(CubeTexCoords[j][1] + t * CubeTexSpeed[i][1]);
            GXPosition1x8(CubeFaces[i][j]);
            GXTexCoord2u16(cs, ct);
        }
    }
    GXEnd();

    // translate s, t coordinates
    ++s;
    ++t;
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

    MTXOrtho(
        cam->proj,
        cam->cfg.top,
        - (cam->cfg.top),
        cam->cfg.left,
        - (cam->cfg.left),
        cam->cfg.znear,
        cam->cfg.zfar );
    GXSetProjection(cam->proj, GX_ORTHOGRAPHIC);
}

/*---------------------------------------------------------------------------*
    Name:           SetViewPort
    
    Description:    set viewports
                    
    Arguments:      vp : pointer to the MyViewPortObj structure
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetViewPort( MyViewPortObj* vp )
{
    GXSetViewport(
        (f32)vp->xorig,
        (f32)vp->yorig,
        (f32)vp->width,
        (f32)vp->height,
        0.0F,
        1.0F );
    GXSetScissor(
        (u32)vp->xorig,
        (u32)vp->yorig,
        (u32)vp->width,
        (u32)vp->height );
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
    OSReport("tex-filter: texture filter mode test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("  Main Stick   : rotate the box\n");
    OSReport("  Sub Stick Y  : change scale of the box\n");
    OSReport("  A button     : change the texture\n");
    OSReport("  B button     : mipmap on/off\n");
    OSReport("Descriptions:\n");
    OSReport("  Upper Left   : Near/Near Filter\n");
    OSReport("  Upper Right  : Near/Linear Filter\n");
    OSReport("  Lower Left   : Linear/Near Filter\n");
    OSReport("  Lower Right  : Linear/Linear Filter\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
