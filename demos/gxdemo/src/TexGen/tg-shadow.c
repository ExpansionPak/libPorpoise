/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-shadow.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-shadow.c $
    
    10    5/12/01 2:25a Hirose
    removed obsolete flags
    
    9     5/21/00 12:14a Hirose
    updated control feature / some parameters
    
    8     5/02/00 4:04p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    7     3/24/00 6:56p Hirose
    changed to use DEMOPad library
    
    6     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    5     3/17/00 6:31p Carl
    Changed flag from INT_TEX_COORDS to BUG_NO_8b_SCALE
    
    4     3/13/00 3:47p Carl
    Changed to FP tex coords.
    
    3     3/07/00 6:51p Carl
    Fixed background problem.
    Changed pipe sync code.
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:12p Alligator
    move from tests/gx and rename
    
    10    3/03/00 4:21p Alligator
    integrate with ArtX source
    
    9     2/24/00 8:22p Hirose
    updated pad control functions to match actual gamepad
    
    8     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    7     2/14/00 6:09p Hirose
    fixed to avoid multiple call of DEMODoneRender on singleframe test.
    
    6     2/12/00 5:16p Alligator
    Integrate ArtX source tree changes
    
    5     1/25/00 2:58p Carl
    Changed to standardized end of test message
    
    4     1/24/00 6:46p Carl
    Removed #ifdef EPPC stuff
    
    3     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    10    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    9     11/15/99 4:49p Yasu
    Change datafile name
    
    8     11/09/99 7:48p Hirose
    added GXSetNumTexGens
    
    7     11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    6     10/29/99 6:31p Hirose
    
    4     10/22/99 4:39p Yasu
    Add GXSetTevStages and GXSetTevOrder
    
    3     10/21/99 3:42p Hirose
    changed to use box-filtered texture for shadow image
    
    2     10/21/99 11:13a Hirose
    added single frame test
    
    1     10/21/99 10:39a Hirose
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   tg-shadow
     Generating shadow by texture projection
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI               3.14159265358979323846F
#define MAX_Z            0x00ffffff // max value of Z buffer
#define NUM_GRIDS        7

// Since using box (mipmap) filter, actual texture size becomes
// quarter of source image size on framebuffer
#define SHADOW_TEX_SIZE  64
#define SHADOW_SRC_SIZE  (SHADOW_TEX_SIZE * 2)

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
    s32           theta;
    s32           phi;
    f32           distance;
} MyCameraObj;

// for light
typedef struct
{
    GXLightObj    lobj;
    MyCameraObj   cam;
} MyLightObj;

// for entire scene control
typedef struct
{
    MyCameraObj   cam;
    MyLightObj    light;
    GXTexObj      shadowTex;
    u8*           shadowTexData;
    s32           modelZ;
    s32           modelRot;
    u32           anim;
    u16           screen_wd;
    u16           screen_ht;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void DrawFloor       ( void );
static void GetTexProjMtx   ( MyCameraObj* cam, Mtx mp );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MyLightObj* light, Mtx v );
static void DisableLight    ( void );
static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
   Lighting parameters
 *---------------------------------------------------------------------------*/
#define COL_WHITE   MyColors[0]
#define COL_BG      MyColors[1]
#define COL_AMBIENT MyColors[2]
#define COL_LIGHT   MyColors[3]
#define COL_SHADOW  MyColors[4]
#define COL_TORUS   MyColors[5]
#define COL_FLOOR   MyColors[6]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {0xff, 0xff, 0xff, 0xff},  // white
    {0x20, 0x20, 0x20, 0x00},  // background
    {0x40, 0x40, 0x40, 0xff},  // ambient
    {0xc0, 0xc0, 0xc0, 0xff},  // light color
    {0x00, 0x00, 0x00, 0xc0},  // shadow color
    {0x80, 0xc0, 0xff, 0xff},  // torus
    {0xf0, 0xf0, 0xf0, 0xff}   // floor
};

// fixed normal vector
static f32 FixedNormal[] ATTRIBUTE_ALIGN(32) =
{
    1.0F, 0.0F, 0.0F,  // X
    0.0F, 1.0F, 0.0F,  // Y
    0.0F, 0.0F, 1.0F   // Z
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 1500.0F, 1500.0F, 400.0F }, // location
    { 0.0F, 0.0F, 1.0F }, // up
    { 0.0F, 0.0F, 0.0F }, // target
    -160.0F,  // left
    120.0F,   // top   
    300.0F,   // near
    5000.0F   // far
};

static CameraConfig DefaultLightCamera =
{
    { 1500.0F, 1500.0F, 2000.0F }, // location
    { 0.0F, 0.0F, 1.0F }, // up
    { 0.0F, 0.0F, 400.0F }, // target
    -32.0F,   // left
     32.0F,   // top   
    150.0F,   // near
    10000.0F   // far
};

/*---------------------------------------------------------------------------*
   Global Variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;    // scene control parameters
static TEXPalettePtr   MyTplObj = NULL;
static GXTexObj        MyFloorTexObj;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);     // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl); // Initialize vertex formats and scene parameters etc.

    PrintIntro();  // Print demo directions
    
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);    // Draw the model.
        DEMODoneRender();
        DEMOPadRead();
        AnimTick(&SceneCtrl);    // Update animation.
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format, texture and
                    default scene parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{ 
    GXRenderModeObj *rmp;
    u32    size;

    // get framebuffer size of current rendering mode
    rmp = DEMOGetRenderModeObj();
    sc->screen_wd = rmp->fbWidth;
    sc->screen_ht = rmp->efbHeight;

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
#ifndef BUG_NO_8b_SCALE
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S8, 4);
#else
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 4);
#endif
    // Set pixel format with alpha
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    GXSetAlphaUpdate(GX_ENABLE);

    GXSetCopyClear(COL_BG, MAX_Z);
    GXSetCullMode(GX_CULL_NONE);
    
    // Need to clear background by specified color
    // since pixelfmt is different than DEMOInit default.
    // The clear operation can be done by dummy copy.
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);

    // Texture object for the floor
    TEXGetPalette(&MyTplObj, "gxTextrs.tpl");
    TEXGetGXTexObjFromPalette(MyTplObj, &MyFloorTexObj, 6);

    // Default scene parameter settings

    // camera
    sc->cam.cfg      = DefaultCamera;
    sc->cam.theta    = 45;
    sc->cam.phi      = 25;
    sc->cam.distance = 2500.0F;

    // light projection camera
    sc->light.cam.cfg      = DefaultLightCamera;
    sc->light.cam.theta    = 0;
    sc->light.cam.phi      = 60;
    sc->light.cam.distance = 3000.0F;

    // model control parameters
    sc->modelZ   = 0;
    sc->modelRot = 0;
    sc->anim     = 1;
    
    // Texture object for shadow
    size = GXGetTexBufferSize(
               SHADOW_TEX_SIZE,
               SHADOW_TEX_SIZE,
               GX_TF_IA4,
               GX_FALSE,
               0 );
    sc->shadowTexData = OSAlloc(size);

    GXInitTexObj(
        &sc->shadowTex,
        sc->shadowTexData,
        SHADOW_TEX_SIZE,
        SHADOW_TEX_SIZE,
        GX_TF_IA4,
        GX_CLAMP,
        GX_CLAMP,
        GX_FALSE );
    GXInitTexObjLOD(
        &sc->shadowTex,
        GX_LINEAR,
        GX_LINEAR,
        0.0F,
        0.0F,
        0.0F,
        GX_FALSE,
        GX_FALSE,
        GX_ANISO_1 );

}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    static u16  count = 0;
    Mtx  ms, mt, mv, mvi, mtr;
    f32  tr, zt;
    
    GXInvalidateTexAll();

    // Modelview matrix for torus object
    zt = 100.0F * sinf(sc->modelZ * PI / 180.0F) + 500.0F;
    MTXTrans(mt, 100.0F, 100.0F, zt);
    MTXScale(ms, 300.0F, 300.0F, 300.0F);
    MTXConcat(mt, ms, mtr);
    MTXRotDeg(ms, 'z', sc->modelRot);
    MTXConcat(mtr, ms, mtr);
    MTXRotDeg(ms, 'x', 90);
    MTXConcat(mtr, ms, mtr);

    //-------------------------------------------
    //    Make Shadow Texture
    //-------------------------------------------

    // Set viewport/camera/light for making shadow texture
    GXSetViewport(0, 0, SHADOW_SRC_SIZE, SHADOW_SRC_SIZE, 0.0F, 1.0F);
    SetCamera(&sc->light.cam);
    DisableLight();
    // Set tev to use only one vertex color
    GXSetNumChans(1);
    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    // Draw torus object for shadow texture
    GXSetChanMatColor(GX_COLOR0A0, COL_SHADOW);

    MTXConcat(sc->light.cam.view, mtr, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    GXDrawTorus(0.20F, 12, 16);

    //-------------------------------------------
    //    Copy Shadow Image into Texture
    //-------------------------------------------
    
    // Don't need to flush pixel pipe here since PE does the copy
    // and it can't start the copy until it's done rendering anyway

    // Copy shadow image into texture
    GXSetTexCopySrc(0, 0, SHADOW_SRC_SIZE, SHADOW_SRC_SIZE);
    GXSetTexCopyDst(SHADOW_TEX_SIZE, SHADOW_TEX_SIZE, GX_TF_IA4, GX_TRUE);
    GXCopyTex(sc->shadowTexData, GX_TRUE);

    // Wait for finishing the copy task in the graphics pipeline
    GXPixModeSync();

    //-------------------------------------------
    //    Make Main Image
    //-------------------------------------------

    // Set viewport/camera/light for the main image
    GXSetViewport(0, 0, sc->screen_wd, sc->screen_ht, 0.0F, 1.0F);
    SetCamera(&sc->cam);
    SetLight(&sc->light, sc->cam.view);

    // for the torus object
    GXSetChanMatColor(GX_COLOR0A0, COL_TORUS);

    MTXConcat(sc->cam.view, mtr, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    // Draw a torus
    GXDrawTorus(0.20F, 12, 16);

    // Now get ready to draw floor
    // Set tev to display multi texture
    GXSetNumTevStages(2);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOp(GX_TEVSTAGE1, GX_DECAL);
    
    // Set texgen (TEXCOORD0 is default setting)
    GXSetNumTexGens(2);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);

    // Textures for floor panels
    GXLoadTexObj(&MyFloorTexObj, GX_TEXMAP0);
    GXLoadTexObj(&sc->shadowTex, GX_TEXMAP1);
    GXSetChanMatColor(GX_COLOR0A0, COL_FLOOR);

    // Modelview matrix for floor panels
    tr = -(f32)NUM_GRIDS / 2.0F;
    MTXScale(ms, 200.0F, 200.0F, 50.0F);
    MTXTrans(mt, tr, tr, 0.0F);
    MTXConcat(ms, mt, ms);
    MTXConcat(sc->cam.view, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    // Projection matrix for texture coordinate generation
    GetTexProjMtx(&sc->light.cam, mv);
    MTXConcat(mv, ms, mv);
    GXLoadTexMtxImm(mv, GX_TEXMTX1, GX_MTX3x4);

    // Draw floor panels
    DrawFloor();
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    // Camera position
    sc->cam.theta += ( DEMOPadGetStickX(0) / 24 );
    Clamp(sc->cam.theta, 0, 90);
    sc->cam.phi   += ( DEMOPadGetStickY(0) / 24 );
    Clamp(sc->cam.phi, 10, 60);

    // Light position
    sc->light.cam.theta += ( DEMOPadGetSubStickX(0) / 24 );
    if ( sc->light.cam.theta > 360 )
    {
        sc->light.cam.theta -= 360;
    }
    if ( sc->light.cam.theta < 0 )
    {
        sc->light.cam.theta += 360;
    }
    sc->light.cam.phi += ( DEMOPadGetSubStickY(0) / 24 );
    Clamp(sc->light.cam.phi, 15, 85);
    
    // Moves the model automatically
    if (sc->anim)
    {
        sc->modelZ += 5;
        if ( sc->modelZ > 360 )
        {
            sc->modelZ -= 360;
        }
        
        sc->modelRot += 8;
        if ( sc->modelRot > 360 )
        {
            sc->modelRot -= 360;
        }
    }

    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_A )
    {
        sc->anim = 1 - sc->anim;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawFloor
    
    Description:    Draws the floor which contains some steps
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawFloor( void )
{
    s8    x, y, z;

    // set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // normal array
    GXSetArray(GX_VA_NRM, FixedNormal, 3 * sizeof(f32));

    z = 0;
    for ( y = 0 ; y < NUM_GRIDS ; ++y )
    {
        GXBegin(GX_QUADS, GX_VTXFMT0, NUM_GRIDS * 12);
            for ( x = 0 ; x < NUM_GRIDS ; ++x )
            {
                z = (s8)(NUM_GRIDS - x - y);
            
#ifndef BUG_NO_8b_SCALE
                // Z side
                GXPosition3s8(x, y, z);
                GXNormal1x8(2);
                GXTexCoord2s8(0, 0);
                GXPosition3s8((s8)(x+1), y, z);
                GXNormal1x8(2);
                GXTexCoord2s8(0x10, 0);
                GXPosition3s8((s8)(x+1), (s8)(y+1), z);
                GXNormal1x8(2);
                GXTexCoord2s8(0x10, 0x10);
                GXPosition3s8(x, (s8)(y+1), z);
                GXNormal1x8(2);
                GXTexCoord2s8(0, 0x10);
                
                // X side
                GXPosition3s8((s8)(x+1), y, z);
                GXNormal1x8(0);
                GXTexCoord2s8(0, 0);
                GXPosition3s8((s8)(x+1), (s8)(y+1), z);
                GXNormal1x8(0);
                GXTexCoord2s8(0x10, 0);
                GXPosition3s8((s8)(x+1), (s8)(y+1), (s8)(z-1));
                GXNormal1x8(0);
                GXTexCoord2s8(0x10, 0x8);
                GXPosition3s8((s8)(x+1), y, (s8)(z-1));
                GXNormal1x8(0);
                GXTexCoord2s8(0, 0x8);

                // Y side
                GXPosition3s8((s8)(x+1), (s8)(y+1), z);
                GXNormal1x8(1);
                GXTexCoord2s8(0, 0);
                GXPosition3s8(x, (s8)(y+1), z);
                GXNormal1x8(1);
                GXTexCoord2s8(0x10, 0);
                GXPosition3s8(x, (s8)(y+1), (s8)(z-1));
                GXNormal1x8(1);
                GXTexCoord2s8(0x10, 0x8);
                GXPosition3s8((s8)(x+1), (s8)(y+1), (s8)(z-1));
                GXNormal1x8(1);
                GXTexCoord2s8(0, 0x8);
#else
                // Z side
                GXPosition3s8(x, y, z);
                GXNormal1x8(2);
                GXTexCoord2s16(0, 0);
                GXPosition3s8((s8)(x+1), y, z);
                GXNormal1x8(2);
                GXTexCoord2s16(0x10, 0);
                GXPosition3s8((s8)(x+1), (s8)(y+1), z);
                GXNormal1x8(2);
                GXTexCoord2s16(0x10, 0x10);
                GXPosition3s8(x, (s8)(y+1), z);
                GXNormal1x8(2);
                GXTexCoord2s16(0, 0x10);
                
                // X side
                GXPosition3s8((s8)(x+1), y, z);
                GXNormal1x8(0);
                GXTexCoord2s16(0, 0);
                GXPosition3s8((s8)(x+1), (s8)(y+1), z);
                GXNormal1x8(0);
                GXTexCoord2s16(0x10, 0);
                GXPosition3s8((s8)(x+1), (s8)(y+1), (s8)(z-1));
                GXNormal1x8(0);
                GXTexCoord2s16(0x10, 0x8);
                GXPosition3s8((s8)(x+1), y, (s8)(z-1));
                GXNormal1x8(0);
                GXTexCoord2s16(0, 0x8);

                // Y side
                GXPosition3s8((s8)(x+1), (s8)(y+1), z);
                GXNormal1x8(1);
                GXTexCoord2s16(0, 0);
                GXPosition3s8(x, (s8)(y+1), z);
                GXNormal1x8(1);
                GXTexCoord2s16(0x10, 0);
                GXPosition3s8(x, (s8)(y+1), (s8)(z-1));
                GXNormal1x8(1);
                GXTexCoord2s16(0x10, 0x8);
                GXPosition3s8((s8)(x+1), (s8)(y+1), (s8)(z-1));
                GXNormal1x8(1);
                GXTexCoord2s16(0, 0x8);
#endif
            }
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           GetTexProjMtx
    
    Description:    sets texture coord generation matrix for shadow texture
                    
    Arguments:      cam : pointer to the MyCameraObj structure
                    mp  : a matrix the result should be set
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void GetTexProjMtx( MyCameraObj* cam, Mtx mp )
{
    Mtx proj;

    MTXLightFrustum(
        proj,
        - (cam->cfg.top), // t = -y in projected texture 
        cam->cfg.top,
        cam->cfg.left,
        - (cam->cfg.left),
        cam->cfg.znear,
        0.5F,
        0.5F,
        0.5F,
        0.5F );
    MTXConcat(proj, cam->view, mp);
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    set view matrix and load projection matrix into hardware
                    
    Arguments:      cam : pointer to the MyCameraObj structure
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetCamera( MyCameraObj* cam )
{
    f32  r_theta, r_phi;
    
    r_theta = (f32)cam->theta * PI / 180.0F;
    r_phi   = (f32)cam->phi   * PI / 180.0F;

    cam->cfg.location.x =
        cam->distance * cosf(r_theta) * cosf(r_phi);
    cam->cfg.location.y =
        cam->distance * sinf(r_theta) * cosf(r_phi);
    cam->cfg.location.z =
        cam->distance * sinf(r_phi);

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
    Name:           SetLight
    
    Description:    Set up lights and lighting channel parameters
                    
    Arguments:      light : pointer to a MyLightObj structure
                    v     : view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetLight( MyLightObj* light, Mtx v )
{
    Vec lpos = light->cam.cfg.location;
    
    // Multipled by view matrix
    MTXMultVec(v, &lpos, &lpos);

    GXInitLightPos(&light->lobj, lpos.x, lpos.y, lpos.z);
    GXInitLightColor(&light->lobj, COL_LIGHT);
    GXLoadLightObjImm(&light->lobj, GX_LIGHT0);

    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE );
    // set up material color
    GXSetChanAmbColor(GX_COLOR0A0, COL_AMBIENT);
}

/*---------------------------------------------------------------------------*
    Name:           DisableLight
    
    Description:    Disables lighting
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void DisableLight( void )
{
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,  // disable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_NONE,  // diffuse function
        GX_AF_NONE);
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
    OSReport("tg-shadow: generating shadow by texture projection\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main stick : Move the camera\n");
    OSReport("Sub  stick : Move the light\n");
    OSReport("A Button   : Stop/Start animation\n");
    OSReport("******************************************************\n");
    OSReport("\n\n");
}

/*============================================================================*/
