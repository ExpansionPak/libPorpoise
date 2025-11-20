/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-preload.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-preload.c $
    
    7     5/11/01 1:20a Hirose
    removed obsolete flags. fixed some verify messages.
    
    6     5/20/00 5:18p Hirose
    deleted tabs
    
    5     5/19/00 9:10p Hirose
    added pause animation button
    
    4     3/24/00 4:57p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    10    3/03/00 4:21p Alligator
    integrate with ArtX source
    
    9     2/25/00 3:45p Hirose
    updated messages and copyright
    
    8     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    7     2/12/00 5:16p Alligator
    Integrate ArtX source tree changes
    
    6     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    5     1/13/00 5:55p Alligator
    integrate with ArtX GX library code
    
    4     1/05/00 6:46p Hirose
    added coverage for multi-texture and CI texture
    
    3     1/04/00 6:20p Hirose
    changed to use only preloaded textures in this test to make things
    easier.
    
    2     12/22/99 7:00p Hirose
    changed some structures and related codes
    
    1     12/17/99 7:23p Hirose
    initial version
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-preload
        simple texture preloading test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI            3.1415926F

#define NUM_TEXTURES  16

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

// for texture
typedef struct
{
    GXTexObj      texObjs[NUM_TEXTURES];
    GXTexRegion   texRegions[NUM_TEXTURES];
} MyTexCtrlObj;

// for entire scene control
typedef struct
{
    MyCameraObj   cam;
    s32           modelRot;
    MyTexCtrlObj  texCtrl;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main                 ( void );
static void DrawInit             ( MySceneCtrlObj* sc );
static void DrawTick             ( MySceneCtrlObj* sc );
static void AnimTick             ( MySceneCtrlObj* sc );
static void DrawCube             ( void );
static void TexInit              ( MyTexCtrlObj* tco );
static u32  GetTexBufferSizeEven ( u16 wd, u16 ht, u32 fmt, GXBool mm, u8 max );
static u32  GetTexBufferSizeOdd  ( u16 wd, u16 ht, u32 fmt, GXBool mm, u8 max );
static void SetCamera            ( MyCameraObj* cam );
static void PrintIntro           ( void );

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
    -32.0F, // left
    24.0F,  // top
    50.0F,  // near
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
    DEMOInit(NULL);

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
    // Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 0);

    // Array pointers
    GXSetArray(GX_VA_POS,  CubeVertices, 3 * sizeof(s16));
    GXSetArray(GX_VA_TEX0, CubeTexCoords, 2 * sizeof(s16));

    // Initialize texture objects / regions
    TexInit(&sc->texCtrl);


    // Default scene control parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

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
    s32  i, ti;
    Mtx  mt, mv, mr;
    f32  px, py;

    // use 2-textures
    GXSetNumTevStages(2);
    GXSetTevOp   (GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOp   (GX_TEVSTAGE1, GX_DECAL);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetNumTexGens(2);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);

    // entire rotation matrix
    MTXRotDeg(mv, 'x', (f32)sc->modelRot / 8.0F);
    MTXRotDeg(mr, 'z', (f32)sc->modelRot);
    MTXConcat(mv, mr, mr);

    // texcoord matrix
    MTXTrans(mv, -0.5F, -0.5F, 0.0F);
    MTXRotDeg(mt, 'z', (f32)sc->modelRot);
    MTXConcat(mt, mv, mv);
    MTXTrans(mt, 0.5F, 0.5F, 0.0F);
    MTXConcat(mt, mv, mv);
    GXLoadTexMtxImm(mv, GX_TEXMTX0, GX_MTX2x4);

    for ( i = 0 ; i < 8 ; ++i )
    {
        // Set modelview matrix
        px = 220.0F * cosf(i * PI * 45 / 180);
        py = 220.0F * sinf(i * PI * 45 / 180);
        MTXTrans( mt, px, py, 0.0F );
        
        MTXConcat(sc->cam.view, mr, mv);
        MTXConcat(mv, mt, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
            
        // Load preloaded texture
        ti = ( i * 2 ) % NUM_TEXTURES;
        
        GXLoadTexObjPreLoaded(
            &sc->texCtrl.texObjs[ti],
            &sc->texCtrl.texRegions[ti],
            GX_TEXMAP0 );
        GXLoadTexObjPreLoaded(
            &sc->texCtrl.texObjs[ti+1],
            &sc->texCtrl.texRegions[ti+1],
            GX_TEXMAP1 );
        
        // Draw a textured cube
        DrawCube();
    }

}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    // Model rotation parameter
    if (!(DEMOPadGetButton(0) & PAD_BUTTON_A ))
    {
        sc->modelRot = ( sc->modelRot + 1 ) % ( 360 * 16 );
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

    // draw the box
    GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);
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
    Name:           TexInit
    
    Description:    Initialize texture objects and regions for preloaded
                    texture. This function also does preloading.
                    
    Arguments:      tco : pointer to a MyTexCtrlObj structure
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void TexInit( MyTexCtrlObj* tco )
{
    TEXDescriptorPtr  tdp;
    GXTlutObj         tlo;
    GXBool            mipMapFlag;
    u32               fmt;
    u16               wd, ht;
    u8                max;
    u32               tmemLow, tmemHigh;
    u32               sizeEven, sizeOdd;
    u32               i, tlutNum;

    tmemLow  = 0x00000;
    tmemHigh = 0x80000;
    tlutNum  = 0;

    TEXGetPalette(&MyTplObj, "gxTests/tex-08.tpl");
    for ( i = 0 ; i < NUM_TEXTURES ; ++i )
    {
        tdp = TEXGet(MyTplObj, i);
        mipMapFlag =
            ( tdp->textureHeader->minLOD == tdp->textureHeader->maxLOD ) ?
            GX_FALSE : GX_TRUE;
        
        wd  = tdp->textureHeader->width;
        ht  = tdp->textureHeader->height;
        fmt = tdp->textureHeader->format;
        max = (u8)tdp->textureHeader->maxLOD;
        
        // Calculate region size
        sizeEven = GetTexBufferSizeEven(wd, ht, fmt, mipMapFlag, max);
        sizeOdd  = GetTexBufferSizeOdd(wd, ht, fmt, mipMapFlag, max);
        
        if (tdp->CLUTHeader)
        {
            // CI format
            GXInitTexObjCI(
                &tco->texObjs[i],
                tdp->textureHeader->data,
                wd,
                ht,
                (GXCITexFmt)fmt,
                tdp->textureHeader->wrapS, // s
                tdp->textureHeader->wrapT, // t
                mipMapFlag,                // Mipmap
                tlutNum );
            
            // Load Tlut data
            GXInitTlutObj(
                &tlo,
                tdp->CLUTHeader->data,
                (GXTlutFmt)tdp->CLUTHeader->format,
                tdp->CLUTHeader->numEntries );
            GXLoadTlut(&tlo, tlutNum);
            
            // Create new region
            GXInitTexPreLoadRegion(
                &tco->texRegions[i],
                tmemLow,             // tmem_even
                sizeEven,            // size_even
                tmemLow + sizeEven,  // tmem_odd
                sizeOdd );           // size_odd
            // Both even/odd regions are placed on lower bank
            // in the case of CI format.
        
            tmemLow += sizeEven + sizeOdd;
            ++tlutNum;
        }
        else
        {
            // Non-CI format
            GXInitTexObj(
                &tco->texObjs[i],
                tdp->textureHeader->data,
                wd,
                ht,
                (GXTexFmt)fmt,
                tdp->textureHeader->wrapS, // s
                tdp->textureHeader->wrapT, // t
                mipMapFlag );    // Mipmap
        
            // Create new region
            GXInitTexPreLoadRegion(
                &tco->texRegions[i],
                tmemLow,             // tmem_even
                sizeEven,            // size_even
                tmemHigh,            // tmem_odd
                sizeOdd );           // size_odd
        
            tmemLow  += sizeEven;
            tmemHigh += sizeOdd;
        }

        // LOD setting
        GXInitTexObjLOD(
            &tco->texObjs[i],
            tdp->textureHeader->minFilter,
            tdp->textureHeader->magFilter,
            tdp->textureHeader->minLOD,
            tdp->textureHeader->maxLOD,
            tdp->textureHeader->LODBias,
            GX_FALSE,
            tdp->textureHeader->edgeLODEnable,
            GX_ANISO_1 );

        // Preload texture (texture i uses region i)
        GXPreLoadEntireTexture(&tco->texObjs[i], &tco->texRegions[i]);
    }

    // In general case, we also have to re-configure
    // cache regions and region callback in order to avoid
    // redundant usage of same TMEM space. (Default TMEM
    // configuration prepares no area for preloading.)
    // As for this test, such procedure is dispensable
    // because all textures are preloaded and so GXLoadTexObj
    // is never called.
    // (An example of mixture usage of cached/preloaded
    // textures is shown in tex-mix-mode.c. See it also.)
}

/*---------------------------------------------------------------------------*
    Name:           GetTexBufferSizeEven
    
    Description:    returns memory size for preloading region
                    (for even LODs)
                    
    Arguments:      wd  : texture width
                    ht  : texture height
                    fmt : texture format
                    mm  : mipmap switch
                    max : max lod value
                    
    Returns:        required region size
 *---------------------------------------------------------------------------*/
static u32 GetTexBufferSizeEven( u16 wd, u16 ht, u32 fmt, GXBool mm, u8 max )
{
    u32  i;
    u32  size;
    
    if ( fmt == GX_TF_RGBA8 )
    {
        // 32bit format (mipmap/non-mipmap)
        size = GXGetTexBufferSize(wd, ht, fmt, mm, (u8)(max+1));
        size /= 2;
    }
    else if ( mm == GX_FALSE )
    {
        // non-mipmap
        size = GXGetTexBufferSize(wd, ht, fmt, GX_FALSE, 0);
    }
    else
    {
        // mipmap
        size = 0;
        for ( i = 0 ; i <= max ; i += 2 )
        {
            size += GXGetTexBufferSize(wd, ht, fmt, GX_FALSE, 1);
            wd >>= 2;
            ht >>= 2;
        }
    }

    return size;
}

/*---------------------------------------------------------------------------*
    Name:           GetTexBufferSizeOdd
    
    Description:    returns memory size for preloading region
                    (for odd LODs)
                    
    Arguments:      wd  : texture width
                    ht  : texture height
                    fmt : texture format
                    mm  : mipmap switch
                    max : max lod value
                    
    Returns:        required region size
 *---------------------------------------------------------------------------*/
static u32 GetTexBufferSizeOdd( u16 wd, u16 ht, u32 fmt, GXBool mm, u8 max )
{
    u32 size;
    
    size = GXGetTexBufferSize(wd, ht, fmt, mm, (u8)(max+1))
         - GetTexBufferSizeEven(wd, ht, fmt, mm, max);
    
    return size;
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
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("tex-preload: texture preloading test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("A Button : pause animation while pressed\n");
    OSReport("\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
