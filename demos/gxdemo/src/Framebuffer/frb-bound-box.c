/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     frb-bound-box.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Framebuffer/frb-bound-box.c $    
    
    2     5/05/01 10:05p Hirose
    removed obsolete flags
    
    1     5/24/00 6:09p Hirose
    initial check in
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    frb-bound-box
        bounding box feature demo
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI          3.14159265F

#define MAX_Z       0x00ffffff // max value of Z buffer

#define NUM_CUBES   8
#define BOUNDARY    400

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

// for cube objects
typedef struct
{
    Vec         pos;
    Vec         vel;
    Vec         axis;
    f32         deg;
} MyCubeObj;

// bounding box
typedef u16 MyBoundingBox[4];

// for entire scene control
typedef struct
{
    MyCameraObj cam;
    MyCubeObj   cube[NUM_CUBES];
    u16         screen_width;
    u16         screen_height;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main                ( void );
static void DrawInit            ( MySceneCtrlObj* sc );
static void DrawTick            ( MySceneCtrlObj* sc );
static void AnimTick            ( MySceneCtrlObj* sc );
static void DrawCubes           ( MySceneCtrlObj* sc );
static void DrawBoundingBox     ( MyBoundingBox bb );
static void SetCamera           ( MyCameraObj* cam );
static void SetScreenSpaceMode  ( MySceneCtrlObj* sc );
static void SetLight            ( void );
static void DisableLight        ( void );
static void PrintIntro          ( void );

/*---------------------------------------------------------------------------*
  Model and texture data
 *---------------------------------------------------------------------------*/
// for cube models
#define REG_AMBIENT  ColorArray[NUM_CUBES]
#define LIGHT_COLOR  ColorArray[NUM_CUBES+1]
#define BG_COLOR     ColorArray[NUM_CUBES+2]

static GXColor ColorArray[NUM_CUBES+3] ATTRIBUTE_ALIGN(32) = 
{
    { 0x80, 0xFF, 0x80, 0xFF },
    { 0x00, 0xFF, 0xFF, 0xFF },
    { 0xFF, 0x00, 0xFF, 0xFF },
    { 0xFF, 0xFF, 0x00, 0xFF },
    { 0x20, 0x20, 0xFF, 0x80 },
    { 0x20, 0xA0, 0x00, 0x80 },
    { 0xC0, 0xC0, 0xC0, 0x40 },
    { 0xFF, 0x80, 0x80, 0x40 },
    { 0x40, 0x40, 0x40, 0xFF },    // Gray  (Ambient etc.)
    { 0xFF, 0xFF, 0xFF, 0xFF },    // White (Light etc.)
    { 0x00, 0x00, 0x00, 0x00 }     // Background
};

static Vec CubeIniData[NUM_CUBES*3] =
{
    // Position          Velocity                 Rotation axis
    { -300, -150,  30 }, {  5.0F,  2.5F,  5.0F }, {  0.0F,  1.0F,  1.0F },
    {  300, -150,  30 }, {  5.0F, -7.5F, -2.5F }, {  0.5F,  0.0F, -1.0F },
    { -200,  250,   0 }, {  2.5F, -2.5F, -7.5F }, { -0.5F, -1.0F,  0.0F },
    {  200,  250,   0 }, { -5.0F, -5.0F, -5.0F }, {  1.0F,  1.0F,  1.0F },
    { -300,   50, -50 }, { 10.0F, -2.5F, -2.5F }, {  0.0F, -0.5F, -1.0F },
    {  300,   50, -50 }, { -2.5F,  5.0F,  2.5F }, {  1.0F,  0.0F, -1.0F },
    { -200,   50, -80 }, {  0.0F,  7.5F, -7.5F }, { -1.0F, -0.5F,  0.5F },
    {  200,   50, -80 }, {  7.5F, -2.5F,  5.0F }, { -0.5F,  0.5F,  1.0F }
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    {   0.0F, 0.0F, 800.0F }, // location
    {   0.0F, 1.0F, 0.0F }, // up
    {   0.0F, 0.0F, 0.0F }, // target
    -160.0F,  // left
     120.0F,  // top
     200.0F,  // near
    2000.0F   // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;                // scene control parameters

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);       // Init the OS, game pad, graphics and video.

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

    OSHalt("End of test");
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
    GXRenderModeObj *rmp;
    u32             i;

    // Get framebuffer size of current rendering mode
    rmp = DEMOGetRenderModeObj();
    sc->screen_width  = rmp->fbWidth;
    sc->screen_height = rmp->efbHeight;

    //  Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    //  Array Pointers and Strides
    GXSetArray(GX_VA_CLR0, ColorArray, 4 * sizeof(u8));

    // Z compare, pixel format and background color
    GXSetCopyClear(BG_COLOR, MAX_Z);


    // Default scene control parameter settings

    // camera
    sc->cam.cfg   = DefaultCamera;

    // cube objects
    for ( i = 0 ; i < NUM_CUBES ; ++i )
    {
        sc->cube[i].pos  = CubeIniData[i*3];
        sc->cube[i].vel  = CubeIniData[i*3+1];
        sc->cube[i].axis = CubeIniData[i*3+2];
        sc->cube[i].deg  = 0.0F;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    static u16      token = 0;
    MyBoundingBox   bbox;

    // Clear bounding box on HW
    GXClearBoundingBox();
    
    // Set up camera
    SetCamera(&sc->cam);
    
    // Draw cube models
    DrawCubes(sc);
    
    // Make sure every cubes are drawn
    GXSetDrawSync(++token);
    while ( GXReadDrawSync() != token )
    {}
    
    // Get bounding box from HW
    GXReadBoundingBox(&bbox[0], &bbox[1], &bbox[2], &bbox[3]);
    
    // Draw the bounding box obtaind
    SetScreenSpaceMode(sc);
    DrawBoundingBox(bbox);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
#define VelReflect(cmp) \
    if ( sc->cube[i].pos.cmp < - BOUNDARY )                 \
        sc->cube[i].vel.cmp = fabsf(sc->cube[i].vel.cmp);   \
    if ( sc->cube[i].pos.cmp > BOUNDARY )                   \
        sc->cube[i].vel.cmp = - fabsf(sc->cube[i].vel.cmp);

static void AnimTick( MySceneCtrlObj* sc )
{
    u32  i;

    // Animate cubes
    if ( !(DEMOPadGetButton(0) & PAD_BUTTON_A) )
    {
        for ( i = 0 ; i < NUM_CUBES ; ++i )
        {
            // rotation
            sc->cube[i].deg += 5;
            if ( sc->cube[i].deg > 360 )
            {
                sc->cube[i].deg -= 360;
            }
            
            // position
            VECAdd(&sc->cube[i].pos, &sc->cube[i].vel, &sc->cube[i].pos);
            
            // velocity
            VelReflect(x);
            VelReflect(y);
            VelReflect(z);
        }
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawCubes
    
    Description:    Draw cube models
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawCubes( MySceneCtrlObj* sc )
{
    u32     i;
    Mtx     mt, mr, ms, mv, mvi;

    // enable lighting
    SetLight();
    
    // set Tev operation to use vertex color
    GXSetNumTexGens(0);
	GXSetNumChans(1);
	GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    for ( i = 0 ; i < NUM_CUBES ; ++i )
    {
        MTXTrans(mt, sc->cube[i].pos.x, sc->cube[i].pos.y, sc->cube[i].pos.z);
        MTXConcat(sc->cam.view, mt, mv);
        MTXScale(ms, 100.0F, 100.0F, 100.0F);
        MTXConcat(mv, ms, mv);
        MTXRotAxisDeg(mr, &sc->cube[i].axis, sc->cube[i].deg);
        MTXConcat(mv, mr, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
        MTXInverse(mv, mvi); 
        MTXTranspose(mvi, mv);
        GXLoadNrmMtxImm(mv, GX_PNMTX0);
        
        GXSetChanMatColor(GX_COLOR0A0, ColorArray[i]);
        
        GXDrawCube();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawBoundingBox
    
    Description:    Draw the bounding box obtained by the API
                    
    Arguments:      bb : a bounding box object which contains
                    (left, top, right, bottom) positions
     
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawBoundingBox( MyBoundingBox bb )
{
    s16  x0, x1, y0, y1;

    // set TEV operation
    GXSetNumTexGens(0);
	GXSetNumChans(1);
	GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // disable light
    DisableLight();

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    x0 = (s16)bb[0];
    y0 = (s16)bb[1];
    x1 = (s16)bb[2];
    y1 = (s16)bb[3];

    // draw the box
    GXBegin(GX_LINESTRIP, GX_VTXFMT0, 5);
        GXPosition3s16(x0, y0, 0);
        GXColor4u8(255, 255, 255, 255);
        GXPosition3s16(x1, y0, 0);
        GXColor4u8(255, 255, 255, 255);
        GXPosition3s16(x1, y1, 0);
        GXColor4u8(255, 255, 255, 255);
        GXPosition3s16(x0, y1, 0);
        GXColor4u8(255, 255, 255, 255);
        GXPosition3s16(x0, y0, 0);
        GXColor4u8(255, 255, 255, 255);
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
    Name:           SetScreenSpaceMode
    
    Description:    set projection matrix up to screen coordinate system
                    
    Arguments:      none
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetScreenSpaceMode( MySceneCtrlObj* sc )
{
    Mtx44  proj;
    Mtx    mv;

    MTXOrtho( proj, 0.0F, sc->screen_height - 1, 0.0f, sc->screen_width - 1, 0.0f, - MAX_Z );
    GXSetProjection( proj, GX_ORTHOGRAPHIC );
    MTXIdentity(mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
}

/*---------------------------------------------------------------------------*
    Name:           SetLight
    
    Description:    Sets light objects and color channels
                    
    Arguments:      le   : pointer to a MyLightEnvObj structure
                    view : view matrix.
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetLight( void )
{
    GXLightObj  lobj;

    // set up light position and color
    GXInitLightPos(&lobj, 0.0F, 0.0F, 10000.0F); // almost parallel
    GXInitLightColor(&lobj, LIGHT_COLOR);
    GXLoadLightObjImm(&lobj, GX_LIGHT0);

    // channel setting
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,        // enable channel
        GX_SRC_REG,       // amb source
        GX_SRC_REG,       // mat source
        GX_LIGHT0,        // light mask
        GX_DF_CLAMP,      // diffuse function
        GX_AF_NONE);      // attenuation function
    // channel ambient
    GXSetChanAmbColor(GX_COLOR0A0, REG_AMBIENT);
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
        GX_DISABLE,    // disable channel
        GX_SRC_REG,    // amb source
        GX_SRC_VTX,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_NONE,    // diffuse function
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
    OSReport("************************************************\n");
    OSReport("frb-bound-box: bounding box feature demo\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("A Button : Pause animation while pressed\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
