/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-mix-mode.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-mix-mode.c $
    
    7     5/11/01 1:00a Hirose
    removed obsolete flags. fixed some verify messages.
    
    6     8/17/00 11:12p Hirose
    corrected the location of the GXTexModeSync call, etc.
    
    5     4/26/00 4:54p Carl
    CallBack -> Callback
    
    4     3/24/00 4:57p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    6     2/25/00 3:23p Hirose
    updated pad control functions to match actual HW1 gamepad
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    3     1/13/00 5:55p Alligator
    integrate with ArtX GX library code
    
    2     1/06/00 6:50p Hirose
    added GXTexModeSync call
    
    1     1/04/00 10:02a Hirose
    initial version
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-mix-mode
        mixture of cached / preloaded textures test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define NUM_TEXTURES    16
#define NUM_REGIONS     8

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

// for texture (includes region control)
typedef struct
{
    u32           numPreload;
    u32           numGen0;
    u32           numGen1;
    GXTexObj      texObjs[NUM_TEXTURES];
    GXTexRegion   texRegions[NUM_REGIONS];
} MyTexCtrlObj;

// for entire scene control
typedef struct
{
    MyCameraObj   cam;
    s32           modelRot;
    s32           duplication;
    MyTexCtrlObj  texCtrl;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main               ( void );
static void DrawInit           ( MySceneCtrlObj* sc );
static void DrawTick           ( MySceneCtrlObj* sc );
static void AnimTick           ( MySceneCtrlObj* sc );
static void DrawCube           ( MyTexCtrlObj* tco );
static void LoadOneTexture     ( MyTexCtrlObj* tco );
static void InitTexNumGen      ( MyTexCtrlObj* tco );
static void ConfigureTmemMap   ( MyTexCtrlObj* tco );
static void SetCamera          ( MyCameraObj* cam );
static void StatusMessage      ( u32 numPreload );
static void PrintIntro         ( void );
static GXTexRegion* 
            MyTexRegionCallback( GXTexObj* texObj, GXTexMapID mapID );

/*---------------------------------------------------------------------------*
   Model Data
 *---------------------------------------------------------------------------*/
static s16 CubeVertices[3*8] ATTRIBUTE_ALIGN(32) = 
{
    -32, -32,  32, // 0
     32, -32,  32, // 1
     32,  32,  32, // 2
    -32,  32,  32, // 3
    -32, -32, -32, // 4
     32, -32, -32, // 5
     32,  32, -32, // 6
    -32,  32, -32  // 7
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
    { 3, 2, 1, 0 },
    { 4, 5, 6, 7 },
    { 4, 7, 3, 0 },
    { 5, 4, 0, 1 },
    { 6, 5, 1, 2 },
    { 7, 6, 2, 3 }
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 500.0F }, // location
    { 0.0F, 1.0F,   0.0F }, // up
    { 0.0F, 0.0F,   0.0F }, // tatget
    -320.0F, // left
    240.0F,  // top
    50.0F,   // near
    2000.0F  // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;          // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;    // texture palette

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);  // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl); // Initialize vertex formats, array pointers
                          // and default scene settings.

    PrintIntro();    // Print demo directions

    StatusMessage(SceneCtrl.texCtrl.numPreload);
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
    TEXDescriptorPtr  tdp;
    GXBool            mipMapFlag;
    u32               i;
    
    // Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 0);

    // Array pointers
    GXSetArray(GX_VA_POS,  CubeVertices, 3 * sizeof(s16));
    GXSetArray(GX_VA_TEX0, CubeTexCoords, 2 * sizeof(s16));

    // Texture coords
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    // Color channels
    GXSetNumChans(0);

    // Use user-defined tex region Callback
    GXSetTexRegionCallback(MyTexRegionCallback);

    // Load TPL file and initialize texture objects
    TEXGetPalette(&MyTplObj, "gxTests/tex-07.tpl");
    for ( i = 0 ; i < NUM_TEXTURES ; ++i )
    {
        tdp = TEXGet(MyTplObj, i);
        mipMapFlag =
            ( tdp->textureHeader->minLOD == tdp->textureHeader->maxLOD ) ?
            GX_FALSE : GX_TRUE;
        
        GXInitTexObj(
            &sc->texCtrl.texObjs[i],
            tdp->textureHeader->data,
            tdp->textureHeader->width,
            tdp->textureHeader->height,
            (GXTexFmt)tdp->textureHeader->format,
            tdp->textureHeader->wrapS, // s
            tdp->textureHeader->wrapT, // t
            mipMapFlag ); // Mipmap
        
        GXInitTexObjLOD(
            &sc->texCtrl.texObjs[i],
            tdp->textureHeader->minFilter,
            tdp->textureHeader->magFilter,
            tdp->textureHeader->minLOD,
            tdp->textureHeader->maxLOD,
            tdp->textureHeader->LODBias,
            GX_FALSE,
            tdp->textureHeader->edgeLODEnable,
            GX_ANISO_1 );
    }


    // Default scene control parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // number of preload textures
    sc->texCtrl.numPreload = 0;
    
    // number of drawing duplications
    sc->duplication = 1;
    
    // model rotation parameter
    sc->modelRot = 0;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mt, mv, mr;
    s32  x, y, z;

    // configure tmem map
    ConfigureTmemMap(&sc->texCtrl);

    // use texture in Tev
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

    // common rotation matrix
    MTXRotDeg(mv, 'y', sc->modelRot);
    MTXRotDeg(mr, 'x', sc->modelRot * 2);
    MTXConcat(mv, mr, mr);

    // initialize tex number generator
    InitTexNumGen(&sc->texCtrl);

    for ( z = sc->duplication ; z > 0 ; --z )
    {
        // display 8x6 cubes
        for ( y = 0 ; y < 6 ; ++y )
        {
            for ( x = 0 ; x < 8 ; ++x )
            {
                // Set modelview matrix
                MTXTrans( mt,
                          (f32)(x * 80 + z * 5 - 280),
                          (f32)(y * 80 + z * 5 - 200),
                          (f32)(z * -100) );
                MTXConcat(sc->cam.view, mt, mv);
                MTXConcat(mv, mr, mv);
                GXLoadPosMtxImm(mv, GX_PNMTX0);
            
                // Draw a textured cube
                DrawCube(&sc->texCtrl);
            
            } // x
        } // y
    } // z

}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    u16 down = DEMOPadGetButtonDown(0);


    // Model rotation parameter
    sc->modelRot = ( sc->modelRot + 1 ) % 360;
    

    // Change number of preloaded textures
    if ( ( down & PAD_TRIGGER_L ) && sc->texCtrl.numPreload > 0 )
    {
        sc->texCtrl.numPreload -= 1;
        StatusMessage(sc->texCtrl.numPreload);
    }

    if ( ( down & PAD_TRIGGER_R ) && sc->texCtrl.numPreload < NUM_REGIONS - 1 )
    {
        sc->texCtrl.numPreload += 1;
        StatusMessage(sc->texCtrl.numPreload);
    }

    // Change number of drawing duplications
    if ( ( down & PAD_BUTTON_Y ) && sc->duplication > 1 )
    {
        sc->duplication -= 1;
        OSReport("Drawing %d layers\n", sc->duplication);
    }

    if ( ( down & PAD_BUTTON_X ) && sc->duplication < 8 )
    {
        sc->duplication += 1;
        OSReport("Drawing %d layers\n", sc->duplication);
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawCube
    
    Description:    Draw a textured cube. Each face can have differnt texture
                    which is automatically set by LoadOneTexture function.
    
    Arguments:      tco : texture control object which includes
                          texture map set mapped on faces

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawCube( MyTexCtrlObj* tco )
{
    u8   i, j;

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);

    // draw the box
    for ( i = 0 ; i < 6 ; ++i )
    {
        LoadOneTexture(tco);
        
        // Draw a face
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            for ( j = 0 ; j < 4 ; ++j )
            {
                GXPosition1x8(CubeFaces[i][j]);
                GXTexCoord1x8(j);
            }
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           LoadOneTexture
    
    Description:    Load an appropriate texture (which number is
                    generated automatically)
                    If the texture is preloaded, this function calls
                    GXLoadTexObjPreLoaded and use preloaded region.
    
    Arguments:      tco : texture control object which includes
                          texture map set to be mapped on
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void LoadOneTexture( MyTexCtrlObj* tco )
{
    u32 texNo;

    // Generate an appropriate number
    texNo = tco->numGen0;
    if ( ++(tco->numGen0) > tco->numGen1 )
    {
        tco->numGen0 = 0;
        tco->numGen1 = ( tco->numGen1 + 1 ) % NUM_TEXTURES;
    }

    // Load texture object
    if ( texNo < tco->numPreload )
    {
        GXLoadTexObjPreLoaded(
            &tco->texObjs[texNo],
            (GXTexRegion*)(GXGetTexObjUserData(&tco->texObjs[texNo])),
            GX_TEXMAP0 );
    }
    else
    {
        GXLoadTexObj(&tco->texObjs[texNo], GX_TEXMAP0);
    }
}

/*---------------------------------------------------------------------------*
    Name:           InitTexNumGen
    
    Description:    Initialize parameters for generating texture number
                    which is used in LoadOneTexture function.
    
    Arguments:      tco : texture control object in which parameters
                          should be initialized
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void InitTexNumGen( MyTexCtrlObj* tco )
{
    tco->numGen0 = 0;
    tco->numGen1 = 0;
}

/*---------------------------------------------------------------------------*
    Name:           ConfigureTmemMap
    
    Description:    Initialize TMEM configuration. Set up cache regions
                    and preloading regions. This function also preloads
                    specified textures.
                    Texture #0 - #(numPreload - 1) are preloaded.
    
    Arguments:      tco : a pointer to MyTexCtrlObj structure which
                          includes number of preloaded texture, texture
                          object array and region array.
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void ConfigureTmemMap( MyTexCtrlObj* tco )
{
    u32  i, r;
    u32  numPreload, numCaches;

    // Synchronization for new TMEM cache configuration
    GXTexModeSync();

    numPreload = tco->numPreload;
    numCaches  = NUM_REGIONS - numPreload;
    
    for ( i = 0 ; i < NUM_REGIONS ; ++i )
    {
        if ( i < numPreload )
        {
            // The region is used for preloading.
            // Though this test is using only 32K region, it is possible
            // to use any 32byte-aligned region for preloading.
            GXInitTexPreLoadRegion(
                &tco->texRegions[i],
                0x00000 + i * 0x08000,  // tmem_even
                0x08000,                // size_even
                0x80000 + i * 0x08000,  // tmem_odd
                0x08000 );              // size_odd
        
            // Preload texture (tex No.i uses region No.i respectively)
            GXPreLoadEntireTexture(&tco->texObjs[i], &tco->texRegions[i]);
            // The userdata field keeps the pointer to region object.
            GXInitTexObjUserData(
                &tco->texObjs[i],
                (void*)(&tco->texRegions[i]) );
        }
        else
        {
            // The region is used as a cache.
            GXInitTexCacheRegion(
                &tco->texRegions[i],
                GX_FALSE,               // 32b mipmap
                0x00000 + i * 0x08000,  // tmem_even
                GX_TEXCACHE_32K,        // size_even
                0x80000 + i * 0x08000,  // tmem_odd
                GX_TEXCACHE_32K );      // size_odd
        }
    }

    // Decide which cache region should be used beforehand.
    for ( i = numPreload ; i < NUM_TEXTURES ; ++i )
    {
        // Get appropriate cache region number.
        r = numPreload + ( i % numCaches );
        // The userdata field keeps the pointer to region object directly.
        GXInitTexObjUserData(
            &tco->texObjs[i],
            (void*)(&tco->texRegions[r]) );
    }

    // Invalidate all new caches.
    GXInvalidateTexAll();
}

/*---------------------------------------------------------------------------*
    Name:           MyTexRegionCallback
    
    Description:    User-defined Callback function for texture cache
                    region allocation

    Arguments:      texObj : a pointer to texture object to be loaded
                    mapID  : destination texmap ID (just same as GXLoadTexObj)
    
    Returns:        appropriate tex cache region for loading texture.
 *---------------------------------------------------------------------------*/
static GXTexRegion* MyTexRegionCallback(GXTexObj* texObj, GXTexMapID mapID)
{
    #pragma unused(mapID)

    // In this test, userdata field already has a pointer
    // to cache region to be used.
    return (GXTexRegion*)(GXGetTexObjUserData(texObj));
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
    Name:           StatusMessage
    
    Description:    Prints current status (number of preloaded textures.)
    
    Arguments:      numPreload : number of preloaded textures
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void StatusMessage( u32 numPreload )
{
    if ( numPreload == 0 )
    {
        OSReport("No preloaded texture\n");
    }
    else if ( numPreload == 1 )
    {
        OSReport("0   : preloaded / 1-15 : cached\n");
    }
    else
    {
        OSReport("0-%d : preloaded / %d-15 : cached\n",
                 numPreload-1, numPreload);
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
    OSReport("************************************************\n");
    OSReport("tex-mix-mode: mixture of preloaded/cached tex test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("L/R Triggers : Change number of preloaded texture.\n");
    OSReport("X/Y Buttons  : Change number of layers to be drawn.\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
