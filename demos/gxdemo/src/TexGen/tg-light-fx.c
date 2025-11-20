/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-light-fix.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-light-fx.c $
    
    6     5/12/01 1:32a Hirose
    removed obsolete flags
    
    5     7/07/00 5:57p Dante
    PC Compatibility
    
    4     6/21/00 7:10p Hirose
    rewrote the calculation of TexGen2 in order to make them easier to
    understand.
    updated the texture pattern for TexGen1
    
    3     3/24/00 6:56p Hirose
    changed to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:12p Alligator
    move from tests/gx and rename
    
    7     2/24/00 8:22p Hirose
    updated pad control functions to match actual gamepad
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     1/19/00 7:05p Alligator
    fix EPPC errors
    
    4     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    3     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/14/99 6:16p Hirose
    changed to use MTXRowCol macro
    
    9     11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    8     11/17/99 10:02a Hirose
    fixed to avoid compiler warning
    
    7     11/16/99 3:49p Hirose
    modified SetTexGenMtx1
    
    6     11/09/99 3:25p Hirose
    changed algorithm of SetTexGenMtx2
    
    5     11/08/99 3:48p Hirose
    added single frame test
    
    4     11/03/99 8:13p Hirose
    added specular highlight 2D version
    
    3     11/02/99 10:01p Hirose
    added specular effect (1D lookup version)
    
    2     11/02/99 12:02a Hirose
    
    1     11/01/99 2:37p Hirose
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   tg-light-fx
     Some applications by texgen from the normal
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI         3.14159265358979323846F
#define NUM_MODES  3

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

// for lighting
typedef struct
{
    GXColor     lightColor;
    GXColor     ambColor;
    GXColor     matColor;
} MyColorEnvObj;

typedef struct
{
    GXLightObj     lobj;
    s32            theta;
    s32            phi;
    GXBool         mark;
    MyColorEnvObj  colorEnv;
} MyLightObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    MyLightObj     light;
    GXTexObj       texture[NUM_MODES];
    f32            tune[NUM_MODES];
    Mtx            modelCtrl;
    u32            mode;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void DrawLightMark   ( void );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MyLightObj* light, Mtx view );
static void DisableLight    ( void );
static void SetTexGenMtx0   ( MySceneCtrlObj* sc );
static void SetTexGenMtx1   ( MySceneCtrlObj* sc );
static void SetTexGenMtx2   ( MySceneCtrlObj* sc );
static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
   Lighting parameters
 *---------------------------------------------------------------------------*/
MyColorEnvObj MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {
        { 0xFF, 0xFF, 0xFF, 0xFF }, // Light color 0
        { 0x80, 0x80, 0x80, 0xFF }, // Ambient 0
        { 0xFF, 0xFF, 0x00, 0xFF }  // Material 0
    },
    {
        { 0xFF, 0xFF, 0xFF, 0xFF }, // Light color 1
        { 0x40, 0x40, 0x40, 0xFF }, // Ambient 1
        { 0x00, 0x00, 0xFF, 0xFF }  // Material 1
    },
    {
        { 0xFF, 0xFF, 0xFF, 0xFF }, // Light color 2
        { 0x40, 0x40, 0x40, 0xFF }, // Ambient 2
        { 0xFF, 0x00, 0x20, 0xFF }  // Material 2
    }
};

/*---------------------------------------------------------------------------*
   Test texture data
 *---------------------------------------------------------------------------*/
static u8 TestTexMap0[] ATTRIBUTE_ALIGN(32) =
{
    // IA4 format 16x4
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF0, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF0, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF0, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF0, 0xF0,

    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00
};

static u8 TestTexMap1[] ATTRIBUTE_ALIGN(32) =
{
    // IA8 format 16x4
    0x00, 0xFF, 0x00, 0xFF, 0x08, 0xFF, 0x20, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x08, 0xFF, 0x20, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x08, 0xFF, 0x20, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x08, 0xFF, 0x20, 0xFF,

    0x38, 0xFF, 0x50, 0xFF, 0x68, 0xFF, 0x80, 0xFF,
    0x38, 0xFF, 0x50, 0xFF, 0x68, 0xFF, 0x80, 0xFF,
    0x38, 0xFF, 0x50, 0xFF, 0x68, 0xFF, 0x80, 0xFF,
    0x38, 0xFF, 0x50, 0xFF, 0x68, 0xFF, 0x80, 0xFF,
    
    0x98, 0xFF, 0xB0, 0xFF, 0xC8, 0xFF, 0xE0, 0xFF,
    0x98, 0xFF, 0xB0, 0xFF, 0xC8, 0xFF, 0xE0, 0xFF,
    0x98, 0xFF, 0xB0, 0xFF, 0xC8, 0xFF, 0xE0, 0xFF,
    0x98, 0xFF, 0xB0, 0xFF, 0xC8, 0xFF, 0xE0, 0xFF,
    
    0xF0, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xF0, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xF0, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xF0, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static u8 TestTexMap2[] ATTRIBUTE_ALIGN(32) =
{
    // IA4 format 16x16
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x1F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x1F, 0x2F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x2F, 0x5F,

    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x1F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x2F, 0x1F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x5F, 0x2F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,


    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x3F, 0x8F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x3F, 0x6F, 0xBF,
    0x0F, 0x0F, 0x1F, 0x2F, 0x3F, 0x6F, 0xAF, 0xDF,
    0x0F, 0x1F, 0x2F, 0x5F, 0x8F, 0xBF, 0xDF, 0xFF,

    0x8F, 0x3F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xBF, 0x6F, 0x3F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xDF, 0xAF, 0x6F, 0x3F, 0x2F, 0x1F, 0x0F, 0x0F,
    0xFF, 0xDF, 0xBF, 0x8F, 0x5F, 0x2F, 0x1F, 0x0F,


    0x0F, 0x1F, 0x2F, 0x5F, 0x8F, 0xBF, 0xDF, 0xFF,
    0x0F, 0x0F, 0x1F, 0x2F, 0x3F, 0x6F, 0xAF, 0xDF,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x3F, 0x6F, 0xBF,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x3F, 0x8F,

    0xFF, 0xDF, 0xBF, 0x8F, 0x5F, 0x2F, 0x1F, 0x0F,
    0xDF, 0xAF, 0x6F, 0x3F, 0x2F, 0x1F, 0x0F, 0x0F,
    0xBF, 0x6F, 0x3F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x8F, 0x3F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,


    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x2F, 0x5F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x1F, 0x2F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x1F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,

    0x5F, 0x2F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x2F, 0x1F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x1F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
};

static u16 TestTexWidth[NUM_MODES] =
{
    16, 16, 16
};

static u16 TestTexHeight[NUM_MODES] =
{
    4,  4,  16
};

static GXTexFmt TestTexFormat[NUM_MODES] =
{
    GX_TF_IA4, GX_TF_IA8, GX_TF_IA4
};

static u8* TestTexData[NUM_MODES] =
{
    TestTexMap0,
    TestTexMap1,
    TestTexMap2
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 2500.0F }, // location
    { 0.0F, 1.0F,    0.0F }, // up
    { 0.0F, 0.0F,    0.0F }, // tatget
    -320.0F, // left
    240.0F,  // top
    1000.0F, // near
    5000.0F  // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;    // scene control parameters

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);    // Init the OS, game pad, graphics and video.
    
    DrawInit(&SceneCtrl);       // Initialize vertex formats and scene parameters.

    PrintIntro();  // Print demo directions
         
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);    // Draw the model.
        DEMODoneRender();
        DEMOPadRead();           // Update pad status.
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
    GXColor bgColor = { 0x40, 0x40, 0x40, 0xFF };
    u32     i;

    GXSetCopyClear(bgColor, 0xFFFFFF);

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // textures
    for ( i = 0 ; i < NUM_MODES ; ++i )
    {
        GXInitTexObj(
            &sc->texture[i],
            TestTexData[i],
            TestTexWidth[i],
            TestTexHeight[i],
            TestTexFormat[i],
            GX_CLAMP, // s
            GX_CLAMP, // t
            GX_FALSE );
        GXInitTexObjLOD(
            &sc->texture[i],
            GX_LINEAR,
            GX_LINEAR,
            0,
            0,
            0,
            GX_FALSE,
            GX_FALSE,
            GX_ANISO_1 );
    }


    // Default scene parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // light parameters
    sc->light.theta = 0;
    sc->light.phi   = 0;
    sc->light.mark  = GX_FALSE;

    // tuning parameters
    for ( i = 0 ; i < NUM_MODES ; ++i )
    {
        sc->tune[i] = 0.5F;
    }

    // mode
    sc->mode = 0;

    // model control matrix
    MTXIdentity(sc->modelCtrl);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mv, mvi, mr, ms, mm;

    // enable lighting
    sc->light.colorEnv = MyColors[sc->mode];
    SetLight(&sc->light, sc->cam.view);
    // set texture environments
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0); 
    GXSetTevOp(GX_TEVSTAGE0, GX_DECAL);
    GXSetNumTexGens(1);
	GXSetNumChans(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0);
    GXLoadTexObj(&sc->texture[sc->mode], GX_TEXMAP0);
    
    // set texture coord generation matrix
    switch ( sc->mode )
    {
      case 0:
        SetTexGenMtx0(sc);
        break;
      case 1:
        SetTexGenMtx1(sc);
        break;
      case 2:
        SetTexGenMtx2(sc);
        break;
    }

    // draw a model
    MTXConcat(sc->cam.view, sc->modelCtrl, mm);
    MTXInverse(mm, mvi);
    MTXTranspose(mvi, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    
    MTXScale(ms, 400.0F, 400.0F, 400.0F);
    MTXConcat(mm, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    GXDrawTorus(0.20F, 16, 32);

    MTXScale(ms, 200.0F, 200.0F, 200.0F);
    MTXConcat(mm, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    GXDrawSphere1(3);

    
    if ( sc->light.mark )
    {
        // disable lighting
        DisableLight();
        // set texture environments
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXSetNumTexGens(0);

        // draw a light mark
        MTXRotDeg(mr, 'y', sc->light.theta);
        MTXConcat(sc->cam.view, mr, mv);
        MTXRotDeg(mr, 'x', - sc->light.phi);
        MTXConcat(mv, mr, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
        DrawLightMark();
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
    Mtx  mrx, mry;
    u16  button, down;
    u32  pr = 0;

    // PAD
    button = DEMOPadGetButton(0);
    down   = DEMOPadGetButtonDown(0);

    // Light Position Calculation
    sc->light.theta += ( DEMOPadGetStickX(0) / 24 );
    sc->light.theta = sc->light.theta % 360;
        
    sc->light.phi += ( DEMOPadGetStickY(0) / 24 );
    Clamp(sc->light.phi, -90, 90);


    // Model Rotation Calculation
    MTXRotDeg(mry, 'x', -(f32)DEMOPadGetSubStickY(0) / 24.0F);
    MTXRotDeg(mrx, 'y', (f32)DEMOPadGetSubStickX(0) / 24.0F);
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);


    // Hide light direction mark
    if ( button & PAD_BUTTON_B )
    {
        sc->light.mark = GX_FALSE;
    }
    else
    {
        sc->light.mark = GX_TRUE;
    }

    // Tuning parameters
    if ( down & PAD_TRIGGER_L )
    {
        sc->tune[sc->mode] -= 0.1F;
    }
    if ( down & PAD_TRIGGER_R )
    {
        sc->tune[sc->mode] += 0.1F;
    }
    Clamp(sc->tune[sc->mode], 0.0F, 1.0F);

    // change mode
    if ( down & PAD_BUTTON_A )
    {
        sc->mode = ( sc->mode + 1 ) % NUM_MODES;
    }
    
}

/*---------------------------------------------------------------------------*
    Name:           DrawLightMark
    
    Description:    Draws a mark which shows position of the light.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawLightMark( void )
{
    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GXBegin(GX_LINES, GX_VTXFMT0, 8);
        GXPosition3f32(500.0F, 500.0F, -500.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 1000.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(500.0F, -500.0F, -500.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 1000.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(-500.0F, 500.0F, -500.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 1000.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(-500.0F, -500.0F, -500.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 1000.0F);
        GXColor4u8(0, 255, 255, 255);
    GXEnd();
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
    
    Description:    Sets up lights and lighting channel parameters
                    
    Arguments:      light : pointer to a MyLightObj structure
                    view  : view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetLight( MyLightObj* light, Mtx view )
{
    Vec lpos;
    f32 theta, phi;

    // Light Position
    theta = (f32)light->theta * PI / 180.0F;
    phi   = (f32)light->phi   * PI / 180.0F;
    lpos.x = 1000.0F * cosf(phi) * sinf(theta);
    lpos.y = 1000.0F * sinf(phi);
    lpos.z = 1000.0F * cosf(phi) * cosf(theta);
    
    // Convert light position into view space
    MTXMultVec(view, &lpos, &lpos);

    GXInitLightPos(&light->lobj, lpos.x, lpos.y, lpos.z);
    GXInitLightColor(&light->lobj, light->colorEnv.lightColor);
    GXLoadLightObjImm(&light->lobj, GX_LIGHT0);

    // Lighting channel
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);
    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0A0, light->colorEnv.ambColor);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, light->colorEnv.matColor);

}

/*---------------------------------------------------------------------------*
    Name:           DisableLight
    
    Description:    Disables lighting
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DisableLight( void )
{
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,  // disable channel
        GX_SRC_VTX,  // amb source
        GX_SRC_VTX,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_NONE,  // diffuse function
        GX_AF_NONE);
}

/*---------------------------------------------------------------------------*
    Name:           SetTexGenMtx0
    
    Description:    Calculate and set texture coord generation matrix
                    for drawing psuedo outline.
                    
    Arguments:      sc : a pointer to MySceneCtrlObj structure
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetTexGenMtx0( MySceneCtrlObj* sc )
{
    Mtx  mt, ms;
    f32  scale;

    // Project object normals from side view
    MTXRotDeg(mt, 'Y', -90);
    MTXConcat(mt, sc->cam.view, mt);
    MTXConcat(mt, sc->modelCtrl, mt);
    MTXInverse(mt, ms);
    MTXTranspose(ms, mt);
    
    // Scaling and bias
    scale = 0.5F - 0.25F * sc->tune[sc->mode];
    MTXScale(ms, scale, 0.5F, 1.0F);
    MTXConcat(ms, mt, mt);
    MTXTrans(ms, 0.5F, 0.5F, 0.0F);
    MTXConcat(ms, mt, mt);

    // Load into the hardware
    GXLoadTexMtxImm(mt, GX_TEXMTX0, GX_MTX2x4);
}

/*---------------------------------------------------------------------------*
    Name:           SetTexGenMtx1
    
    Description:    Calculate and set texture coord generation matrix
                    for specular highlight (1D lookup version).
                    
    Arguments:      sc : a pointer to MySceneCtrlObj structure
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetTexGenMtx1( MySceneCtrlObj* sc )
{
    Mtx  mt, ms;
    f32  theta, phi, scale;
    Vec  vview = { 0.0F, 0.0F, -1.0F };
    Vec  vlight, vhalf;

    // Calculate light direction
    theta = sc->light.theta * PI / 180.0F;
    phi   = sc->light.phi   * PI / 180.0F;
    vlight.x = - cosf(phi) * sinf(theta);
    vlight.y = - sinf(phi);
    vlight.z = - cosf(phi) * cosf(theta);

    // Convert light direction into view space
    MTXMultVecSR(sc->cam.view, &vlight, &vlight);

    // Calculate the half angle
    VECHalfAngle(&vlight, &vview, &vhalf);

    // Modelview translation
    MTXConcat(sc->cam.view, sc->modelCtrl, mt);
    MTXInverse(mt, ms);
    MTXTranspose(ms, mt);

    // Make a matrix which generates s-coord by half angle.
    // actually generated coord is 1D
    MTXIdentity(ms);
    MTXRowCol(ms, 0, 0) = vhalf.x;
    MTXRowCol(ms, 0, 1) = vhalf.y;
    MTXRowCol(ms, 0, 2) = vhalf.z;
    MTXConcat(ms, mt, mt);
    
    // Scaling and bias
    scale = 14.0F + 20.0F * sc->tune[sc->mode];
    MTXScale(ms, scale, 1.0F, 1.0F);
    MTXConcat(ms, mt, mt);
    MTXTrans(ms,  1.0F - scale, 0.0F, 0.0F);
    MTXConcat(ms, mt, mt);

    // Load into the hardware
    GXLoadTexMtxImm(mt, GX_TEXMTX0, GX_MTX2x4);
}

/*---------------------------------------------------------------------------*
    Name:           SetTexGenMtx2
    
    Description:    Calculate and set texture coord generation matrix
                    for specular highlight (2D lookup version).
                    
    Arguments:      sc : a pointer to MySceneCtrlObj structure
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetTexGenMtx2( MySceneCtrlObj* sc )
{
    Mtx  mt, mv, ms;
    f32  theta, phi, scale, dotp, r;
    Vec  vlight, vhalf, vaxis;
    Vec  vview = { 0.0F, 0.0F, 1.0F };

    // Calculate light direction
    theta = sc->light.theta * PI / 180.0F;
    phi   = sc->light.phi   * PI / 180.0F;
    vlight.x = cosf(phi) * sinf(theta);
    vlight.y = sinf(phi);
    vlight.z = cosf(phi) * cosf(theta);

    // Convert light direction into view space
    MTXMultVecSR(sc->cam.view, &vlight, &vlight);

    // Calculate the half angle
    dotp = VECDotProduct(&vlight, &vview);
    if ( dotp == -1.0F )
    {
        // Singular potnt ( vview + vlight = 0 )
        MTXScale(mt, 0.0F, 0.0F, 0.0F); // zero matrix
        
        GXLoadTexMtxImm(mt, GX_TEXMTX0, GX_MTX2x4);
        return;
    }
    VECHalfAngle(&vlight, &vview, &vhalf);
    // The obtained half-angle vector directs an opposite side
    vhalf.x = -vhalf.x;
    vhalf.y = -vhalf.y;
    vhalf.z = -vhalf.z;

    // Modelview translation
    MTXConcat(sc->cam.view, sc->modelCtrl, mv);
    MTXInverse(mv, ms);
    MTXTranspose(ms, mv);

    // Rotation
    if ( dotp == 1.0F )
    {
        MTXIdentity(ms);
    }
    else
    {
        VECCrossProduct(&vhalf, &vview, &vaxis);  // rotation axis
        r = acosf(VECDotProduct(&vview, &vhalf)); // rotation amounts
        MTXRotAxisRad(ms, &vaxis, r);
    }
    MTXConcat(ms, mv, mt);
    
    // Scaling and bias
    scale = 2.0F * sc->tune[sc->mode] + 1.5F;
    MTXScale(ms, scale, scale, 1.0F);
    MTXConcat(ms, mt, mt);
    MTXTrans(ms, 0.5F, 0.5F, 0.0F);
    MTXConcat(ms, mt, mt);

    // Load into the hardware
    GXLoadTexMtxImm(mt, GX_TEXMTX0, GX_MTX2x4);
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
    OSReport("**********************************************\n");
    OSReport("tg-light-fx: some applications using texgen\n");
    OSReport("             from normals\n");
    OSReport("**********************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main stick   : Move Light Position\n");
    OSReport("Sub  stick   : Rotate the Moddel\n");
    OSReport("L/R Triggers : Tune parameter\n");
    OSReport("A Button     : Change texgen mode\n");
    OSReport("B Button     : Hide light direction mark\n");
    OSReport("**********************************************\n");
}

/*============================================================================*/
