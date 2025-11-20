/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-lod.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-lod.c $    
    
    5     5/11/01 1:00a Hirose
    removed obsolete flags. fixed some verify messages.
    
    4     3/24/00 4:57p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    7     2/25/00 3:23p Hirose
    updated pad control functions to match actual HW1 gamepad
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     1/21/00 5:49p Hirose
    removed #ifdef EPPC code
    
    4     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    3     1/11/00 2:46p Hirose
    fixed a bug at singleframe test
    
    2     1/11/00 10:18a Hirose
    updated something
    
    1     1/05/00 6:51p Hirose
    initial version
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-lod
        advanced LOD filters test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define NUM_VIEWS          4
#define NUM_PARAMETERS     4

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
    CameraConfig   cfg;
    Mtx            view;
    Mtx44          proj;
} MyCameraObj;

// for viewports
typedef struct
{
    s32    xorig;
    s32    yorig;
    s32    width;
    s32    height;
} MyViewPortObj;

// for subscene in a view 
typedef struct
{
    s32            filterCtrl[NUM_PARAMETERS];
    Mtx            modelCtrl;
    f32            modelScale;
    MyViewPortObj  viewPort;
} MySubSceneObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    u32            texNumber;
    MySubSceneObj  subScene[NUM_VIEWS];
    u32            curV;
    u32            curP;
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
static void PrintIntro     ( void );

/*---------------------------------------------------------------------------*
   Model Data
 *---------------------------------------------------------------------------*/
static s16 CubeVertices[3*8] ATTRIBUTE_ALIGN(32) = 
{
    -64, -64,  64, // 0
     64, -64,  64, // 1
     64,  64,  64, // 2
    -64,  64,  64, // 3
    -64, -64, -64, // 4
     64, -64, -64, // 5
     64,  64, -64, // 6
    -64,  64, -64  // 7
};

static s16 CubeTexCoords[2*4] ATTRIBUTE_ALIGN(32) =
{
      0,   0,
      1,   0,
      1,   1,
      0,   1
};

static u8 CubeFaces[6][4] =
{
    { 1, 0, 3, 2 },
    { 6, 7, 4, 5 },
    { 3, 0, 4, 7 },
    { 0, 1, 5, 4 },
    { 1, 2, 6, 5 },
    { 2, 3, 7, 6 }
};

/*---------------------------------------------------------------------------*
   Data for views and parameters
 *---------------------------------------------------------------------------*/
// Viewport origin for each view
static u32 ViewOrigX[NUM_VIEWS] = { 0, 1, 0, 1 };
static u32 ViewOrigY[NUM_VIEWS] = { 0, 0, 1, 1 };

// Max/Min value of parameters
static s32 ParamMax[NUM_PARAMETERS] = {  7, 1, 1, 2 };
static s32 ParamMin[NUM_PARAMETERS] = { -7, 0, 0, 0 };

// Anisotropic filter mode
static u32 AnisoNum[3] = { 1, 2, 4 };
static GXAnisotropy AnisoMode[3] =
{
    GX_ANISO_1,
    GX_ANISO_2,
    GX_ANISO_4
};

// Strings for captions
static char* SwStr[2] = { "OFF", "ON" };

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
static MySceneCtrlObj   SceneCtrl;          // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;    // texture palette
static u32              TexNumMax;          // number of textures

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
    GXRenderModeObj* rmode;
    f32    swd, sht;
    u32    i, j;

    // Vertex Attribute ( VTXFMT0 is used by DEMOPuts lib.)
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_S16, 0);

    // Array Pointers and Strides
    GXInvalidateVtxCache();
    GXSetArray(GX_VA_POS,  CubeVertices, 3 * sizeof(s16));
    GXSetArray(GX_VA_TEX0, CubeTexCoords, 2 * sizeof(s16));

    // Load TPL file
    TEXGetPalette(&MyTplObj, "gxTests/tex-01.tpl");
    TexNumMax = MyTplObj->numDescriptors;

    // Get Screen Information
    rmode = DEMOGetRenderModeObj();
    swd   = rmode->fbWidth;    // Screen Width
    sht   = rmode->efbHeight;  // Screen Height


    // Default scene control parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);
    
    // texture number
    sc->texNumber = 0;
    
    // each subscene objects
    for ( i = 0 ; i < NUM_VIEWS ; ++i )
    {
        // viewport
        sc->subScene[i].viewPort.xorig  = (s32)(ViewOrigX[i] * (swd / 2));
        sc->subScene[i].viewPort.yorig  = (s32)(ViewOrigY[i] * (sht / 2));
        sc->subScene[i].viewPort.width  = (s32)(swd / 2);
        sc->subScene[i].viewPort.height = (s32)(sht / 2);

        // filters
        for ( j = 0 ; j < NUM_PARAMETERS ; ++j )
        {
            sc->subScene[i].filterCtrl[j] = 0;
        }
        
        // model
        sc->subScene[i].modelScale = 1.0F;
        MTXIdentity(sc->subScene[i].modelCtrl);
    }
    
    // cursor position
    sc->curV = 0;
    sc->curP = 0;
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
    MySubSceneObj*   ssc;
    GXTexObj         texObj;
    f32              lodBias;
    GXBool           biasClamp, edgeLod;
    GXAnisotropy     aniso;
    Mtx              ms, mv;

    // get texture descriptor from texture palette
    tdp = TEXGet(MyTplObj, sc->texNumber);
    GXInitTexObj(
        &texObj,
        tdp->textureHeader->data,
        tdp->textureHeader->width,
        tdp->textureHeader->height,
        (GXTexFmt)tdp->textureHeader->format,
        tdp->textureHeader->wrapS, // s
        tdp->textureHeader->wrapT, // t
        GX_TRUE );     // Mipmap always enabled

    // Draw each view
    for ( i = 0 ; i < NUM_VIEWS ; ++i )
    {
        ssc = &sc->subScene[i];
        
        // Viewport/Scissor
        GXSetViewport(
            (f32)ssc->viewPort.xorig,
            (f32)ssc->viewPort.yorig,
            (f32)ssc->viewPort.width,
            (f32)ssc->viewPort.height,
            0.0F,
            1.0F );
        GXSetScissor(
            (u32)ssc->viewPort.xorig,
            (u32)ssc->viewPort.yorig,
            (u32)ssc->viewPort.width,
            (u32)ssc->viewPort.height );

        // Camera
        SetCamera(&sc->cam);

        // Modelview matrix
        MTXConcat(sc->cam.view, ssc->modelCtrl, mv); 
        MTXScale(ms, ssc->modelScale, ssc->modelScale, ssc->modelScale);
        MTXConcat(mv, ms, mv);
        
        // Tev, TexGen and Zmode
        GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);

        // LOD filter settings
        lodBias   = (f32)(ssc->filterCtrl[0]) / 2.0F;
        biasClamp = (GXBool)(ssc->filterCtrl[1]);
        edgeLod   = (GXBool)(ssc->filterCtrl[2]);
        aniso     = AnisoMode[ssc->filterCtrl[3]];
        
        GXInitTexObjLOD(
            &texObj,
            GX_LIN_MIP_LIN,
            GX_LINEAR,
            tdp->textureHeader->minLOD,
            tdp->textureHeader->maxLOD,
            lodBias,     // LOD bias
            biasClamp,   // bias clamp
            edgeLod,     // edge LOD enable
            aniso );     // anisotropy
        
        // Draw a textured cube
        GXLoadTexObj(&texObj, GX_TEXMAP0);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
        DrawCube();
        
        // Show parameters by caption
        DEMOInitCaption(
            DM_FT_OPQ,
            (s32)ssc->viewPort.width,
            (s32)ssc->viewPort.height );
      
        DEMOPrintf(0,  0, 0, " LOD Bias  :%1.1f", lodBias);
        DEMOPrintf(0,  8, 0, " Bias Clamp:%s",
                   SwStr[ssc->filterCtrl[1]] );
        DEMOPrintf(0, 16, 0, " Edge LOD  :%s",
                   SwStr[ssc->filterCtrl[2]] );
        DEMOPrintf(0, 24, 0, " Max Aniso.:%d",
                   AnisoNum[ssc->filterCtrl[3]] );
        
        // Draw cursor
        if ( i == sc->curV )
        {
            DEMOPrintf(0, (s16)(sc->curP * 8), 0, "%c", 0x7F);
        }
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
    u16  button, down;
    MySubSceneObj* ssc;
    Mtx  mrx, mry;
    f32  dscale;
    u32  i;
    
    // PAD
    down   = DEMOPadGetButtonDown(0);
    button = DEMOPadGetButton(0);

    ssc = &sc->subScene[sc->curV];

    // Model Rotation Calculation
    MTXRotDeg(mry, 'x', -(DEMOPadGetStickY(0) / 24));
    MTXRotDeg(mrx, 'z',  (DEMOPadGetStickX(0) / 24));
    MTXConcat(mry, ssc->modelCtrl, ssc->modelCtrl);
    MTXConcat(mrx, ssc->modelCtrl, ssc->modelCtrl);

    // Model scale control
    dscale = 1.0F + ( DEMOPadGetSubStickY(0) / 16 ) * 0.025F;
    ssc->modelScale *= dscale;
    Clamp(ssc->modelScale, 0.05F, 3.0F);


    // Synchronize scale and rotation
    if ( button & PAD_BUTTON_A )
    {
        for ( i = 0 ; i < NUM_VIEWS ; ++i )
        {
            if ( i == sc->curV )
                continue;
            
            sc->subScene[i].modelScale = ssc->modelScale;
            MTXCopy(ssc->modelCtrl, sc->subScene[i].modelCtrl);
        }
    }

    // Change the texture
    if ( down & PAD_BUTTON_B )
    {
        sc->texNumber = ( sc->texNumber + 1 ) % TexNumMax;
    }

    // Change parameter
    if ( down & PAD_TRIGGER_L )
    {
        if ( ssc->filterCtrl[sc->curP] > ParamMin[sc->curP] )
        {
            ssc->filterCtrl[sc->curP] -= 1;
        }
    }
    if ( down & PAD_TRIGGER_R )
    {
        if ( ssc->filterCtrl[sc->curP] < ParamMax[sc->curP] )
        {
            ssc->filterCtrl[sc->curP] += 1;
        }
    }

    // Move cursor
    if ( down & PAD_BUTTON_Y )
    {
        sc->curP = ( sc->curP + NUM_PARAMETERS - 1 ) % NUM_PARAMETERS;
        if ( sc->curP == NUM_PARAMETERS - 1 )
        {
            sc->curV = ( sc->curV + NUM_VIEWS - 1 ) % NUM_VIEWS;
        }
    }
    if ( down & PAD_BUTTON_X )
    {
        sc->curP = ( sc->curP + 1 ) % NUM_PARAMETERS;
        if ( sc->curP == 0 )
        {
            sc->curV = ( sc->curV + 1 ) % NUM_VIEWS;
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
    u8   i, j;

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);

    // draw a cube
    GXBegin(GX_QUADS, GX_VTXFMT1, 24);
    for ( i = 0 ; i < 6 ; ++i )
    {
        // Draw a face
        for ( j = 0 ; j < 4 ; ++j )
        {
            GXPosition1x8(CubeFaces[i][j]);
            GXTexCoord1x8(j);
        }
    }
    GXEnd();
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
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("tex-lod: advanced LOD filter test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick   : rotate the box\n");
    OSReport("Sub Stick Y  : change scale of the box\n");
    OSReport("X/Y Buttons  : move cursor\n");
    OSReport("L/R Triggers : change parameters\n");
    OSReport("A button     : synchronize scale and rotation\n");
    OSReport("B button     : change the texture\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
