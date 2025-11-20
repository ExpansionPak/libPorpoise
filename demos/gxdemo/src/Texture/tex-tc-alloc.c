/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-tc-alloc.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-tc-alloc.c $
    
    8     5/11/01 1:20a Hirose
    removed obsolete flags. fixed some verify messages.
    
    7     8/17/00 11:13p Hirose
    corrected the location of the GXTexModeSync call, etc.
    
    6     4/26/00 4:54p Carl
    CallBack -> Callback
    
    5     3/26/00 7:32p Hirose
    centered display object more considering H/V overscan again
    
    4     3/24/00 4:57p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    9     2/25/00 3:23p Hirose
    updated pad control functions to match actual HW1 gamepad
    
    8     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    7     1/24/00 6:30p Hirose
    removed #ifdef EPPC stuff
    merged DrawCaption() function into DrawTick()
    
    6     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    5     1/06/00 6:50p Hirose
    added GXTexModeSync call
    
    4     12/22/99 7:00p Hirose
    deleted StatusMessage() function
    
    3     12/21/99 7:57p Hirose
    added caption
    
    2     12/20/99 9:05p Hirose
    
    1     12/17/99 7:23p Hirose
    initial version
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-tc-alloc
        texture cache region allocator test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define NUM_TEXTURES    16
#define MAX_REGIONS     8
#define NUM_TMEM_MODES  7

#define Clamp(val,min,max) \
    ((val) = (((val)<(min)) ? (min) : ((val)>(max)) ? (max) : (val)))

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

// coord generator
typedef struct
{
    u32    currentColumn;
    s32    count[MAX_REGIONS];
} MyCoordGenObj;

// for texture cache management
typedef struct
{
    u32            numRegions;
    u32            current;
    GXTexRegion    region[MAX_REGIONS];
    u32            tag[MAX_REGIONS];
} MyTexCacheManageObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    s32            ypos;
    s32            ymax;
    u32            numQuads;
    u32            tmemMode;
    u32            rndSeed;
    u16            scrWidth;
    u16            scrHeight;
    GXTexObj       texture[NUM_TEXTURES];   
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main               ( void );
static void DrawInit           ( MySceneCtrlObj* sc );
static void DrawTick           ( MySceneCtrlObj* sc );
static void AnimTick           ( MySceneCtrlObj* sc );
static void DrawQuad           ( void );
static void SetCamera          ( MyCameraObj* cam );

static void ResetCoordGen      ( void );
inline void SetCoordGenCol     ( u32 col );
static void GetMtxFromCoordGen ( Mtx mt );
static s32  GetCoordGenMax     ( void );

static void ConfigureTmemMap   ( u32 mode );
static void TmemMapper32K      ( MyTexCacheManageObj* tcm );
static void TmemMapper128K     ( MyTexCacheManageObj* tcm );
static void TmemMapper512K     ( MyTexCacheManageObj* tcm );
static GXTexRegion* 
            TexRegionCallback0 ( GXTexObj* texObj, GXTexMapID mapID );
static GXTexRegion* 
            TexRegionCallback1 ( GXTexObj* texObj, GXTexMapID mapID );
static GXTexRegion* 
            TexRegionCallback2 ( GXTexObj* texObj, GXTexMapID mapID );
static GXTexRegion* 
            TexRegionCallback3 ( GXTexObj* texObj, GXTexMapID mapID );

static void PrintIntro         ( void );


/*---------------------------------------------------------------------------*
   Model Data
 *---------------------------------------------------------------------------*/
static s8 QuadVertices[3*8] ATTRIBUTE_ALIGN(32) = 
{
    -32,  32,  32, // 0
     32,  32,  32, // 1
     32, -32,  32, // 2
    -32, -32,  32, // 3
};

static s8 QuadTexCoords[2*4] ATTRIBUTE_ALIGN(32) =
{
      0,   0,
      1,   0,
      1,   1,
      0,   1
};

/*---------------------------------------------------------------------------*
   Tmem mapper & allocator (tex region callback) table
 *---------------------------------------------------------------------------*/
static void (* MyTmemMappers[NUM_TMEM_MODES])
            ( MyTexCacheManageObj* tcm ) =
{
    TmemMapper32K,
    TmemMapper32K,
    TmemMapper32K,
    TmemMapper128K,
    TmemMapper128K,
    TmemMapper128K,
    TmemMapper512K
};

static GXTexRegion* (* MyTexRegionCallbacks[NUM_TMEM_MODES])
                    ( GXTexObj* texObj, GXTexMapID mapID ) =
{
    TexRegionCallback1,
    TexRegionCallback2,
    TexRegionCallback3,
    TexRegionCallback1,
    TexRegionCallback2,
    TexRegionCallback3,
    TexRegionCallback0
};

static char* TmemModeMsg[NUM_TMEM_MODES] =
{
    "8x32K caches  / allocation algorithm 0",
    "8x32K caches  / allocation algorithm 1",
    "8x32K caches  / allocation algorithm 2",
    "4x128K caches / allocation algorithm 0",
    "4x128K caches / allocation algorithm 1",
    "4x128K caches / allocation algorithm 2",
    "1x512K cache  / every texture uses one cache"
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
static MySceneCtrlObj       SceneCtrl;                // scene control parameters

static TEXPalettePtr        MyTplObj = NULL; // texture palette
static MyTexCacheManageObj  TexCacheManager; // texture cache manager
static MyCoordGenObj        CoordGenerator;  // display coord generator

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
    GXRenderModeObj*  rmode;
    TEXDescriptorPtr  tdp;
    GXBool            mipMapFlag;
    u32               i;
    
    // Vertex Attribute (VTXFMT0 is used by DEMOPuts library)
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_S8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_S8, 0);

    // Array pointers
    GXSetArray(GX_VA_POS,  QuadVertices, 3 * sizeof(s8));
    GXSetArray(GX_VA_TEX0, QuadTexCoords, 2 * sizeof(s8));

    // Load TPL file and initialize texture objects
    TEXGetPalette(&MyTplObj, "gxTests/tex-07.tpl");
    for ( i = 0 ; i < NUM_TEXTURES ; ++i )
    {
        tdp = TEXGet(MyTplObj, i);
        mipMapFlag =
            ( tdp->textureHeader->minLOD == tdp->textureHeader->maxLOD ) ?
            GX_FALSE : GX_TRUE;
        
        GXInitTexObj(
            &sc->texture[i],
            tdp->textureHeader->data,
            tdp->textureHeader->width,
            tdp->textureHeader->height,
            (GXTexFmt)tdp->textureHeader->format,
            tdp->textureHeader->wrapS, // s
            tdp->textureHeader->wrapT, // t
            mipMapFlag ); // Mipmap
        
        GXInitTexObjLOD(
            &sc->texture[i],
            tdp->textureHeader->minFilter,
            tdp->textureHeader->magFilter,
            tdp->textureHeader->minLOD,
            tdp->textureHeader->maxLOD,
            tdp->textureHeader->LODBias,
            GX_FALSE,
            tdp->textureHeader->edgeLODEnable,
            GX_ANISO_1 );
        
        // UserData field is used as ID number.
        GXInitTexObjUserData(&sc->texture[i], (void*)i);
    }

    // Get Screen Information
    rmode = DEMOGetRenderModeObj();
    sc->scrWidth  = (u16)rmode->fbWidth;   // Screen Width
    sc->scrHeight = (u16)rmode->efbHeight; // Screen Height


    // Default scene control parameter settings

    // camera and position
    sc->cam.cfg = DefaultCamera;
    sc->ypos = 0;
    sc->ymax = 0;

    // number of quads to be displayed
    sc->numQuads = 500;

    // tmem configuration mode
    sc->tmemMode = 0;
    
    // seed of random function
    sc->rndSeed = 1;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mt, mv;
    u32  ti;
    s32  i;

    // configure tmem map
    ConfigureTmemMap(sc->tmemMode);

    // set camera position
    SetCamera(&sc->cam);
    
    // reset coord generator
    ResetCoordGen();

    // set up Tev, TexGen and Zmode
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
    
    // set seed of random number 
    srand(sc->rndSeed);

    for ( i = 0 ; i < sc->numQuads ; ++i )
    {
        // Make random tex number for each face
        ti = (u32)rand() % NUM_TEXTURES;
        
        // Load texture
        GXLoadTexObj(&sc->texture[ti], GX_TEXMAP0);
        
        // Set modelview matrix
        GetMtxFromCoordGen(mt);
        MTXConcat(sc->cam.view, mt, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);

        // Draw a textured Quad
        DrawQuad();
    }
    // Keep max Y value of current coord generator
    sc->ymax = GetCoordGenMax() * 48 - 400;

    
    // Draw Captions

    // DEMOPuts lib uses a texture which ID is not defined.
    // So set the Callback which never uses ID information.
    GXSetTexRegionCallback(TexRegionCallback0);

    DEMOInitCaption(DM_FT_OPQ, sc->scrWidth, sc->scrHeight);
    DEMOPrintf(56, (s16)(sc->scrHeight - 40), 0, TmemModeMsg[sc->tmemMode]);
    for ( i = 0 ; i < TexCacheManager.numRegions ; ++i )
    {
        DEMOPrintf((s16)(i*68+48), 32, 0, "[CACHE%d]", i);
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
    static u32 frameCounter = 0;
    u16 down;
    
    down = DEMOPadGetButtonDown(0);

    // change TMEM map & allocator mode
    if ( down & PAD_TRIGGER_L )
    {
        sc->tmemMode += NUM_TMEM_MODES - 1;
        sc->tmemMode = sc->tmemMode % NUM_TMEM_MODES;
        sc->ypos = 0;
    }
    if ( down & PAD_TRIGGER_R )
    {
        sc->tmemMode += 1;
        sc->tmemMode = sc->tmemMode % NUM_TMEM_MODES;
        sc->ypos = 0;
    }

    // change number of quads to be displayed
    if ( down & PAD_BUTTON_Y )
    {
        if ( sc->numQuads > 100 )
        {
            sc->numQuads -= 100;
            OSReport("Quads : %d\n", sc->numQuads);
        }
    }
    if ( down & PAD_BUTTON_X )
    {
        if ( sc->numQuads < 1000 )
        {
            sc->numQuads += 100;
            OSReport("Quads : %d\n", sc->numQuads);
        }
    }

    // change seed of random value generation
    if ( down & PAD_BUTTON_B )
    {
        sc->rndSeed = sc->rndSeed * sc->rndSeed + frameCounter;
        sc->ypos = 0;
    }

    // move camera position
    sc->ypos -= DEMOPadGetStickY(0) / 2;
    Clamp(sc->ypos, 0, sc->ymax);
    sc->cam.cfg.location.y = (f32)(- sc->ypos);
    sc->cam.cfg.target.y   = (f32)(- sc->ypos);

    // count up frames
    ++frameCounter;    
}

/*---------------------------------------------------------------------------*
    Name:           DrawQuad
    
    Description:    Draw a textured Quad. Each face can have differnt texture
                    which is specified by given texture number list.
    
    Arguments:      texNoList : a pointer to texture number list

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawQuad( void )
{
    u8   i;

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);

    // draw the quad
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        for ( i = 0 ; i < 4 ; ++i )
        {
            GXPosition1x8(i);
            GXTexCoord1x8(i);
        }
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    Set view matrix and load projection matrix into hardware
                    
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
    Name:           ResetCoordGen
    
    Description:    Reset coordinate generator which is used to decide
                    position for each quads dynamically.
    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void ResetCoordGen( void )
{
    u32  i;

    CoordGenerator.currentColumn = 0;
    for ( i = 0 ; i < MAX_REGIONS ; ++i )
    {
        CoordGenerator.count[i] = 0;
    }
}

/*---------------------------------------------------------------------------*
    Name:           SetCoordGenCol
    
    Description:    Set current column for generating coordinates
    
    Arguments:      col : column number
    
    Returns:        none
 *---------------------------------------------------------------------------*/
inline void SetCoordGenCol( u32 col )
{
    CoordGenerator.currentColumn = col;
}

/*---------------------------------------------------------------------------*
    Name:           GetMtxFromCoordGen
    
    Description:    Get model transfer matrix by using current state
                    of the coordinate generator

    Arguments:      mt : a matrix the result should be put
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void GetMtxFromCoordGen( Mtx mt )
{
    u32  col;
    s32  cnt;
    
    col = CoordGenerator.currentColumn;
    cnt = CoordGenerator.count[col]++;

    // model transfer matrix
    MTXTrans( mt,
              (f32)col * 68.0F - 240.0F,
              (f32)cnt * -48.0F + 160.0F,
              0.0F );
}

/*---------------------------------------------------------------------------*
    Name:           GetCoordGenMax
    
    Descriptions:   Get the maximum count number of all columns
                    at current state of the coordinate generator
    
    Arguments:      none
    
    Returns:        the maximum number
 *---------------------------------------------------------------------------*/
static s32 GetCoordGenMax( void )
{
    u32  i;
    s32  max = 0;

    for ( i = 0 ; i < MAX_REGIONS ; ++i )
    {
        if ( CoordGenerator.count[i] > max )
        {
            max = CoordGenerator.count[i];
        }
    }

    return max;
}

/*---------------------------------------------------------------------------*
    Name:           ConfigureTmemMap
    
    Description:    Initialize TMEM configuration. 
                    Set up cache regions.
    
    Arguments:      mode : tmem map & cache allocation mode
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void ConfigureTmemMap( u32 mode )
{
    u32  i;

    // Synchronization for new TMEM cache configuration
    GXTexModeSync();

    // Call tmem mapper for specified mode.
    (*MyTmemMappers[mode])(&TexCacheManager);

    // Tex region Callback for specified mode.
    GXSetTexRegionCallback(MyTexRegionCallbacks[mode]);

    // Initialize cache manager
    TexCacheManager.current = 0;
    for ( i = 0 ; i < MAX_REGIONS ; ++i )
    {
        TexCacheManager.tag[i]   = 0;
    }

    // Invalidate all new caches
    GXInvalidateTexAll();
}

/*---------------------------------------------------------------------------*
    Name:           TmemMapper32K
    
    Description:    Set up 8x32K texture cache regions
    
    Arguments:      tcm : a pointer to the cache manager
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void TmemMapper32K( MyTexCacheManageObj* tcm )
{
    u32  i;

    tcm->numRegions = MAX_REGIONS;

    for ( i = 0 ; i < tcm->numRegions ; ++i )
    {
        // The region is used as a 32K cache.
        GXInitTexCacheRegion(
            &tcm->region[i],
            GX_FALSE,               // 32b mipmap
            0x00000 + i * 0x08000,  // tmem_even
            GX_TEXCACHE_32K,        // size_even
            0x80000 + i * 0x08000,  // tmem_odd
            GX_TEXCACHE_32K );      // size_odd
    }
}

/*---------------------------------------------------------------------------*
    Name:           TmemMapper128K
    
    Description:    Set up 4x128K texture cache regions
    
    Arguments:      tcm : a pointer to the cache manager
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void TmemMapper128K( MyTexCacheManageObj* tcm )
{
    u32  i;

    tcm->numRegions = 4;

    for ( i = 0 ; i < tcm->numRegions ; ++i )
    {
        // The region is used as a 128K cache.
        GXInitTexCacheRegion(
            &tcm->region[i],
            GX_FALSE,               // 32b mipmap
            0x00000 + i * 0x20000,  // tmem_even
            GX_TEXCACHE_128K,       // size_even
            0x80000 + i * 0x20000,  // tmem_odd
            GX_TEXCACHE_128K );     // size_odd
    }
}

/*---------------------------------------------------------------------------*
    Name:           TmemMapper512K
    
    Description:    Set up 1x512K texture cache regions
    
    Arguments:      tcm : a pointer to the cache manager
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void TmemMapper512K( MyTexCacheManageObj* tcm )
{
    tcm->numRegions = 1;

    // The region is used as a 512K cache.
    GXInitTexCacheRegion(
        &tcm->region[0],
        GX_FALSE,           // 32b mipmap
        0x00000,            // tmem_even
        GX_TEXCACHE_512K,   // size_even
        0x80000,            // tmem_odd
        GX_TEXCACHE_512K ); // size_odd
}

/*---------------------------------------------------------------------------*
    Name:           TexRegionCallback0
    
    Description:    Tex cache allocator which always returns region 0

    Arguments:      texObj : a pointer to texture object to be loaded
                    mapID  : destination texmap ID (just same as GXLoadTexObj)
    
    Returns:        appropriate tex cache region for loading texture.
 *---------------------------------------------------------------------------*/
static GXTexRegion* TexRegionCallback0(GXTexObj* texObj, GXTexMapID mapID)
{
    #pragma unused(texObj)
    #pragma unused(mapID)

    SetCoordGenCol(0);
    
    return &TexCacheManager.region[0];
}

/*---------------------------------------------------------------------------*
    Name:           TexRegionCallback1
    
    Description:    Tex cache allocator using simple round algorithm

    Arguments:      texObj : a pointer to texture object to be loaded
                    mapID  : destination texmap ID (just same as GXLoadTexObj)
    
    Returns:        appropriate tex cache region for loading texture.
 *---------------------------------------------------------------------------*/
static GXTexRegion* TexRegionCallback1(GXTexObj* texObj, GXTexMapID mapID)
{
    #pragma unused(texObj)
    #pragma unused(mapID)

    u32  regionNum;

    regionNum = TexCacheManager.current++;
    TexCacheManager.current %= TexCacheManager.numRegions;

    SetCoordGenCol(regionNum);
    
    return &TexCacheManager.region[regionNum];
}

/*---------------------------------------------------------------------------*
    Name:           TexRegionCallback2
    
    Description:    Tex cache allocator using texture ID

    Arguments:      texObj : a pointer to texture object to be loaded
                    mapID  : destination texmap ID (just same as GXLoadTexObj)
    
    Returns:        appropriate tex cache region for loading texture.
 *---------------------------------------------------------------------------*/
static GXTexRegion* TexRegionCallback2(GXTexObj* texObj, GXTexMapID mapID)
{
    #pragma unused(mapID)

    u32  texID, regionNum;
    
    texID     = (u32)GXGetTexObjUserData(texObj);
    regionNum = texID % TexCacheManager.numRegions;

    SetCoordGenCol(regionNum);

    return &TexCacheManager.region[regionNum];
}

/*---------------------------------------------------------------------------*
    Name:           TexRegionCallback3
    
    Description:    Check whether ID of given texture is used in any
                    cache region. If it hits, uses the region.

    Arguments:      texObj : a pointer to texture object to be loaded
                    mapID  : destination texmap ID (just same as GXLoadTexObj)
    
    Returns:        appropriate tex cache region for loading texture.
 *---------------------------------------------------------------------------*/
static GXTexRegion* TexRegionCallback3(GXTexObj* texObj, GXTexMapID mapID)
{
    #pragma unused(mapID)

    u32  i, n;
    u32  texID, regionNum;

    texID = (u32)GXGetTexObjUserData(texObj);
    n = TexCacheManager.numRegions;
    // Search given ID from tags
    for ( i = 0 ; i < n ; ++i )
    {
        if ( TexCacheManager.tag[i] == texID )
        {
            regionNum = i;
            break;
        }
    }

    if ( i == n )
    {
        regionNum = TexCacheManager.current++;
        TexCacheManager.current %= TexCacheManager.numRegions;
    }

    TexCacheManager.tag[regionNum] = texID;
    SetCoordGenCol(regionNum);
    
    return &TexCacheManager.region[regionNum];
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
    OSReport("tex-tc-alloc: tex cache region allocator test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick   : Scroll the scene\n");
    OSReport("L/R Triggers : Change TMEM map & allocation algorithm\n");
    OSReport("X/Y Buttons  : Change number of quads to be drawn.\n");
    OSReport("B Button     : Change seed of random function.\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
