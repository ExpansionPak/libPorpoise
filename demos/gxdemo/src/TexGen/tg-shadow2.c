/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-shadow2.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-shadow2.c $
    
    12    5/12/01 2:25a Hirose
    removed obsolete flags
    
    11    11/01/00 10:30p Hirose
    removed GXSetTevClampMode call from GX revision 2 target builds
    
    10    11/01/00 5:38p Hirose
    added codes for GX revision 2
    
    9     7/20/00 3:40p Hirose
    added overview
    
    8     5/19/00 2:41a Hirose
    fixed TEV clamp mode state issue 
    
    7     5/02/00 4:04p Hirose
    updated to call DEMOGetCurrentBuffer
    some small changes
    
    6     4/13/00 8:36p Hirose
    updated some function names and parameters
    
    5     4/07/00 7:28p Hirose
    fix to match GXSetCopyFilter prototype update
    
    4     4/07/00 4:23p Hirose
    changed scale value for TEV to avoid clamp HW bug
    avoid using vfilter while doing framebuffer-to-texture copy
    
    3     4/05/00 7:23p Hirose
    added some control features
    
    2     4/03/00 11:50p Hirose
    
    1     4/02/00 10:09p Hirose
    initial protorype
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   tg-shadow2
     Full-scene shadow method by object ID map
 *---------------------------------------------------------------------------*

   Overview of this method:

       (1st. pass) Render the entire scene from the light's point
       of view. At this time, paint each object by its individual
       ID number (as gray scale value of the color).
       
       Then copy the result framebuffer into a texture (shadow ID map).
       The framebuffer is cleared at the same time.
       
       (2nd. pass) Render the entire scene viewed from the camera.
       Then load the shadow ID map and apply texture coord projection
       from the light. Compare the ID number of the object to be
       drawn and that from projected shadow map. If two ID numbers
       are not similar, it means a ray from the light is obstructed
       by an another polygon which contains another ID. So the part
       should become shadow. The comparison is done by some TEV
       stages where ID values are considered as color data.
       
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI                  3.14159265358979323846F
#define MAX_Z               0x00ffffff // max value of Z buffer

#define NUM_SHADOW_SIZES    4
#define SHADOW_TEX_MAX      480

#if ( GX_REV == 1 )         // GX revision 1
#define SHADOW_TEX_FMT      GX_TF_RGBA8
#define SHADOW_TEX_CPFMT    GX_TF_RGBA8
#else                       // GX revision 2 or later
#define SHADOW_TEX_FMT      GX_TF_I8
#define SHADOW_TEX_CPFMT    GX_CTF_R8
#endif


#define NUM_DISP_MODES      3

// default parameters for animating object
#define BALL_VX_INI         15.0F
#define BALL_VZ_INI         25.0F


#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
  Structure definitions
 *---------------------------------------------------------------------------*/
// for camera
typedef struct
{
    Vec               location;
    Vec               up;
    Vec               target;
    f32               left;
    f32               top;
    f32               znear;
    f32               zfar;
    GXProjectionType  type;
} CameraConfig;

typedef struct
{
    CameraConfig      cfg; 
    Mtx               view;
    Mtx44             proj;
    s32               theta;
    s32               phi;
    f32               distance;
} MyCameraObj;

// for light
typedef struct
{
    GXLightObj    lobj;
    MyCameraObj   cam;  // for projection
} MyLightObj;

// for animating objects
typedef struct
{
    u32           octaRot;
    f32           ballX;
    f32           ballZ;
    f32           ballVx;
    f32           ballVz;
} MyAnimCtrlObj;

// for entire scene control
typedef struct
{
    MyCameraObj   cam;
    MyLightObj    light;
    MyAnimCtrlObj anim;
    u32           dispMode;
    u32           projMode;
    u32           adjMode;
    u8*           shadowTexData;
    u32           shadowTexSize;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void DrawModels      ( Mtx view, Mtx tgv, f32 adj, MyAnimCtrlObj* anim );
static void LoadGXMatrices  ( Mtx view, Mtx tgv, Mtx mdm );
static void GetTexProjMtx   ( MyCameraObj* cam, Mtx mp );
static void SetModelID      ( u8 id );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MyLightObj* light, Mtx v );

static void SetShaderModeFor1stPass  ( void );
static void SetShaderModeFor2ndPass  ( void );
static void SetShaderModeForProj     ( void );

static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
   Lighting parameters
 *---------------------------------------------------------------------------*/
#define COL_WHITE    MyColors[0]
#define COL_BLACK    MyColors[1]
#define COL_AMBIENT  MyColors[2]
#define COL_LIGHT    MyColors[3]
#define COL_MAT0     MyColors[4]
#define COL_SHADOW0  MyColors[5]
#define COL_MAT1     MyColors[6]
#define COL_SHADOW1  MyColors[7]
#define COL_MAT2     MyColors[8]
#define COL_SHADOW2  MyColors[9]
#define COL_MAT3     MyColors[10]
#define COL_SHADOW3  MyColors[11]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {0xff, 0xff, 0xff, 0xff},  // white
    {0x00, 0x00, 0x00, 0x00},  // background
    {0x40, 0x40, 0x40, 0xff},  // ambient
    {0xb0, 0xb0, 0xb0, 0xff},  // light color
    {0xf0, 0xf0, 0xf0, 0xff},  // material color 0
    {0x3C, 0x3C, 0x3C, 0xff},  // shadow color 0
    {0xff, 0xe0, 0x80, 0xff},  // material color 1
    {0x40, 0x38, 0x20, 0xff},  // shadow color 1
    {0x80, 0xff, 0x80, 0xff},  // material color 2
    {0x20, 0x40, 0x20, 0xff},  // shadow color 2
    {0xA0, 0xc0, 0xff, 0xff},  // material color 3
    {0x28, 0x30, 0x40, 0xff},  // shadow color 3
};

/*---------------------------------------------------------------------------*
   Other table data
 *---------------------------------------------------------------------------*/
// shadow map texture size
static u16 ShadowSizeTbl[NUM_SHADOW_SIZES] =
{
    480, 256, 128, 64
};

/*---------------------------------------------------------------------------*
   Strings for messages
 *---------------------------------------------------------------------------*/
static char* DispModeMsg[] =
{
    "result scene",
    "ID of each object",
    "projected shadow (ID) map"
};

static char* ProjModeMsg[] =
{
    "Perspective",
    "Orthographic"
};

static char* AdjModeMsg[] =
{
    "OFF",
    "ON"
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 1.0F, 1.0F, 1.0F }, // location (Not used)
    { 0.0F, 0.0F, 1.0F }, // up
    { 0.0F, 0.0F, 0.0F }, // target
    -160.0F,  // left
    120.0F,   // top   
    300.0F,   // near
    5000.0F,  // far
    GX_PERSPECTIVE // type
};

static CameraConfig DefaultLightCamera0 =
{
    { 1.0F, 1.0F, 1.0F }, // location (Not used)
    { 0.0F, 0.0F, 1.0F }, // up
    { 0.0F, 0.0F, 0.0F }, // target
    -64.0F,   // left
     64.0F,   // top   
    200.0F,   // near
    10000.0F, // far
    GX_PERSPECTIVE // type
};

static CameraConfig DefaultLightCamera1 =
{
    { 1.0F, 1.0F, 1.0F }, // location (Not used)
    { 0.0F, 0.0F, 1.0F }, // up
    { 0.0F, 0.0F, 0.0F }, // target
    -832.0F, // left
     832.0F, // top   
    200.0F,   // near
    10000.0F, // far
    GX_ORTHOGRAPHIC // type
};

/*---------------------------------------------------------------------------*
   Global Variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;    // scene control parameters

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);       // Init the OS, game pad, graphics and video.

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
    u32    size;

    // Set pixel format and background color
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GXSetCopyClear(COL_BLACK, MAX_Z);
    GXSetCullMode(GX_CULL_NONE);
    
    // Perform dummy copy to clear EFB by specified color
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);


    // Default scene parameter settings

    // camera
    sc->cam.cfg      = DefaultCamera;
    sc->cam.theta    = 45;
    sc->cam.phi      = 25;
    sc->cam.distance = 2000.0F;

    // Light camera
    sc->projMode           = 0;
    sc->light.cam.theta    = 0;
    sc->light.cam.phi      = 60;
    sc->light.cam.distance = 3000.0F;

    // display mode and shadow map control mode
    sc->dispMode      = 0;
    sc->shadowTexSize = 0;
    sc->adjMode       = 1;

    // animating objects
    sc->anim.octaRot = 0;
    sc->anim.ballX   = 0.0F;
    sc->anim.ballZ   = 0.0F;
    sc->anim.ballVx  = BALL_VX_INI;
    sc->anim.ballVz  = BALL_VZ_INI;

    // Memory area for dynamic shadow map
    size = GXGetTexBufferSize(
               SHADOW_TEX_MAX,
               SHADOW_TEX_MAX,
               SHADOW_TEX_FMT,
               GX_FALSE,
               0 );
    sc->shadowTexData = OSAlloc(size);

}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    GXRenderModeObj *rmp;
    Mtx       mtg;
    GXTexObj  to;
    u16       shSize, scrWd, scrHt;
    f32       adjf;
    
    // get current rendering mode structure
    rmp   = DEMOGetRenderModeObj();
    scrWd = rmp->fbWidth;
    scrHt = rmp->efbHeight;

    GXInvalidateTexAll();
    shSize = ShadowSizeTbl[sc->shadowTexSize];

    //-------------------------------------------
    //    1st pass.
    //    Make ID map viewed from the light
    //-------------------------------------------

    // Set viewport/camera for making shadow texture
    GXSetViewport(0, 0, shSize, shSize, 0.0F, 1.0F);
    GXSetScissor(0, 0, (u32)shSize, (u32)shSize);
    sc->light.cam.cfg = ( sc->projMode ) ?
                        DefaultLightCamera1 : DefaultLightCamera0;
    SetCamera(&sc->light.cam);
    
    // Set render mode which only draws ID number as a color
    SetShaderModeFor1stPass();

    // Scale adjustment factor which can be used to enlarge
    // drawing area of each object in the first pass
    adjf = ( sc->adjMode ) ? 1.15F : 1.0F;

    // Draw models with ID
    MTXIdentity(mtg);
    DrawModels(sc->light.cam.view, mtg, adjf, &sc->anim);
    

    //-------------------------------------------
    //    Copy ID map into Texture
    //-------------------------------------------
    
    // Turn off vertical de-flicker filter temporary
    // (to avoid filtering while framebuffer-to-texture copy)
    GXSetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);

    // Copy shadow image into texture
    GXSetTexCopySrc(0, 0, shSize, shSize);
    GXSetTexCopyDst(shSize, shSize, SHADOW_TEX_CPFMT, GX_FALSE);
    GXCopyTex(sc->shadowTexData, GX_TRUE);

    // Wait for finishing the copy task in the graphics pipeline
    GXPixModeSync();

    // Restore vertical de-flicker filter mode
    GXSetCopyFilter(rmp->aa, rmp->sample_pattern, GX_TRUE, rmp->vfilter);


    //-------------------------------------------
    //    2nd pass.
    //    Make main image from the viewer
    //-------------------------------------------

    // Set viewport/camera/light for the main image
    GXSetViewport(0, 0, scrWd, scrHt, 0.0F, 1.0F);
    GXSetScissor(0, 0, (u32)scrWd, (u32)scrHt);
    SetCamera(&sc->cam);
    SetLight(&sc->light, sc->cam.view);

    // Set shading target according to the current display mode
    switch(sc->dispMode)
    {
      case 0 :
        // Shows result scene with full-scene shadow
        SetShaderModeFor2ndPass();
        break;
      case 1 :
         // Shows ID number of each object
        SetShaderModeFor1stPass();
        break;
      case 2 :
      default:
        // Shows projected object ID map (shadow map)
        SetShaderModeForProj();
        break;
    }

    // Load object ID map (shadow map)
    GXInitTexObj(
        &to,
        sc->shadowTexData,
        shSize,
        shSize,
        SHADOW_TEX_FMT,
        GX_CLAMP,
        GX_CLAMP,
        GX_FALSE );
    GXInitTexObjLOD(&to, GX_NEAR, GX_NEAR, 0, 0, 0, 0, 0, GX_ANISO_1);
    GXLoadTexObj(&to, GX_TEXMAP0);

    // Set up shadow projection
    GetTexProjMtx(&sc->light.cam, mtg);

    // Draw models with IDs
    DrawModels(sc->cam.view, mtg, 1.0F, &sc->anim);
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

    // Camera position
    sc->cam.theta += ( DEMOPadGetStickX(0) / 24 );
    sc->cam.theta = ( sc->cam.theta + 360 ) % 360;
    sc->cam.phi   += ( DEMOPadGetStickY(0) / 24 );
    Clamp(sc->cam.phi, 10, 60);

    // Light position
    sc->light.cam.theta += ( DEMOPadGetSubStickX(0) / 24 );
    sc->light.cam.theta = ( sc->light.cam.theta + 360 ) % 360;
    sc->light.cam.phi += ( DEMOPadGetSubStickY(0) / 24 );
    Clamp(sc->light.cam.phi, 15, 90);
    
    // Change display mode
    if ( down & PAD_BUTTON_A )
    {
        sc->dispMode = (++sc->dispMode) % NUM_DISP_MODES;
        OSReport("Mode = %s\n", DispModeMsg[sc->dispMode]);
    }

    // Change shadow map size
    if ( down & PAD_BUTTON_Y )
    {
        sc->shadowTexSize = (++sc->shadowTexSize) % NUM_SHADOW_SIZES;
        OSReport(
            "Shadow Size = %dx%d\n",
            ShadowSizeTbl[sc->shadowTexSize],
            ShadowSizeTbl[sc->shadowTexSize] );
    }
    
    // Toggle shadow projection type
    if ( down & PAD_BUTTON_B )
    {
        sc->projMode = ( sc->projMode == 0 ) ? 1U : 0U;
        OSReport("Projection = %s\n", ProjModeMsg[sc->projMode]);
    }

    // Object scale adjustment
    if ( down & PAD_BUTTON_X )
    {
        sc->adjMode = ( sc->adjMode == 0 ) ? 1U : 0U;
        OSReport("Scale adjustment = %s\n", AdjModeMsg[sc->adjMode]);
    }


    // Animating objects
    sc->anim.ballX += sc->anim.ballVx;
    if ( sc->anim.ballX > 450.0F )
    {
        sc->anim.ballVx = - BALL_VX_INI;
    }
    if ( sc->anim.ballX < -450.0F )
    {
        sc->anim.ballVx = BALL_VX_INI;
    }
    
    sc->anim.ballZ  += sc->anim.ballVz;
    sc->anim.ballVz -= 1.5F;    // gravity
    if ( sc->anim.ballZ < 0.0F )
    {
        sc->anim.ballVz = BALL_VZ_INI;
    }
    
    sc->anim.octaRot = ( sc->anim.octaRot + 5 ) % 360;
   
}

/*---------------------------------------------------------------------------*
    Name:           DrawModels
    
    Description:    Draw all models with object ID and shadow
                    projection texgen matrix
                    
    Arguments:      view : view matrix
                    tgv  : view & projection matrix for texgen
                    adj  : scale adjustment factor
                    anim : parameters for animating objects
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModels( Mtx view, Mtx tgv, f32 adj, MyAnimCtrlObj* anim )
{
    u8     id = 0; // object ID for generating ID map
    s32    i, j, x, y;
    Mtx    mt, ms, mr, mm;

    // 8X values are used as ID numbers in order to visualize
    // differences of IDs between an object and another.
    // In general, you can use continuous integer numbers.

    // Cylinders
    GXSetChanMatColor(GX_COLOR1A1, COL_MAT0);
    GXSetTevColor(GX_TEVREG0, COL_SHADOW0); // TEVREG0 = shadow color

    MTXScale(ms, (f32)(40.0F*adj), (f32)(40.0F*adj), (f32)(300.0F*adj));
    for ( i = -2 ; i < 3 ; ++i )
    {
        for ( j = -1 ; j < 2 ; j += 2 )
        {
            x = i - j;
            y = i + j;
            MTXTrans(mt, (f32)(x*150), (f32)(y*150), 200.0F);
            MTXConcat(mt, ms, mm);
            LoadGXMatrices(view, tgv, mm);
        
            id += 8;
            SetModelID(id);
            GXDrawCylinder(16);
        }
    }
    
    // Roofs
    GXSetChanMatColor(GX_COLOR1A1, COL_MAT0);
    GXSetTevColor(GX_TEVREG0, COL_SHADOW0); // TEVREG0 = shadow color
    
    MTXRotDeg(mr, 'z', 45);
    MTXScale(ms, (f32)(80.0F*adj), (f32)(480.0F*adj), (f32)(60.0F*adj));
    MTXConcat(mr, ms, ms);
    for ( i = -2 ; i < 3 ; ++i )
    {
        MTXTrans(mt, (f32)(i*150), (f32)(i*150), 530.0F);
        MTXConcat(mt, ms, mm);
        LoadGXMatrices(view, tgv, mm);

        id += 8;
        SetModelID(id);
        GXDrawCube();
    }
    
    // Bounding ball
    GXSetChanMatColor(GX_COLOR1A1, COL_MAT1);
    GXSetTevColor(GX_TEVREG0, COL_SHADOW1); // TEVREG0 = shadow color
    
    MTXScale(ms, (f32)(100.0F*adj), (f32)(100.0F*adj), (f32)(100.0F*adj));
    MTXTrans(mt, anim->ballX, anim->ballX, anim->ballZ);
    MTXConcat(mt, ms, mm);
    LoadGXMatrices(view, tgv, mm);
    
    id += 8;
    SetModelID(id);
    GXDrawSphere1(1);
    

    // Rotating octahedra
    GXSetChanMatColor(GX_COLOR1A1, COL_MAT2);
    GXSetTevColor(GX_TEVREG0, COL_SHADOW2); // TEVREG0 = shadow color
    
    MTXScale(ms, (f32)(150.0F*adj), (f32)(150.0F*adj), (f32)(150.0F*adj));
    MTXRotDeg(mr, 'z', anim->octaRot);
    MTXConcat(mr, ms, ms);
    for ( i = -1 ; i <= 1 ; i += 2 )
    {
        MTXTrans(mt, (f32)(-i*400.0F), (f32)(i*400.0F), 50.0F);
        MTXConcat(mt, ms, mm);
        LoadGXMatrices(view, tgv, mm);
        
        id += 8;
        SetModelID(id);
        GXDrawOctahedron();
    }

    
    // Floor
    GXSetChanMatColor(GX_COLOR1A1, COL_MAT3);
    GXSetTevColor(GX_TEVREG0, COL_SHADOW3); // TEVREG0 = shadow color
    
    MTXScale(ms, (f32)(1000.0F*adj), (f32)(1000.0F*adj), (f32)(50.0F*adj));
    MTXTrans(mt, 0.0F, 0.0F, -125.0F);
    MTXConcat(mt, ms, mm);
    LoadGXMatrices(view, tgv, mm);
    
    id += 8;
    SetModelID(id);
    GXDrawCube();
}

/*---------------------------------------------------------------------------*
    Name:           LoadGXMatrices
    
    Description:    Calculates each Pos/Nrm/Tex matrices and
                    loads them into the hardware.

    Arguments:      view : view matrix
                    tgv  : view & projection matrix for texgen
                    mdm  : model transform matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void LoadGXMatrices( Mtx view, Mtx tgv, Mtx mdm )
{
    Mtx mv, mvi;
    
    MTXConcat(view, mdm, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi);
    MTXTranspose(mvi, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    MTXConcat(tgv, mdm, mv);
    GXLoadTexMtxImm(mv, GX_TEXMTX0, GX_MTX3x4);
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

    if ( cam->cfg.type == GX_PERSPECTIVE )
    {
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
    }
    else // ( cam->cfg.type == GX_ORTHOGRAPHIC )
    {
        MTXLightOrtho(
            proj,
            - (cam->cfg.top), // t = -y in projected texture 
            cam->cfg.top,
            cam->cfg.left,
            - (cam->cfg.left),
            0.5F,
            0.5F,
            0.5F,
            0.5F );
    }
    
    MTXConcat(proj, cam->view, mp);
}

/*---------------------------------------------------------------------------*
    Name:           SetModelID
    
    Description:    Sets ID number for next drawing object
                    ID number is used for generating shadow map
                    This program uses constant color output from color
                    channel 0 for ID encoding. You can also use direct
                    vertex color or TEV register value instead.
                    
    Arguments:      id : ID number for the object (> 0)
        
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetModelID( u8 id )
{
    GXColor  col;
    col.r = col.g = col.b = id;
    
    GXSetChanMatColor(GX_COLOR0A0, col);
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

    if ( cam->cfg.type == GX_PERSPECTIVE )
    {
        MTXFrustum(
            cam->proj,
            cam->cfg.top,
            - (cam->cfg.top),
            cam->cfg.left,
            - (cam->cfg.left),
            cam->cfg.znear,
            cam->cfg.zfar );
    }
    else // ( cam->cfg.type == GX_ORTHOGRAPHIC )
    {
        MTXOrtho(
            cam->proj,
            cam->cfg.top,
            - (cam->cfg.top),
            cam->cfg.left,
            - (cam->cfg.left),
            cam->cfg.znear,
            cam->cfg.zfar );
    }
    
    GXSetProjection(cam->proj, cam->cfg.type);
}

/*---------------------------------------------------------------------------*
    Name:           SetLight
    
    Description:    Set up light parameters
                    
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
}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeFor1stPass
    
    Description:    Set up shading mode (color channel, TEV, etc.)
                    for 1st pass which generates object ID map for shadow.
                    This pass only uses constant color set as material.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeFor1stPass( void )
{
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,    // enable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE );
    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0A0, COL_BLACK);
    
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
#if ( GX_REV == 1 ) // Only available on GX revision 1
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_LINEAR);
#endif
    
    GXSetNumTexGens(0);
}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeFor2ndPass
    
    Description:    Set up shading mode (coloe channel, TEV, etc.)
                    for 2nd pass which draws actual scene from the
                    viewer with full-scene shadow.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeFor2ndPass( void )
{
    GXSetNumChans(2);

    // Still channel 0 is used to send ID
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,    // enable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE );
    GXSetChanAmbColor(GX_COLOR0A0, COL_BLACK);

    // channel 1 is used to lighting
    GXSetChanCtrl(
        GX_COLOR1A1,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE );
    GXSetChanAmbColor(GX_COLOR1A1, COL_AMBIENT);

    // texgen for shadow projection
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);

#ifdef EMU
    //--------------------------------------------------------------
    //  Code for the emulator (cannot draw shadows)
    //--------------------------------------------------------------
    GXSetNumTevStages(1);

    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    
#else // !EMU
#if ( GX_REV == 1 )
    //--------------------------------------------------------------
    //  Code for real hardware with GX revision 1 (HW1_DRIP, HW1)
    //--------------------------------------------------------------
    GXSetNumTevStages(2);

    // TEV Stage 0  ( Color0 - Texture0 with EQ clamp mode )
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC);
    GXSetTevColorOp(
        GX_TEVSTAGE0,
        GX_TEV_SUB,
        GX_TB_ZERO,
        GX_CS_DIVIDE_2, // Clamp mode works correctly only when scale = 1/2
        GX_TRUE,
        GX_TEVPREV );
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_EQ);
    
    // TEV Stage 1  ( REGPREV == 0 ? Color1 : Reg0 )
    // Register 0 is supporsed to hold shadow color
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_RASC, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO);
    GXSetTevColorOp(
        GX_TEVSTAGE1,
        GX_TEV_ADD,
        GX_TB_ZERO,
        GX_CS_SCALE_1,
        GX_TRUE,
        GX_TEVPREV );
    GXSetTevClampMode(GX_TEVSTAGE1, GX_TC_LINEAR);

#else // ( GX_REV >= 2 )
    //--------------------------------------------------------------
    //  Code for real hardware with GX revision 2 or later
    //--------------------------------------------------------------
    GXSetNumTevStages(2);

    // TEV Stage 0  ( Color0(R) == Texture0(R) ? 255 : 0 )
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_RASC, GX_CC_ONE, GX_CC_ZERO);
    GXSetTevColorOp(
        GX_TEVSTAGE0,
        GX_TEV_COMP_R8_EQ,  // R8 equal compare mode
        GX_TB_ZERO,         // actually N/A
        GX_CS_SCALE_1,      // actually N/A
        GX_TRUE,
        GX_TEVPREV );

    // TEV Stage 1  ( REGPREV == 0 ? Reg0 : Color1 )
    // Register 0 is supporsed to hold shadow color
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_C0, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO);
    GXSetTevColorOp(
        GX_TEVSTAGE1,
        GX_TEV_ADD,
        GX_TB_ZERO,
        GX_CS_SCALE_1,
        GX_TRUE,
        GX_TEVPREV );

#endif // GX_REV

#endif // EMU

}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeForProj
    
    Description:    Set up shading mode (color channel, TEV, etc.)
                    for 2nd pass.
                    This mode displays only projected shadow map.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeForProj( void )
{
    GXSetNumChans(0);

    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
#if ( GX_REV == 1 ) // Only available on GX revision 1
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_LINEAR);
#endif
    
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
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
    OSReport("tg-shadow2: full-scene shadow by object ID map\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main stick : Move the camera\n");
    OSReport("Main stick : Move the light\n");
    OSReport("A button   : Change display mode\n");
    OSReport("B button   : Change projection type\n");
    OSReport("X button   : Scale adjustment ON/OFF\n");
    OSReport("Y button   : Change shadow map size\n");
    OSReport("******************************************************\n");
    OSReport("\n\n");
}

/*============================================================================*/
