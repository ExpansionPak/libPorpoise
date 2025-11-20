/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-shadow3.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-shadow3.c $
    
    10    5/12/01 2:25a Hirose
    removed obsolete flags
    
    9     11/03/00 3:50p Hirose
    updated HW2 codes to use Z8/Z16 texture copy formats
    
    8     11/02/00 11:39p Hirose
    added GX revision 2 specific codes
    
    7     5/19/00 1:31a Hirose
    deleted #ifdef CSIM_OUTPUT stuff
    
    6     5/02/00 4:04p Hirose
    updated to call DEMOGetCurrentBuffer
    some small changes
    
    5     4/29/00 12:31a Hirose
    implemented second depth shadow map method
    
    4     4/21/00 7:01p Hirose
    updated comments / disabled color update during drawing 1st. pass image
    
    3     4/18/00 9:52p Hirose
    fixed a bug on 16bit comare mode
    
    2     4/15/00 1:06a Hirose
    added 16bit compare mode
    
    1     4/13/00 8:35p Hirose
    initial version (16bit comparison is not implemented yet.)

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   tg-shadow3
       Full-scene shadow method by texture copy from Z buffer
 *---------------------------------------------------------------------------*

   Overview of this method:

       (1st. pass) Render the entire scene from the light's point
       of view. Since only geometry information is necessary,
       color update is turned off. Only Z buffer will be updated.
       And front-face culling helps creating shadow map of "second"
       visible surfaces which can avoid the "first" lit surfaces
       to become shadow. 
       
       Copy the result Z buffer into a texture (shadow map).
       Z buffer is cleared at the same time.
       
       (2nd. pass) Render the entire scene viewed from the camera.
       Then load the shadow map and apply texture coord projection
       from the light. And also get the distance from the light
       which can be obtained by looking up a ramp texture with
       appropriate texture coordinate generation matrix. Compare
       these two values (shadow map and distance) and detect whether
       a point should become shadow. If Z data from shadow map is
       less than actual distance, it means a ray from the light is
       obstructed by an another polygon and the point becomes shadow.
       The comparison is done by some TEV stages where Z values are
       considered as color data.
       
       This demo shows both 8bit/16bit Z precision comparison scheme.

 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI               3.14159265F
#define MAX_Z            0x00ffffff // max value of Z buffer

#define NUM_SHADOW_SIZES 4
#define NUM_DISP_MODES   6

// For drawing objects
#define NUM_GRIDS        8
#define SP_NUMR          16

// For shadowing
#define SHADOW_TEX_MAX   400

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

// for packing various transform matrices
typedef struct
{
    Mtx    view;     // Viewing transformation
    Mtx    texProj;  // Texgen for projected shadow map
    Mtx    depth;    // Texgen for depth value lookup
} MyTransMtxObj;

// for entire scene control
typedef struct
{
    MyCameraObj   cam;
    MyLightObj    light;
    u32           dispMode;
    u32           projMode;
    s32           modelRot;
    u8*           shadowTexData;
    u32           shadowTexSize;
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
static void DrawModels      ( MyTransMtxObj* tmo, s32 rot );
static void DrawFloor       ( void );
static void DrawSpiral      ( u32 roll );
static void LoadGXMatrices  ( MyTransMtxObj* tmo, Mtx mm );
static void GetTexProjMtx   ( MyTransMtxObj* tmo, MyCameraObj* cam, u8 bitMode );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MyLightObj* light, Mtx v );

static void SetShaderModeFor1stPass     ( void );
static void SetShaderModeFor2ndPass8    ( void );
static void SetShaderModeFor2ndPass16   ( void );
static void SetShaderModeForProj8       ( void );
static void SetShaderModeForProj16      ( void );
static void SetShaderModeForDepth8      ( void );
static void SetShaderModeForDepth16     ( void );

static void CreateRampTex8  ( GXTexObj* to );
static void CreateRampTex16 ( GXTexObj* to );
static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
   Lighting parameters
 *---------------------------------------------------------------------------*/
#define COL_BLACK    MyColors[0]
#define COL_GREEN    MyColors[1]
#define COL_RED      MyColors[2]
#define COL_YELLOW   MyColors[3]
#define COL_AMBIENT  MyColors[4]
#define COL_LIGHT    MyColors[5]
#define COL_MAT0     MyColors[6]
#define COL_SHADOW0  MyColors[7]
#define COL_MAT1     MyColors[8]
#define COL_SHADOW1  MyColors[9]
#define COL_MAT2     MyColors[10]
#define COL_SHADOW2  MyColors[11]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {0x00, 0x00, 0x00, 0xff},  // black
    {0x00, 0xff, 0x00, 0xff},  // green
    {0xff, 0x00, 0x00, 0xff},  // red
    {0xff, 0xff, 0x00, 0xff},  // yellow
    {0x40, 0x40, 0x40, 0xff},  // ambient
    {0xb0, 0xb0, 0xb0, 0xff},  // light color
    {0xf0, 0xf0, 0xf0, 0xff},  // material color 0
    {0x3C, 0x3C, 0x3C, 0xff},  // shadow color 0
    {0x80, 0xff, 0x80, 0xff},  // material color 1
    {0x20, 0x40, 0x20, 0xff},  // shadow color 1
    {0xA0, 0xc0, 0xff, 0xff},  // material color 2
    {0x28, 0x30, 0x40, 0xff},  // shadow color 2
};

// fixed normal vector
static f32 FixedNormal[] ATTRIBUTE_ALIGN(32) =
{
    1.0F,  0.0F,  0.0F,  // +X
   -1.0F,  0.0F,  0.0F,  // -X
    0.0F,  1.0F,  0.0F,  // +Y
    0.0F, -1.0F,  0.0F,  // -Y
    0.0F,  0.0F,  1.0F,  // +Z
    0.0F,  0.0F, -1.0F   // -Z
};

/*---------------------------------------------------------------------------*
   Floor elevation map data
 *---------------------------------------------------------------------------*/
static s8 FloorElvTbl[NUM_GRIDS][NUM_GRIDS] =
{
    { 6, 4, 2, 4, 3, 5, 1, 1 },
    { 4, 1, 1, 3, 2, 2, 1, 1 },
    { 1, 1, 5, 1, 1, 5, 3, 5 },
    { 1, 1, 1, 1, 1, 4, 2, 3 },
    { 5, 4, 1, 1, 1, 1, 1, 1 },
    { 7, 5, 2, 1, 1, 5, 1, 1 },
    { 1, 1, 5, 7, 3, 1, 1, 3 },
    { 1, 1, 3, 5, 1, 1, 6, 2 }
};

/*---------------------------------------------------------------------------*
   Shadow texture size table
 *---------------------------------------------------------------------------*/
static u16 ShadowSizeTbl[NUM_SHADOW_SIZES] =
{
    400, 256, 128, 64
};

/*---------------------------------------------------------------------------*
   Strings for messages
 *---------------------------------------------------------------------------*/
static char* DispModeMsg[NUM_DISP_MODES] =
{
    "result scene (8bit depth)",
    "actual depth from the light (8bit depth)",
    "projected shadow depth map (8bit depth)",
    "result scene (16bit depth)",
    "actual depth from the light (16bit depth)",
    "projected shadow depth map (16bit depth)"
};

static char* ProjModeMsg[] =
{
    "Perspective",
    "Orthographic"
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

// Perspective projection for local light effect
static CameraConfig DefaultLightCamera0 =
{
    { 1.0F, 1.0F, 1.0F }, // location (Not used)
    { 0.0F, 0.0F, 1.0F }, // up
    { 0.0F, 0.0F, 0.0F }, // target
    -640.0F,  // left
     640.0F,  // top   
    2000.0F,  // near
    4000.0F,  // far
    GX_PERSPECTIVE // type
};

// Orthographic projection for infinite light effect
static CameraConfig DefaultLightCamera1 =
{
    { 1.0F, 1.0F, 1.0F }, // location (Not used)
    { 0.0F, 0.0F, 1.0F }, // up
    { 0.0F, 0.0F, 0.0F }, // target
    -960.0F, // left
     960.0F, // top   
    2000.0F, // near
    4000.0F, // far
    GX_ORTHOGRAPHIC // type
};

/*---------------------------------------------------------------------------*
   Global Variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl; // scene control parameters
static GXTexObj        RampTex8;  // 8 bit ramp texture
static GXTexObj        RampTex16; // 16 bit ramp texture

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
    GXRenderModeObj *rmp;
    u32    size;

    // get framebuffer size of current rendering mode
    rmp = DEMOGetRenderModeObj();
    sc->screen_wd = rmp->fbWidth;
    sc->screen_ht = rmp->efbHeight;

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);

    // Set pixel format and background color
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GXSetCopyClear(COL_BLACK, MAX_Z);
    
    // Perform dummy copy to clear EFB by specified color
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);

    // Create ramp textures
    CreateRampTex8(&RampTex8);
    CreateRampTex16(&RampTex16);


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

    // animating objects
    sc->modelRot = 0;

    // Memory area for dynamic shadow map
    size = GXGetTexBufferSize(
               SHADOW_TEX_MAX,
               SHADOW_TEX_MAX,
               GX_TF_RGBA8,
               GX_FALSE,
               0 );
    sc->shadowTexData = OSAlloc(size);

}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the scene by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    MyTransMtxObj   tmo;
    GXTexObj        to;
    GXTexFmt        shFmt, shCpFmt;
    u16             shSize;
    
    // Invalidate texture cache
    GXInvalidateTexAll();

    // Shadow map texture setting
    shSize = ShadowSizeTbl[sc->shadowTexSize];

#if ( GX_REV == 1 ) // GX revision 1 (HW1, etc.)
    // Only Z24X8 copy format is available
    shCpFmt = GX_TF_Z24X8;
    shFmt   = GX_TF_RGBA8;

#else // ( GX_REV >= 2 ) : GX revision 2 or later (HW2, ...)
    if ( sc->dispMode <= 2 )
    {
        // 8bit mode
        shCpFmt = GX_TF_Z8;
        shFmt   = GX_TF_I8;
    }
    else
    {
        // 16bit mode
        shCpFmt = GX_TF_Z16;
        shFmt   = GX_TF_IA8;
    }
#endif  // GX_REV

    //-------------------------------------------
    //  1st. pass
    //  Make an image viewed from the light
    //-------------------------------------------

    // Color update is disabled. Only Z will be updated.
    GXSetColorUpdate(GX_DISABLE);

    // To draw "second" surfaces from the light
    GXSetCullMode(GX_CULL_FRONT);

    // Set viewport for making shadow texture
    GXSetViewport(0, 0, shSize, shSize, 0.0F, 1.0F);
    GXSetScissor(0, 0, (u32)shSize, (u32)shSize);

    // Set camera configuration
    sc->light.cam.cfg = ( sc->projMode ) ?
                        DefaultLightCamera1 : DefaultLightCamera0;
    SetCamera(&sc->light.cam);
    
    // Set render mode to use only constant color
    // because we need only depth buffer
    SetShaderModeFor1stPass();

    // Draw models
    MTXCopy(sc->light.cam.view, tmo.view);
    DrawModels(&tmo, sc->modelRot);
    

    //-------------------------------------------
    //  Copy shadow depth map into Texture
    //-------------------------------------------
    
    // Copy shadow image into texture
    GXSetTexCopySrc(0, 0, shSize, shSize);
    GXSetTexCopyDst(shSize, shSize, shCpFmt, GX_FALSE);
    GXCopyTex(sc->shadowTexData, GX_TRUE);

    // Wait for finishing the copy task in the graphics pipeline
    GXPixModeSync();

    //-------------------------------------------
    //  2nd. pass
    //  Make an image viewed from the camera
    //-------------------------------------------

    // Enable color update
    GXSetColorUpdate(GX_ENABLE);

    // Restore culling mode to normal
    GXSetCullMode(GX_CULL_BACK);

    // Set viewport/camera/light for the main image
    GXSetViewport(0, 0, sc->screen_wd, sc->screen_ht, 0.0F, 1.0F);
    GXSetScissor(0, 0, (u32)sc->screen_wd, (u32)sc->screen_ht);
    SetCamera(&sc->cam);
    SetLight(&sc->light, sc->cam.view);

    // Set up shadow map texture
    GXInitTexObj(
        &to,
        sc->shadowTexData,
        shSize,
        shSize,
        shFmt,
        GX_CLAMP,
        GX_CLAMP,
        GX_FALSE );
    GXInitTexObjLOD(&to, GX_NEAR, GX_NEAR, 0, 0, 0, 0, 0, GX_ANISO_1);


    // Set shading mode according to the current display mode
    switch(sc->dispMode)
    {
      case 0 :
        // Shows acturl result scene (8bit comparison)
        SetShaderModeFor2ndPass8();
        GetTexProjMtx(&tmo, &sc->light.cam, 0);
        GXLoadTexObj(&RampTex8, GX_TEXMAP0);
        GXLoadTexObj(&to, GX_TEXMAP1);
        break;
      case 1 :
        // Shows depth from the light (8bit)
        SetShaderModeForDepth8();
        GetTexProjMtx(&tmo, &sc->light.cam, 0);
        GXLoadTexObj(&RampTex8, GX_TEXMAP0);
        break;
      case 2 :
        // Shows projected shadow depth map (8bit)
        SetShaderModeForProj8();
        GetTexProjMtx(&tmo, &sc->light.cam, 0);
        GXLoadTexObj(&to, GX_TEXMAP0);
        break;
      case 3 :
        // Shows acturl result scene (16bit comparison)
        SetShaderModeFor2ndPass16();
        GetTexProjMtx(&tmo, &sc->light.cam, 1);
        GXLoadTexObj(&RampTex16, GX_TEXMAP0);
        GXLoadTexObj(&to, GX_TEXMAP1);
        break;
      case 4 :
        // Shows depth from the light (16bit)
        SetShaderModeForDepth16();
        GetTexProjMtx(&tmo, &sc->light.cam, 1);
        GXLoadTexObj(&RampTex16, GX_TEXMAP0);
        break;
      case 5 :
      default:
        // Shows projected shadow depth map (16bit)
        SetShaderModeForProj16();
        GetTexProjMtx(&tmo, &sc->light.cam, 1);
        GXLoadTexObj(&to, GX_TEXMAP0);
        break;
    }

    // Draw models
    MTXCopy(sc->cam.view, tmo.view);
    DrawModels(&tmo, sc->modelRot);
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

    sc->modelRot = ( sc->modelRot + 5 ) % 360;
   
}

/*---------------------------------------------------------------------------*
    Name:           DrawModels
    
    Description:    Draw all models included in the scene
                    
    Arguments:      tmo : a transform matrix set which includes view
                          matrix and texgen matrix
                    rot : rotation parameter for animating object
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModels( MyTransMtxObj* tmo, s32 rot )
{
    s32    i, j, x, y;
    Mtx    mt, ms, mr, mm;

    // Columns
    GXSetChanMatColor(GX_COLOR0A0, COL_MAT0);
    GXSetTevColor(GX_TEVREG0, COL_SHADOW0); // TEVREG0 = shadow color

    MTXScale(ms, 40.0F, 40.0F, 300.0F);
    for ( i = -2 ; i < 3 ; ++i )
    {
        for ( j = -1 ; j < 2 ; j += 2 )
        {
            x = i - j;
            y = i + j;
            MTXTrans(mt, (f32)(x*150), (f32)(y*150), 200.0F);
            MTXConcat(mt, ms, mm);
            LoadGXMatrices(tmo, mm);
        
            GXDrawCylinder(16);
        }
    }

    // Roofs
    GXSetChanMatColor(GX_COLOR0A0, COL_MAT0);
    GXSetTevColor(GX_TEVREG0, COL_SHADOW0); // TEVREG0 = shadow color
    
    MTXRotDeg(mr, 'z', 45);
    MTXScale(ms, 80.0F, 480.0F, 60.0F);
    MTXConcat(mr, ms, ms);
    for ( i = -2 ; i < 3 ; ++i )
    {
        MTXTrans(mt, (f32)(i*150), (f32)(i*150), 530.0F);
        MTXConcat(mt, ms, mm);
        LoadGXMatrices(tmo, mm);

        GXDrawCube();
    }
    
    // Rotating spiral objects
    GXSetChanMatColor(GX_COLOR0A0, COL_MAT1);
    GXSetTevColor(GX_TEVREG0, COL_SHADOW1); // TEVREG0 = shadow color
    
    MTXScale(ms, 125.0F, 125.0F, 20.0F);
    MTXRotDeg(mr, 'z', rot);
    MTXConcat(mr, ms, ms);
    for ( i = -1 ; i <= 1 ; i += 2 )
    {
        MTXTrans(mt, (f32)(-i*450.0F), (f32)(i*450.0F), -100.0F);
        MTXConcat(mt, ms, mm);
        LoadGXMatrices(tmo, mm);
        
        DrawSpiral(5);
    }
    
    // Floor
    GXSetChanMatColor(GX_COLOR0A0, COL_MAT2);
    GXSetTevColor(GX_TEVREG0, COL_SHADOW2); // TEVREG0 = shadow color

    MTXTrans(mt, - (f32)NUM_GRIDS / 2, -(f32)NUM_GRIDS / 2, -5);
    MTXScale(ms, 150.0F, 150.0F, 25.0F);
    MTXConcat(ms, mt, mm);
    LoadGXMatrices(tmo, mm);
        
    DrawFloor();
}

/*---------------------------------------------------------------------------*
    Name:           DrawFloor
    
    Description:    Draws the floor which contains some steps
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawFloor( void )
{
    s8  x, y, z0, z1;
    u8  n;

    // set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);

    // normal array
    GXSetArray(GX_VA_NRM, FixedNormal, 3 * sizeof(f32));

    // Z side
    for ( y = 0 ; y < NUM_GRIDS ; ++y )
    {
        GXBegin(GX_QUADS, GX_VTXFMT0, NUM_GRIDS * 4);
            for ( x = 0 ; x < NUM_GRIDS ; ++x )
            {
                z0 = FloorElvTbl[y][x];
            
                GXPosition3s8(x, y, z0);
                GXNormal1x8(4);
                GXPosition3s8(x, (s8)(y+1), z0);
                GXNormal1x8(4);
                GXPosition3s8((s8)(x+1), (s8)(y+1), z0);
                GXNormal1x8(4);
                GXPosition3s8((s8)(x+1), y, z0);
                GXNormal1x8(4);
            }
        GXEnd();
    }
                
    // X side
    for ( y = 0 ; y < NUM_GRIDS ; ++y )
    {
        // This may generate small blank between two tiles
        GXBegin(GX_QUADS, GX_VTXFMT0, (NUM_GRIDS + 1) * 4);
            for ( x = 0 ; x <= NUM_GRIDS ; ++x )
            {
                z0 = (s8)(( x == 0 ) ? 0 : FloorElvTbl[y][x-1]);
                z1 = (s8)(( x == NUM_GRIDS ) ? 0 : FloorElvTbl[y][x]);
                n  = (u8)(( z0 > z1 ) ? 0 : 1);
            
                GXPosition3s8(x, y, z0);
                GXNormal1x8(n);
                GXPosition3s8(x, (s8)(y+1), z0);
                GXNormal1x8(n);
                GXPosition3s8(x, (s8)(y+1), z1);
                GXNormal1x8(n);
                GXPosition3s8(x, y, z1);
                GXNormal1x8(n);
            }
        GXEnd();
    }

    // Y side
    for ( x = 0 ; x < NUM_GRIDS ; ++x )
    {
        // This may generate small blank between two tiles
        GXBegin(GX_QUADS, GX_VTXFMT0, (NUM_GRIDS + 1) * 4);
            for ( y = 0 ; y <= NUM_GRIDS ; ++y )
            {
                z0 = (s8)(( y == 0 ) ? 0 : FloorElvTbl[y-1][x]);
                z1 = (s8)(( y == NUM_GRIDS ) ? 0 : FloorElvTbl[y][x]);
                n  = (u8)(( z0 > z1 ) ? 2 : 3);
            
                GXPosition3s8(x, y, z0);
                GXNormal1x8(n);
                GXPosition3s8(x, y, z1);
                GXNormal1x8(n);
                GXPosition3s8((s8)(x+1), y, z1);
                GXNormal1x8(n);
                GXPosition3s8((s8)(x+1), y, z0);
                GXNormal1x8(n);
            }
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawSpiral
    
    Description:    Draws a spiral object
                    
    Arguments:      roll : number of rolls
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawSpiral( u32 roll )
{
    s32  i;
    u16  nv;
    f32  dx[SP_NUMR], dy[SP_NUMR];
    f32  px, py, pz, dr;

    // set up sin/cos table
    for ( i = 0 ; i < SP_NUMR ; ++i )
    {
        dr = 2.0F * PI * i / SP_NUMR;
        dx[i] = cosf(dr);
        dy[i] = sinf(dr);
    }
    nv = (u16)((roll*SP_NUMR+1) * 2);

    // set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);

    // Upper side
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, nv);
        for ( i = 0 ; i <= SP_NUMR*roll ; ++i )
        {
            px = dx[i % SP_NUMR];
            py = dy[i % SP_NUMR];
            pz = 1.0F + 2.0F * PI * i / SP_NUMR;
        
            GXPosition3f32(px, py, pz);
            GXNormal3f32(-px, -py, 1.0F);
            GXPosition3f32(px*0.5F, py*0.5F, pz);
            GXNormal3f32(-px, -py, 0.5F);
        }
    GXEnd();

    // Lower side
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, nv);
        for ( i = 0 ; i <= SP_NUMR*roll ; ++i )
        {
            px = dx[i % SP_NUMR];
            py = dy[i % SP_NUMR];
            pz = 2.0F * PI * i / SP_NUMR;
        
            GXPosition3f32(px*0.5F, py*0.5F, pz);
            GXNormal3f32(-px, -py, -0.5F);
            GXPosition3f32(px, py, pz);
            GXNormal3f32(-px, -py, -1.0F);
        }
    GXEnd();

    // Outside
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, nv);
        for ( i = 0 ; i <= SP_NUMR*roll ; ++i )
        {
            px = dx[i % SP_NUMR];
            py = dy[i % SP_NUMR];
            pz = 2.0F * PI * i / SP_NUMR;
        
            GXPosition3f32(px, py, pz);
            GXNormal3f32(px, py, 0.0F);
            GXPosition3f32(px, py, pz+1.0F);
            GXNormal3f32(px, py, 0.0F);
        }
    GXEnd();

    // Inside
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, nv);
        for ( i = 0 ; i <= SP_NUMR*roll ; ++i )
        {
            px = dx[i % SP_NUMR];
            py = dy[i % SP_NUMR];
            pz = 2.0F * PI * i / SP_NUMR;
        
            GXPosition3f32(px*0.5F, py*0.5F, pz+1.0F);
            GXNormal3f32(-px, -py, 0.0F);
            GXPosition3f32(px*0.5F, py*0.5F, pz);
            GXNormal3f32(-px, -py, 0.0F);
        }
    GXEnd();

    // Both ends
    GXBegin(GX_QUADS, GX_VTXFMT1, 8);

        px = dx[0];
        py = dy[0];
        pz = 2.0F * PI * roll;
        
        GXPosition3f32(px*0.5F, py*0.5F, 0.0F);
        GXNormal3f32(py, -px, 0.0F);
        GXPosition3f32(px*0.5F, py*0.5F, 1.0F);
        GXNormal3f32(py, -px, 0.0F);
        GXPosition3f32(px, py, 1.0F);
        GXNormal3f32(py, -px, 0.0F);
        GXPosition3f32(px, py, 0.0F);
        GXNormal3f32(py, -px, 0.0F);

        GXPosition3f32(px*0.5F, py*0.5F, pz+1.0F);
        GXNormal3f32(py, px, 0.0F);
        GXPosition3f32(px*0.5F, py*0.5F, pz);
        GXNormal3f32(py, px, 0.0F);
        GXPosition3f32(px, py, pz);
        GXNormal3f32(py, px, 0.0F);
        GXPosition3f32(px, py, pz+1.0F);
        GXNormal3f32(py, px, 0.0F);

    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           LoadGXMatrices
    
    Description:    Calculates each Pos/Nrm/Tex matrix and
                    loads them into the hardware.

    Arguments:      tmo  : MyTransMtxObj structure which includes
                           view and texgen matrices
                    mm   : model transformation matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void LoadGXMatrices( MyTransMtxObj* tmo, Mtx mm )
{
    Mtx mv, mvi;
    
    // Position and Normal
    MTXConcat(tmo->view, mm, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi);
    MTXTranspose(mvi, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    // Tex coord generation for depth value lookup 
    MTXConcat(tmo->depth, mm, mv);
    GXLoadTexMtxImm(mv, GX_TEXMTX0, GX_MTX3x4);
    // Tex coord genaration for shadow map projection
    MTXConcat(tmo->texProj, mm, mv);
    GXLoadTexMtxImm(mv, GX_TEXMTX1, GX_MTX3x4);
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
    Name:           GetTexProjMtx
    
    Description:    Sets texture coord generation matrix for shadow
                    projection and depth value lookup
                    
    Arguments:      tmo     : MyTransMtxObj structure in which generated
                              texgen matrices should be set
                    cam     : camera from a light for projection
                    bitMode : shadow depth precision mode
                              0 - 8bit / 1 - 16bit
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void GetTexProjMtx( MyTransMtxObj* tmo, MyCameraObj* cam, u8 bitMode )
{
    f32 n, f, range, tscale;
    Mtx proj, dp;

    n = cam->cfg.znear;
    f = cam->cfg.zfar;
    range = f - n;

    // Make a zero matrix
    MTXScale(dp, 0.0F, 0.0F, 0.0F);
        
    // Precision control
    tscale = ( bitMode ) ? 256.0F : 16.0F;
    
    if ( cam->cfg.type == GX_PERSPECTIVE )
    {
        // Shadow projection matrix
        MTXLightFrustum(
            proj,
            - (cam->cfg.top), // t = -y in projected texture 
            cam->cfg.top,
            cam->cfg.left,
            - (cam->cfg.left),
            cam->cfg.znear,
            0.50F,
            0.50F,
            0.50F,
            0.50F );
        
        // Depth lookup matrix
        // in order to generate:
        //     s = (1 + N/z) * F / (F - N)
        //     t = s * tscale due to the texture size
        MTXRowCol(dp, 0, 2) = f / range;
        MTXRowCol(dp, 0, 3) = f * n / range;
        MTXRowCol(dp, 1, 2) = MTXRowCol(dp, 0, 2) * tscale;
        MTXRowCol(dp, 1, 3) = MTXRowCol(dp, 0, 3) * tscale;
        MTXRowCol(dp, 2, 2) = 1.0F;
    }
    else // ( cam->cfg.type == GX_ORTHOGRAPHIC )
    {
        // Shadow projection matrix
        MTXLightOrtho(
            proj,
            - (cam->cfg.top), // t = -y in projected texture 
            cam->cfg.top,
            cam->cfg.left,
            - (cam->cfg.left),
            0.50F,
            0.50F,
            0.50F,
            0.50F );
        
        // Depth lookup matrix
        // in order to generate:
        //     s = - (z + N) / (F - N)
        //     t = s * tscale due to the texture size
        MTXRowCol(dp, 0, 2) = - 1.0F / range;
        MTXRowCol(dp, 0, 3) = - n / range;
        MTXRowCol(dp, 1, 2) = MTXRowCol(dp, 0, 2) * tscale;
        MTXRowCol(dp, 1, 3) = MTXRowCol(dp, 0, 3) * tscale;
        MTXRowCol(dp, 2, 3) = 1.0F;
    }
    
    MTXConcat(proj, cam->view, tmo->texProj);
    MTXConcat(dp,   cam->view, tmo->depth);
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
                    for 1st pass which generates shadow depth map.
                    Actually this pass requires only position data.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeFor1stPass( void )
{
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,    // disable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE );
    
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL,
                  GX_TEXMAP_NULL, GX_COLOR0A0);

#if ( GX_REV == 1 ) // GX revision 1
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_LINEAR);
#endif
    
    GXSetNumTexGens(0);
}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeFor2ndPass8
    
    Description:    Set up shading mode (color channel, TEV, etc.)
                    for 2nd pass which draws actual scene from the
                    viewer with full-scene shadow.

                    [ 8bit precision comparison version ]
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeFor2ndPass8( void )
{
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,     // enable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT0,     // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE );
    GXSetChanAmbColor(GX_COLOR0A0, COL_AMBIENT);
   
#ifndef EMU // real hardware
#if ( GX_REV == 1 )
    //-------------------------------------------
    //  TEV Codes for GX revision 1 (HW1, etc.)
    //-------------------------------------------
    GXSetNumTevStages(3);
    
    // TEV Stage 0  ( Loads a depth value from ramp texture )
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_LINEAR);
    
    // TEV Stage 1  ( REGPREV >= shadow map texture ? 0 : 255 )
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1,
                  GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXRRR,
                    GX_CC_ONE, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO,
                    GX_CS_DIVIDE_2, GX_FALSE, GX_TEVPREV);
    GXSetTevClampMode(GX_TEVSTAGE1, GX_TC_LE);

    // TEV Stage 2 ( REGPREV == 0 ? shadow color : rasterized color )
    // Register 0 is supporsed to hold shadow color
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL,
                  GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_C0, GX_CC_RASC,
                    GX_CC_CPREV, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevClampMode(GX_TEVSTAGE2, GX_TC_LINEAR);

#else // ( GX_REV >= 2 )
    //-------------------------------------------
    //  TEV Codes for GX revision >= 2 (HW2, ...)
    //-------------------------------------------
    GXSetNumTevStages(3);

    // TEV Stage 0  ( Loads a depth value from ramp texture )
    // REGPREV(C) = TEX(I)
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    
    // TEV Stage 1  ( Compare with shadow map texture )
    // REGPREV = ( REGPREV > shadow map texture(R) ? 255 : 0 )
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1,
                  GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC,
                    GX_CC_ONE, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_COMP_R8_GT, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);

    // TEV Stage 2  ( Select shadow/lit color )
    // output = ( REGPREV == 0 ? rasterized color : shadow color )
    // Register 0 is supporsed to hold shadow color
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL,
                  GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_RASC, GX_CC_C0,
                    GX_CC_CPREV, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);

#endif // GX_REV

    // Tex coords
    GXSetNumTexGens(2);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);

#else // EMU
    //-------------------------------------------
    //  For emulator (cannot draw shadows)
    //-------------------------------------------
    GXSetNumTevStages(1);

    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL,
                  GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetNumTexGens(0);
    
#endif // EMU
}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeFor2ndPass16
    
    Description:    Set up shading mode (color channel, TEV, etc.)
                    for 2nd pass which draws actual scene from the
                    viewer with full-scene shadow.

                    [ 16bit precision comparison version ]
                   
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeFor2ndPass16( void )
{
    GXColor col_one = { 1, 1, 1, 1 };
        
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,     // enable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT0,     // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE );
    GXSetChanAmbColor(GX_COLOR0A0, COL_AMBIENT);

#ifndef EMU // real hardware
#if ( GX_REV == 1 )
    //-------------------------------------------
    //  TEV Codes for GX revision 1 (HW1, etc.)
    //-------------------------------------------
    GXSetNumTevStages(5);
       
    // REG2 = constant {1, 1, 1, 1}
    GXSetTevColor(GX_TEVREG2, col_one);
    
    // TEV Stage 0  ( Loads a depth value from ramp texture )
    // TEXA -> REG1(A) / TEXC -> REG1(C)
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVREG1);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVREG1);
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_LINEAR);
    
    // TEV Stage 1 ( Compare Lower 8bit )
    // REGPREV(C) = REG1(C) >= shadow map(G) ? 0 : 255
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1,
                  GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXGGG,
                    GX_CC_ONE, GX_CC_C1);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO,
                    GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_ZERO); // Dummy
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV); // Dummy out
    GXSetTevClampMode(GX_TEVSTAGE1, GX_TC_GE);

    // TEV Stage 2 ( Compare Higher 8 bit )
    // REG1(C) = ( REG1(A) - shadow map(R) ) without clamp
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1,
                  GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXRRR,
                    GX_CC_ONE, GX_CC_A1);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_SUB, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_FALSE, GX_TEVREG1);
    GXSetTevClampMode(GX_TEVSTAGE2, GX_TC_LINEAR);

    // TEV Stage 3 ( Compare Higher 8 bit (cont.) )
    // REGPREV(C) =  REG1(C) - REGPREV(C) * 1/255 >= 0 ? 0 : 255 )
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL,
                  GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_C2,
                    GX_CC_CPREV, GX_CC_C1);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_SUB, GX_TB_ZERO,
                    GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
    GXSetTevClampMode(GX_TEVSTAGE3, GX_TC_GE);

    // TEV Stage 4 ( Select shadow/lit color )
    // output = ( REGPREV == 0 ? shadow color : rasterized color )
    // Register 0 is supporsed to hold shadow color
    GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD_NULL,
                  GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE4, GX_CC_C0, GX_CC_RASC,
                    GX_CC_CPREV, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevClampMode(GX_TEVSTAGE4, GX_TC_LINEAR);

#else // ( GX_REV >= 2 )
    //-------------------------------------------
    //  TEV Codes for GX revision >= 2 (HW2, ...)
    //-------------------------------------------
    GXSetNumTevStages(3);

    // Make a swap table which performs A->G, I->R conversion
    GXSetTevSwapModeTable(GX_TEV_SWAP1,
                          GX_CH_RED,  GX_CH_ALPHA,
                          GX_CH_BLUE, GX_CH_ALPHA);
    // Make a swap table which performs I->G, A->R conversion
    GXSetTevSwapModeTable(GX_TEV_SWAP2,
                          GX_CH_ALPHA, GX_CH_GREEN,
                          GX_CH_BLUE,  GX_CH_ALPHA);

    // TEV Stage 0  ( Loads a depth value from ramp texture )
    // REGPREV(R) = TEX(I) / REGPREV(G) = TEX(A)
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
    
    // TEV Stage 1  ( Compare G+R 16bit value )
    // REGPREV(C) = REGPREV(GR) > shadow map texture(GR) ? 255 : 0
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1,
                  GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC,
                    GX_CC_ONE, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_COMP_GR16_GT, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP2);

    // TEV Stage 2  ( Select shadow/lit color )
    // output = ( REGPREV == 0 ? rasterized color : shadow color )
    // Register 0 is supporsed to hold shadow color
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL,
                  GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_RASC, GX_CC_C0,
                    GX_CC_CPREV, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);

#endif // GX_REV

    // Tex coords
    GXSetNumTexGens(2);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);

#else // EMU
    //-------------------------------------------
    //  For emulator (cannot draw shadows)
    //-------------------------------------------
    GXSetNumTevStages(1);

    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL,
                  GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetNumTexGens(0);
    
#endif // EMU
}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeForProj8
    
    Description:    Set up shading mode (color channel, TEV, etc.)
                    for 2nd pass.
                    This mode displays projected shadow map only.
 
                    [ 8bit precision comparison version ]
                   
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeForProj8( void )
{
#ifndef EMU // real hardware
    //-------------------------------------------
    //  TEV Codes for real hardware
    //-------------------------------------------
    GXSetNumTevStages(1);
    
    // Color mask value for displaying only red component.
    GXSetTevColor(GX_TEVREG1, COL_RED);

    // TEV stage 0 ( output = TEX(C) * TEVREG1(Red) )
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC,
                    GX_CC_C1, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

#if ( GX_REV == 1 ) // GX revision 1
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_LINEAR);
#else               // GX revision 2 or later
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
#endif

    // No color channel necessary
    GXSetNumChans(0);

#else // EMU
    //-------------------------------------------
    //  For emulator
    //-------------------------------------------
    // The emulator uses second color channel to do color mask
    GXSetNumChans(2);
    GXSetChanCtrl(
        GX_COLOR1A1,
        GX_DISABLE,    // disable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE );
    GXSetChanMatColor(GX_COLOR1A1, COL_RED);

    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR1A1);

#endif // EMU
    
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);
}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeForProj16
    
    Description:    Set up shading mode (color channel, TEV, etc.)
                    for 2nd pass.
                    This mode displays projected shadow map only.

                    [ 16bit precision comparison version ]
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeForProj16( void )
{
#ifndef EMU // real hardware
#if ( GX_REV == 1 ) // GX revision 1
    //-------------------------------------------
    //  TEV Codes for GX revision 1 (HW1, etc.)
    //-------------------------------------------
    GXSetNumTevStages(1);

    // Color mask value for displaying only Red/Green components.
    GXSetTevColor(GX_TEVREG1, COL_YELLOW);

    // TEV stage 0 ( output = TEX(C) * TEVREG1(Yellow) )
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC,
                    GX_CC_C1, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                   GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_LINEAR);

#else // ( GX_REV >= 2 )
    //-------------------------------------------
    //  TEV Codes for GX revision >= 2 (HW2, ...)
    //-------------------------------------------
    GXSetNumTevStages(1);

    // Mask color which show only Red/Green components.
    GXSetTevColor(GX_TEVREG1, COL_YELLOW);

    // Make a swap table which performs I->R, A->G conversion
    GXSetTevSwapModeTable(GX_TEV_SWAP1,
                          GX_CH_RED,   GX_CH_ALPHA,
                          GX_CH_BLUE,  GX_CH_ALPHA);

    // TEV stage 0 ( output = TEX(AII) * TEVREG1(Yellow) )
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C1,
                    GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);

#endif // GX_REV

    // No color channel necessary
    GXSetNumChans(0);

#else // EMU
    //-------------------------------------------
    //  For emulator
    //-------------------------------------------
    // The emulator uses second color channel for color mask
    GXSetNumChans(2);
    GXSetChanCtrl(
        GX_COLOR1A1,
        GX_DISABLE,    // disable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE );
    GXSetChanMatColor(GX_COLOR1A1, COL_YELLOW);

    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR1A1);

#endif // EMU
    
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);
}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeForDepth8
    
    Description:    Set up shading mode (color channel, TEV, etc.)
                    for 2nd pass.
                    This mode shows acturl depth from the light.
                    
                    [ 8bit precision version ]
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeForDepth8( void )
{
#ifndef EMU // real hardware
    //-------------------------------------------
    //  TEV Codes for real hardware
    //-------------------------------------------
    GXSetNumTevStages(1);

    // Color mask value for displaying only red component.
    GXSetTevColor(GX_TEVREG1, COL_RED);
          
    // TEV stage 0 ( output = TEX(I) * TEVREG1(Red) )
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC,
                    GX_CC_C1, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

#if ( GX_REV == 1 ) // GX revision 1
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_LINEAR);
#else               // GX revision 2 or later
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
#endif

    // No color channel necessary
    GXSetNumChans(0);

#else // EMU
    //-------------------------------------------
    //  For emulator
    //-------------------------------------------
    // The emulator uses second color channel for color mask
    GXSetNumChans(2);
    GXSetChanCtrl(
        GX_COLOR1A1,
        GX_DISABLE,    // disable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE );
    GXSetChanMatColor(GX_COLOR1A1, COL_RED);

    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE); // To display only Red component
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR1A1);

#endif // EMU

    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeForDepth16
    
    Description:    Set up shading mode (color channel, TEV, etc.)
                    for 2nd pass.
                    This mode shows acturl depth from the light.

                    [ 16bit precision version ]
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetShaderModeForDepth16( void )
{

#ifndef EMU // real hardware
#if ( GX_REV == 1 )
    //-------------------------------------------
    //  TEV Codes for GX revision 1 (HW1, etc.)
    //-------------------------------------------
    // need to convert A->R and I->G
    GXSetNumTevStages(2);
    GXSetTevColor(GX_TEVREG1, COL_RED);
    GXSetTevColor(GX_TEVREG2, COL_GREEN);

    // TEV stage 0 ( TEVPREV = Tex alpha * TEVREG1 color )
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C1,
                    GX_CC_TEXA, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevClampMode(GX_TEVSTAGE0, GX_TC_LINEAR);

    // TEV stage 1 ( output = TEVPREV + Tex color * TEVREG2 color )
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_C2,
                    GX_CC_TEXC, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevClampMode(GX_TEVSTAGE1, GX_TC_LINEAR);

#else // ( GX_REV >= 2 )
    //-------------------------------------------
    //  TEV Codes for GX revision >= 2 (HW2, ...)
    //-------------------------------------------
    GXSetNumTevStages(1);
    
    // Mask color which show only G/R components
    GXSetTevColor(GX_TEVREG1, COL_YELLOW);

    // Make a swap table which performs A->R, I->G conversion
    GXSetTevSwapModeTable(GX_TEV_SWAP1,
                          GX_CH_ALPHA, GX_CH_GREEN,
                          GX_CH_BLUE,  GX_CH_ALPHA);

    // TEV stage 0 ( output = TEX(AII) * TEVREG1(Yellow) )
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C1,
                    GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);

#endif // GX_REV

    // No color channel necessary
    GXSetNumChans(0);

#else // EMU
    //-------------------------------------------
    //  For emulator ( displays Red only )
    //-------------------------------------------
    // The emulator uses second color channel for color mask
    GXSetNumChans(2);
    GXSetChanCtrl(
        GX_COLOR1A1,
        GX_DISABLE,    // disable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE );
    GXSetChanMatColor(GX_COLOR1A1, COL_RED);

    // Can display only Red component
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR1A1);

#endif // EMU

    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
}

/*---------------------------------------------------------------------------*
    Name:           CreateRampTex8
    
    Description:    Create a ramp texture (8bit version) which is used
                    to lookup depth value from a light.
                    This function is called only once at the beginning.
                    Actually you can prepare this ramp texture as static
                    data.

                    Created texture data (I8 format) is like this:
                    
                    | 00 10 20 30 .... F0 |
                    | 01 11 21 31 .... F1 |
                    |  :  :  :  :       : |
                    | 0F 1F 2F 3F .... FF |
                   
    Arguments:      to : a texture object where the texture data should be set
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void CreateRampTex8( GXTexObj* to )
{
    u8*   data;
    u32   size, i, offset;
    
    size = GXGetTexBufferSize(16, 16, GX_TF_I8, GX_FALSE, 0);
    data = (u8*)OSAlloc(size);
    
    // Write ramp data pattern
    for ( i = 0 ; i < 0x100 ; ++i )
    {
        // I8 format tile offset
        offset = ((i & 0x80) >> 2)
               + ((i & 0x70) >> 4)
               + ((i & 0x0C) << 4)
               + ((i & 0x03) << 3);
        *(data+offset) = (u8)i;
    }
   
    // Initialize texture object properties
    GXInitTexObj(to, data, 16, 16, GX_TF_I8, GX_CLAMP, GX_REPEAT, GX_FALSE);
    GXInitTexObjLOD(to, GX_NEAR, GX_NEAR, 0, 0, 0, 0, 0, GX_ANISO_1);

    // Make sure data is written into the main memory.
    DCFlushRange(data, size);
}

/*---------------------------------------------------------------------------*
    Name:           CreateRampTex16
    
    Description:    Create a ramp texture (16bit version) which is used
                    to lookup depth value from a light.
                    This function is called only once at the beginning.
                    Actually you can prepare this ramp texture as static
                    data.

                    Created texture data (IA8 format) is like this:
                    
                    | 0000 0100 0200 .... FF00 |
                    | 0001 0101 0201 .... FF01 |
                    |   :    :    :         :  |
                    | 00FF 01FF 02FF .... FFFF |
                   
    Arguments:      to : a texture object where the texture data should be set
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void CreateRampTex16( GXTexObj* to )
{
    u16*  data;
    u32   size, i, offset;
    
    size = GXGetTexBufferSize(256, 256, GX_TF_IA8, GX_FALSE, 0);
    data = (u16*)OSAlloc(size);
    
    // Write ramp data pattern
    for ( i = 0 ; i < 0x10000 ; ++i )
    {
        // IA8 format tile offset
        offset = ((i & 0xFC00) >> 6)
               + ((i & 0x0300) >> 8)
               + ((i & 0x00FC) << 8)
               + ((i & 0x0003) << 2);
        *(data+offset) = (u16)i;
    }
   
    // Initialize texture object properties
    GXInitTexObj(to, data, 256, 256, GX_TF_IA8, GX_CLAMP, GX_REPEAT, GX_FALSE);
    GXInitTexObjLOD(to, GX_NEAR, GX_NEAR, 0, 0, 0, 0, 0, GX_ANISO_1);

    // Make sure data is written into the main memory.
    DCFlushRange(data, size);
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
    OSReport("tg-shadow3: full-scene shadow by Z buffer copy\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main stick : Move the camera\n");
    OSReport("Sub  stick : Move the light\n");
    OSReport("A button   : Change display mode\n");
    OSReport("B button   : Change projection type\n");
    OSReport("Y button   : Change shadow map size\n");
    OSReport("******************************************************\n");
    OSReport("\n\n");
}

/*============================================================================*/
