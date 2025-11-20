/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-tlut.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-tlut.c $    
    
    8     5/11/01 1:20a Hirose
    removed obsolete flags. fixed some verify messages.
    
    7     6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    6     4/26/00 4:54p Carl
    CallBack -> Callback
    
    5     3/26/00 7:32p Hirose
    centered display object more considering H/V overscan again
    
    4     3/24/00 6:23p Carl
    Adjusted for overscan.
    
    3     3/24/00 4:57p Hirose
    changed to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    7     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    6     2/25/00 2:36p Hirose
    updated pad control function to match actual HW1 gamepad
    changed memory allocation scheme
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    3     1/17/00 3:46p Hirose
    made TlutSizeTable and set tlut size value by the correct way
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    8     11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    7     10/20/99 11:25a Hirose
    deleted vertex color
    
    6     10/07/99 2:54p Hirose
    added single frame test / changed some functions
    
    5     10/04/99 6:47p Yasu
    Rename -Sz to -Size
    
    4     9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    3     9/24/99 8:15p Hirose
    
    2     9/23/99 10:33p Hirose
    beta version
    
    1     9/21/99 3:13p Hirose
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-tlut
        CI texture with arbitrary TLUT size test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>

/*---------------------------------------------------------------------------*
  Macro definitions
 *---------------------------------------------------------------------------*/
#define NUM_REGIONS        9

#define BG_TEXTURE_WIDTH   8
#define BG_TEXTURE_HEIGHT  8
#define BG_TEXTURE_FORMAT  GX_TF_I4

#define SAMPLE_IMG_WIDTH   128
#define SAMPLE_IMG_HEIGHT  128

#define TEX_CMAP           0  // ID for color map texture
#define TEX_SIMG           1  // ID for quantized sample image texture
#define TEX_BG             2  // ID for background texture

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

// for TLUT size
typedef struct
{
    GXTlutSize    symbol;
    u32           actualSize;
} MyTlutSize;

// for texture
typedef struct
{
    GXTexObj      tobj;
    u8*           data;
    u16           width;
    u16           height;
    GXCITexFmt    format;
} MyTextureObj;

// for color map
typedef struct
{
    u32           tlutNumber;
    GXTlutFmt     tlutFormat;
    u16*          tlutBuffer;
    u8            mapCtrl[4];
} MyColorMapObj;

// for entire scene control
typedef struct
{
    MyCameraObj   cam;
    MyTextureObj  texture[3];
    MyColorMapObj cmap;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main           ( void );
static void DrawInit       ( MySceneCtrlObj* sc );
static void DrawTick       ( MySceneCtrlObj* sc );
static void AnimTick       ( MySceneCtrlObj* sc );
static void SetScene       ( MySceneCtrlObj* sc );
static void SetCamera      ( MyCameraObj* cam );
static void DrawQuad       ( void );

static void CreateColorMapTexture  ( MyTextureObj* to, MyColorMapObj* cmo );
static void CreateSampleImgTexture ( MyTextureObj* to, MyColorMapObj* cmo );
static void InitColorMap           ( MyColorMapObj* cmo );
static void ChangeColorMap         ( MyColorMapObj* cmo, u32 cmp_id );
static void LoadColorMap           ( MyColorMapObj* cmo );
static void PlotOnTexture          ( MyTextureObj* to, u32 x, u32 y, u32 val );

static void          MyTlutRegionInit     ( void );
static GXTlutRegion* MyTlutRegionCallback ( u32 idx );

static void StatusMessage  ( MyColorMapObj* cmo );
static void PrintIntro     ( void );

/*---------------------------------------------------------------------------*
   External data in tex-data04.c
 *---------------------------------------------------------------------------*/
extern u8 BGTextureData[];
extern u8 SampleImageData[];

/*---------------------------------------------------------------------------*
   Data for TLUT configuration
 *---------------------------------------------------------------------------*/
static MyTlutSize TlutSizeTable[NUM_REGIONS] =
{
    { GX_TLUT_16,  0x00200 },
    { GX_TLUT_32,  0x00400 },
    { GX_TLUT_64,  0x00800 },
    { GX_TLUT_128, 0x01000 },
    { GX_TLUT_256, 0x02000 },
    { GX_TLUT_512, 0x04000 },
    { GX_TLUT_1K,  0x08000 },
    { GX_TLUT_2K,  0x10000 },
    { GX_TLUT_4K,  0x20000 }
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 100.0F }, // location
    { 0.0F, 1.0F,   0.0F }, // up
    { 0.0F, 0.0F,   0.0F }, // tatget
    -320.0F, // left
    224.0F,  // top  (note: was 240, now adjusted for overscan)
    50.0F,   // near
    2000.0F  // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;                  // scene control parameters
static GXTlutRegion    MyTlutRegions[NUM_REGIONS]; // own tlut configulation

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);

    DrawInit(&SceneCtrl); // Initialize vertex formats, tlut regions
                          // and default scene settings.

    PrintIntro();  // Print demo directions

    StatusMessage(&SceneCtrl.cmap);
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
                    tlut regions.
                    This function also initializes scene control parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{ 
    Mtx  ms; // Scale matrix.
    Mtx  mt; // Translation matrix.
    Mtx  mv; // Modelview matrix.

    //   Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    //  Texture and TLUT
    MyTlutRegionInit();
    GXInvalidateTexAll();
        
    // Texture coord generation setting
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetNumTexGens(1);


    // Default scene control parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // colormap information
    sc->cmap.tlutNumber = 0;
    sc->cmap.tlutFormat = GX_TL_IA8;
    sc->cmap.tlutBuffer = NULL;
    InitColorMap(&sc->cmap);

    // dynamic generated textures
    sc->texture[TEX_CMAP].data = NULL;
    sc->texture[TEX_SIMG].data = NULL;

    // static background texture
    GXInitTexObj(
        &sc->texture[TEX_BG].tobj,
        BGTextureData,
        BG_TEXTURE_WIDTH,
        BG_TEXTURE_HEIGHT,
        BG_TEXTURE_FORMAT,
        GX_REPEAT,
        GX_REPEAT,
        GX_FALSE );
     GXInitTexObjLOD(
        &sc->texture[TEX_BG].tobj,
        GX_LINEAR,
        GX_LINEAR,
        0,
        0,
        0,
        GX_FALSE,
        GX_FALSE,
        GX_ANISO_1 );

    // create default scene
    SetScene(sc);


    //  Make modelview matrices (never changes in this test)
    
    // GX_PNMTX0 is used for color map pattern texture wall
    MTXTrans(mt, -144.0F, 0.0F, 0.0F);
    MTXConcat(sc->cam.view, mt, mv);
    MTXScale(ms, 128.0F, 128.0F, 1.0F);
    MTXConcat(mv, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    // GX_PNMTX1 is used for sample image texture wall
    MTXTrans(mt, 144.0F, 0.0F, 0.0F);
    MTXConcat(sc->cam.view, mt, mv);
    MTXScale(ms, 128.0F, 128.0F, 1.0F);
    MTXConcat(mv, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX1);

    // GX_PNMTX2 is used for background tesselated wall
    MTXTrans(mt, -144.0F, 0.0F, -1.0F);
    MTXConcat(sc->cam.view, mt, mv);
    MTXScale(ms, 128.0F, 128.0F, 1.0F);
    MTXConcat(mv, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX2);
    
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    static f32 s = 0.0F;

    Mtx  ms; // Scale matrix.
    Mtx  mc; // TexCoord matrix.

    // set tev operation to display texture
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

    // enable alpha blend mode
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);

    //
    // Draw background wall
    //

    // set modelview and texcoord matrix
    GXSetCurrentMtx(GX_PNMTX2);
    MTXTrans(mc, s, s, 0.0F);
    MTXScale(ms, 16.0F, 16.0F, 0.0F);
    MTXConcat(mc, ms, mc);
    GXLoadTexMtxImm(mc, GX_TEXMTX0, GX_MTX2x4);
    // set texture object for background
    GXLoadTexObj(&sc->texture[TEX_BG].tobj, GX_TEXMAP0);
    // draw a quad
    DrawQuad();

    //
    // Draw color map texture
    //

    // set modelview and texcoord matrix
    GXSetCurrentMtx(GX_PNMTX0);
    MTXIdentity(mc);
    GXLoadTexMtxImm(mc, GX_TEXMTX0, GX_MTX2x4);
    // set texture object for test pattern
    GXLoadTexObj(&sc->texture[TEX_CMAP].tobj, GX_TEXMAP0);
    // draw a quad
    DrawQuad();

    //
    // Draw quantized sample image texture
    //

    // set modelview and texcoord matrix
    GXSetCurrentMtx(GX_PNMTX1);
    MTXIdentity(mc);
    GXLoadTexMtxImm(mc, GX_TEXMTX0, GX_MTX2x4);
    // set texture object for sample image
    GXLoadTexObj(&sc->texture[TEX_SIMG].tobj, GX_TEXMAP0);
    // draw a quad
    DrawQuad();

    // texture coord animation
    s += 0.005F;
    if ( s >= 1.0F )
    {
        s = 0.0F;
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
    u16  down = DEMOPadGetButtonDown(0);
    u16  dirs = DEMOPadGetDirsNew(0);
    
    
    // Tlut number (& size) control
    if ( down & PAD_TRIGGER_L )
    {
        if ( sc->cmap.tlutNumber > 0 )
        {
            --(sc->cmap.tlutNumber);
        }
        
        InitColorMap(&sc->cmap);
        SetScene(sc);
        StatusMessage(&sc->cmap);
    }
    if ( down & PAD_TRIGGER_R )
    {
        if ( sc->cmap.tlutNumber < NUM_REGIONS - 1 )
        {
            ++(sc->cmap.tlutNumber);
        }
        
        InitColorMap(&sc->cmap);
        SetScene(sc);
        StatusMessage(&sc->cmap);
    }

    // Tlut format control
    if ( down & PAD_BUTTON_A )
    {
        sc->cmap.tlutFormat =
              ( sc->cmap.tlutFormat == GX_TL_IA8    ) ? GX_TL_RGB565
            : ( sc->cmap.tlutFormat == GX_TL_RGB565 ) ? GX_TL_RGB5A3
            :                                           GX_TL_IA8;
        
        InitColorMap(&sc->cmap);
        SetScene(sc);
        StatusMessage(&sc->cmap);
    }

    // Change color map
    if ( dirs & DEMO_STICK_RIGHT )
    {
        ChangeColorMap(&sc->cmap, 0);
        SetScene(sc);
        StatusMessage(&sc->cmap);
    }
    if ( dirs & DEMO_STICK_LEFT )
    {
        ChangeColorMap(&sc->cmap, 1);
        SetScene(sc);
        StatusMessage(&sc->cmap);
    }
    if ( dirs & DEMO_STICK_UP )
    {
        ChangeColorMap(&sc->cmap, 2);
        SetScene(sc);
        StatusMessage(&sc->cmap);
    }
    if ( dirs & DEMO_STICK_DOWN )
    {
        ChangeColorMap(&sc->cmap, 3);
        SetScene(sc);
        StatusMessage(&sc->cmap);
    }
}

/*---------------------------------------------------------------------------*
    Name:           SetScene
    
    Description:    Sets up tlut data and textures necessary to make a scene
    
    Arguments:      sc : pointer to the structure of scene control parameters

    Returns:        none
 *---------------------------------------------------------------------------*/
void SetScene( MySceneCtrlObj* sc )
{
    LoadColorMap(&sc->cmap);
    CreateColorMapTexture(&sc->texture[TEX_CMAP], &sc->cmap);
    CreateSampleImgTexture(&sc->texture[TEX_SIMG], &sc->cmap);

    // Invalidate all cached textures
    GXInvalidateTexAll();
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    Sets view matrix and loads projection matrix into hardware
                    
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
    Name:           DrawQuad
    
    Description:    Draws a textured quad
    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawQuad( void )
{
    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // draw a quad
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16( 1,  1, -1);
        GXTexCoord2f32(1.0F, 0.0F);
        GXPosition3s16( 1, -1, -1);
        GXTexCoord2f32(1.0F, 1.0F);
        GXPosition3s16(-1, -1, -1);
        GXTexCoord2f32(0.0F, 1.0F);
        GXPosition3s16(-1,  1, -1);
        GXTexCoord2f32(0.0F, 0.0F);
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           CreateColorMapTexture
    
    Description:    Creates a mosaic texture which shows all indexed
                    color data in given color map
    
    Arguments:      to  : a pointer to the MyTextureObj structure
                          where created texture data is stored
                    cmo : a pointer to the MyColorMapObj structure
                          which provides current color map information

    Returns:        none
 *---------------------------------------------------------------------------*/
static void CreateColorMapTexture(MyTextureObj* to, MyColorMapObj* cmo)
{
    u16  iw, ih;
    u32  ciSize, bufferSize, ic;

    // calculate texture size by CI size
    ciSize = 1u << ( cmo->tlutNumber + 4 );
    to->width = to->height = 1;
    while(1)
    {
        to->width *= 2;
        if ( to->width * to->height >= ciSize )
            break;
        
        to->height *= 2;
        if ( to->width * to->height >= ciSize )
            break;
    }

    // determine texture format
    to->format = ( ciSize <= 16  ) ? GX_TF_C4
               : ( ciSize <= 256 ) ? GX_TF_C8
               :                     GX_TF_C14X2;

    // allocate texture data memory
    if ( to->data != NULL )
    {
        OSFree(to->data);
    }
    bufferSize = GXGetTexBufferSize(
                     to->width,
                     to->height,
                     to->format,
                     GX_FALSE,
                     0 );
    to->data   = OSAlloc(bufferSize);

    // make map pattern
    ic = 0;
    for ( ih = 0 ; ih < to->height ; ++ih )
    {
        for ( iw = 0 ; iw < to->width ; ++iw )
        {
            PlotOnTexture(to, iw, ih, ic);
            ic = ( ic + 1 ) % ciSize;
        }
    }

    // initialize texture object
    GXInitTexObjCI(
        &to->tobj,
        to->data,
        to->width,
        to->height,
        to->format,
        GX_REPEAT,
        GX_REPEAT,
        GX_FALSE,
        cmo->tlutNumber );
    GXInitTexObjLOD(
        &to->tobj,
        GX_NEAR,
        GX_NEAR,
        0,
        0,
        0,
        GX_FALSE,
        GX_FALSE,
        GX_ANISO_1 );

    // flush data cache to certainly send all created data into main memory.
    DCFlushRange(to->data, bufferSize);
}

/*---------------------------------------------------------------------------*
    Name:           CreateSampleImgTexture
    
    Description:    Quantizes sample image data by using given color map 
                    and store as a texture data
    
    Arguments:      to  : a pointer to the MyTextureObj structure
                          where created texture data is stored
                    cmo : a pointer to the MyColorMapObj structure
                          which provides current color map information

    Returns:        none
 *---------------------------------------------------------------------------*/
static void CreateSampleImgTexture(MyTextureObj* to, MyColorMapObj* cmo)
{
    u8*  srcdata = SampleImageData;
    u16  iw, ih;
    u32  ciSize;
    u32  offset;
    u32  bufferSize;
    
    ciSize     = 1u << ( cmo->tlutNumber + 4 );
    to->width  = SAMPLE_IMG_WIDTH;
    to->height = SAMPLE_IMG_HEIGHT;
    offset = 0;

    // determine texture format
    to->format = ( ciSize <= 16  ) ? GX_TF_C4
               : ( ciSize <= 256 ) ? GX_TF_C8
               :                     GX_TF_C14X2;
    
    // allocate texture data memory
    if ( to->data != NULL )
    {
        OSFree(to->data);
    }
    bufferSize = GXGetTexBufferSize(
                     to->width,
                     to->height,
                     to->format,
                     GX_FALSE,
                     0 );
    to->data   = OSAlloc(bufferSize);


    // make quantized sample image
    for ( ih = 0 ; ih < to->height ; ++ih )
    {
        for ( iw = 0 ; iw < to->width ; ++iw )
        {
            u32 ic, c, shift, id;
 
            ic = c = shift = 0;
            
            switch(cmo->tlutFormat)
            {
                case GX_TL_IA8:
                {
                    for ( id = 0 ; id < 3 ; ++id )
                    {
                        c += srcdata[offset];
                        ++offset;
                    }
                    
                    // Intensity
                    ic = ( ( c / 3 ) >> ( 8 - cmo->mapCtrl[0] ));
                    // Alpha
                    shift = cmo->mapCtrl[0];
                    ic += (( 0xFF >> ( 8 - cmo->mapCtrl[1] )) << shift );
                } break;
                case GX_TL_RGB565:
                {
                    for ( id = 0 ; id < 3 ; ++id )
                    {
                        c   = srcdata[offset];
                        ic += (( c >> ( 8 - cmo->mapCtrl[id] )) << shift );
                        shift += cmo->mapCtrl[id];
                        
                        ++offset;
                    }
                    
                } break;
                case GX_TL_RGB5A3:
                {
                    ic = ( 0x00FFu >> ( 8 - cmo->mapCtrl[0] ));
                    shift = cmo->mapCtrl[0];
                    
                    for ( id = 1 ; id < 4 ; ++id )
                    {
                        c   = srcdata[offset];
                        ic += (( c >> ( 8 - cmo->mapCtrl[id] )) << shift );
                        shift += cmo->mapCtrl[id];
                        
                        ++offset;
                    }
                
                } break;
            }
            
            PlotOnTexture( to, iw, ih, ic );
        }
    }

    // initialize texture object
    GXInitTexObjCI(
        &to->tobj,
        to->data,
        to->width,
        to->height,
        to->format,
        GX_REPEAT,
        GX_REPEAT,
        GX_FALSE,
        cmo->tlutNumber );
    GXInitTexObjLOD(
        &to->tobj,
        GX_LINEAR,
        GX_LINEAR,
        0,
        0,
        0,
        GX_FALSE,
        GX_FALSE,
        GX_ANISO_1 );

    // flush data cache to certainly send all created data into main memory.
    DCFlushRange(to->data, bufferSize);
}

/*---------------------------------------------------------------------------*
    Name:           InitColorMap
    
    Description:    Makes default color map settings
    
    Arguments:      cmo : a pointer to the MyColorMapObj structure
                          to be initialized

    Returns:        none
 *---------------------------------------------------------------------------*/
static void InitColorMap( MyColorMapObj* cmo )
{
    u32 tlutNumBits;
    u32 components, i;

    tlutNumBits = cmo->tlutNumber + 4;

    switch(cmo->tlutFormat)
    {
        case GX_TL_IA8:
            components = 2;
            break;
        case GX_TL_RGB565:
            components = 3;
            break;
        case GX_TL_RGB5A3:
            components = 4;
            break;
    }
        
    for ( i = 0 ; i < 4 ; ++i )
    {
        cmo->mapCtrl[i] = (u8)( ( i < components ) ?
                              (tlutNumBits + i ) / components : 0 );
    }
}

/*---------------------------------------------------------------------------*
    Name:           ChangeColorMap
    
    Description:    Changes the size of a component of color map
    
    Arguments:      cmo : a pointer to the MyColorMapObj structure
                          to be modified
                    cmp_id : specifies which component should be changed
                             (actually the number of bits is incremented)

    Returns:        none
 *---------------------------------------------------------------------------*/
static void ChangeColorMap( MyColorMapObj* cmo, u32 cmp_id )
{
    u32  i, components;
    u8   limits[4];

    switch(cmo->tlutFormat)
    {
        case GX_TL_IA8:
        {
            components = 2;
            limits[0]  = 8;
            limits[1]  = 8;
        } break;
        case GX_TL_RGB565:
        {
            components = 3;
            limits[0]  = 5;
            limits[1]  = 6;
            limits[2]  = 5;
        } break;
        case GX_TL_RGB5A3:
        {
            components = 4;
            limits[0]  = 3;
            limits[1]  = 4;
            limits[2]  = 4;
            limits[3]  = 4;
        } break;
    }

    cmp_id %= components;
    
    if ( cmo->mapCtrl[cmp_id] >= limits[cmp_id] )
        return;

    i = ( cmp_id + 1 ) % components ;
    while ( cmo->mapCtrl[i] <= 1 )
    {
        i = ( i + 1 ) % components;
        
        if ( i == cmp_id )
            break;
    }
    --(cmo->mapCtrl[i]);
    ++(cmo->mapCtrl[cmp_id]);

    return;
}

/*---------------------------------------------------------------------------*
    Name:           LoadColorMap
    
    Description:    Makes all color index data by using given color map
                    setting and load them into TMEM

    Arguments:      cmo : a pointer to the MyColorMapObj structure
                          which provides current color map setting

    Returns:        none
 *---------------------------------------------------------------------------*/
static void LoadColorMap(MyColorMapObj* cmo)
{
    GXTlutObj tlutObj;
    u32       i, offset, ciSize;
    u32       countTbl[4], sizeTbl[4];
    
    ciSize     = 1u << ( cmo->tlutNumber + 4 );
    offset     = 0;
    
    if ( cmo->tlutBuffer != NULL )
    {
        OSFree(cmo->tlutBuffer);
    }
    cmo->tlutBuffer = (u16*)OSAlloc(ciSize * sizeof(u16));

    // reset counter table
    for ( i = 0 ; i < 4 ; ++i )
    {
        countTbl[i] = 0;
        sizeTbl[i]  = ( 1u << cmo->mapCtrl[i] ) - 1;
    }

    // make color data
    do
    {
        u16 data;
    
        if      ( cmo->tlutFormat == GX_TL_IA8 )
        {
            data = (u16)
                 (  ( countTbl[0] * 255 / sizeTbl[0] )
                 + (( countTbl[1] * 255 / sizeTbl[1] ) << 8 ) );
        }
        else if ( cmo->tlutFormat == GX_TL_RGB565 )
        {
            data = GXPackedRGB565(
                       countTbl[2] * 255 / sizeTbl[2],
                       countTbl[1] * 255 / sizeTbl[1],
                       countTbl[0] * 255 / sizeTbl[0] );
        }
        else if ( cmo->tlutFormat == GX_TL_RGB5A3 )
        {
            data = GXPackedRGB5A3(
                       countTbl[3] * 255 / sizeTbl[3],
                       countTbl[2] * 255 / sizeTbl[2],
                       countTbl[1] * 255 / sizeTbl[1],
                       countTbl[0] * 255 / sizeTbl[0] );
        }
        
        cmo->tlutBuffer[offset] = (u16)data;
        ++offset;
        if ( offset >= ciSize )
            break;
        
        for ( i = 0 ; i < 4 ; ++i )
        {
            ++countTbl[i];
            if ( countTbl[i] > sizeTbl[i] )
            {
                countTbl[i] = 0;
            }
            else
                break;
        }
    }
    while ( i < 4 );

    // flush data cache to certainly send all created data into main memory.
    DCFlushRange(cmo->tlutBuffer, ciSize * sizeof(u16));

    // initialize TLUT object
    GXInitTlutObj(
        &tlutObj,
        cmo->tlutBuffer,
        cmo->tlutFormat,
        (u16)ciSize );
    
    // load tlut data into TMEM
    GXLoadTlut(&tlutObj, cmo->tlutNumber);
}

/*---------------------------------------------------------------------------*
    Name:           PlotOnTexture
    
    Description:    Plots a point on texture map
    
    Arguments:      to  : a pointer to the MyTextureObj structure
                          on which a point is plotted
                    x   : x position
                    y   : y position
                    val : color of the point (index)

    Returns:        none
 *---------------------------------------------------------------------------*/
static void PlotOnTexture(MyTextureObj* to, u32 x, u32 y, u32 val)
{
    u32 xp, xb, yp, yb, aw, offset;

    if ( x >= to->width || y >= to->height )
    {
        return;
    }

    switch(to->format)
    {
        case GX_TF_C4 :
        {
            u8 d = (u8)(val & 0x0F);
        
            aw = ( to->width + 7 ) / 8u;
            xp = x % 8;
            xb = x / 8;
            yp = y % 8;
            yb = y / 8;
            offset = ( yb * aw + xb ) * 32 + ( yp * 4 + xp / 2 );
            
            if ( ( xp % 2 ) == 0 )
            {
                u8 m = (u8)( *(to->data + offset) & 0x0F );
                *(to->data + offset) = (u8)( m | ( d * 0x10) );
            }
            else
            {
                u8 m = (u8)( *(to->data + offset) & 0xF0 );
                *(to->data + offset) = (u8)( m | d );
            }
            
        } break;
        case GX_TF_C8 :
        {
            u8 d = (u8)val;
        
            aw = ( to->width + 7 ) / 8u;
            xp = x % 8;
            xb = x / 8;
            yp = y % 4;
            yb = y / 4;
            offset = ( yb * aw + xb ) * 32 + ( yp * 8 + xp );
            *(to->data + offset) = d;
        } break;
        case GX_TF_C14X2 :
        {
            u16 d = (u16)val;
        
            aw = ( to->width + 3 ) / 4u;
            xp = x % 4;
            xb = x / 4;
            yp = y % 4;
            yb = y / 4;
            offset = ( yb * aw + xb ) * 32 + ( yp * 4 + xp ) * 2;
            *(u16*)(to->data + offset) = d;
        } break;
    }

} 

/*---------------------------------------------------------------------------*
    Name:           MyTlutRegionInit
    
    Description:    Initializes and customizes TLUT region configurations.
                    This test doesn't use default TLUT configuration.
    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void MyTlutRegionInit( void )
{
    s32  i;
    u32  addr = 0xC0000;  // The lower half of high bank tmem
    
    for ( i = NUM_REGIONS - 1 ; i >= 0 ; --i )
    {
        GXInitTlutRegion(
            &MyTlutRegions[i],
            addr,
            TlutSizeTable[i].symbol );
        
        addr += TlutSizeTable[i].actualSize;
    }

    GXSetTlutRegionCallback(MyTlutRegionCallback);
}

/*---------------------------------------------------------------------------*
    Name:           MyTlutRegionCallback
    
    Description:    Looks up TLUT region configuration of this test.
                    Used by GX TLUT library.

    Arguments:      idx    region identifier
                    idx = 0 : returns 16 Entry TLUT Region
                        = 1 :         32
                        = 2 :         64
                        = 3 :        128
                        = 4 :        256
                        = 5 :        512
                        = 6 :       1024
                        = 7 :       2048
                        = 8 :       4096

    Returns:        pointer to specified TLUT region
 *---------------------------------------------------------------------------*/
static GXTlutRegion* MyTlutRegionCallback( u32 idx )
{
    ASSERTMSG(idx < NUM_REGIONS, "Invalid TLUT Region index\n");
    
    return &MyTlutRegions[idx];
}

/*---------------------------------------------------------------------------*
    Name:           StatusMessage
    
    Description:    Shows current status
    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void StatusMessage( MyColorMapObj* cmo )
{
    u32 numEntries;
    
    numEntries = 1u << ( cmo->tlutNumber + 4 );
    OSReport("%d entries : ", numEntries);

    switch(cmo->tlutFormat)
    {
        case GX_TL_IA8:
        {
            OSReport(
                "I%dA%d (GX_TL_IA8)\n",
                 cmo->mapCtrl[0],
                 cmo->mapCtrl[1] );
        } break;
        case GX_TL_RGB565:
        {
            OSReport(
                "R%dG%dB%d (GX_TL_RGB565)\n",
                 cmo->mapCtrl[2],
                 cmo->mapCtrl[1],
                 cmo->mapCtrl[0] );
        } break;
        case GX_TL_RGB5A3:
        {
            OSReport(
                "R%dG%dB%dA%d (GX_TL_RGB5A3)\n",
                 cmo->mapCtrl[3],
                 cmo->mapCtrl[2],
                 cmo->mapCtrl[1],
                 cmo->mapCtrl[0] );
        } break;
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
    OSReport("tex-tlut: CI texture with arbitrary TLUT size test\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("L/R triggers : change number of tlut entry.\n");
    OSReport("A button     : change the TLUT format.\n");
    OSReport("Main Stick   : change color map.\n");
    OSReport("******************************************************\n");
    OSReport("\n");
}

/*============================================================================*/
