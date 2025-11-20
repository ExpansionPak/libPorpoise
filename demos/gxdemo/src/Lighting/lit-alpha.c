/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-alpha.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-alpha.c $
    
    5     5/12/01 11:25p Hirose
    removed obsolete flags
    
    4     11/16/00 7:51p Carl
    Added mode to show both channels at once.
    
    3     3/23/00 4:56p Hirose
    updated to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    7     3/03/00 11:41p Hirose
    drawing axes stuff is recovered
    
    6     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    5     2/25/00 12:51a Hirose
    updated pad control functions to match actual HW1 gamepad
    
    4     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    3     1/25/00 2:54p Carl
    Changed to standardized end of test message
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    25    11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    24    11/12/99 4:29p Yasu
    Add GXSetNumTexGens(0)  for GX_PASSCLR
    
    23    11/12/99 3:39p Hirose
    changed single frame test to check dual channel
    
    22    10/29/99 6:58p Hirose
    added dual color channel test
    
    21    10/18/99 5:27p Hirose
    changed value for light mask
    
    20    10/07/99 3:31p Hirose
    changed camera portion / fixed some comments
    
    19    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    18    9/28/99 5:28p Hirose
    added single frame test / fixed some comments
    
    17    9/20/99 11:46a Ryan
    update to use new DEMO calls
    
    16    9/13/99 11:24p Hirose
    confirmed alignment/coherency
    removed all warnings
    
    15    9/02/99 5:35p Hirose
    
    14    8/31/99 4:57p Hirose
    turned all math functions into f32 version
    eliminated all tab characters
    
    13    8/28/99 9:39p Hirose
    
    12    8/27/99 2:29p Yasu
    Change function paramters of GXSetBlendMode()
    
    11    8/26/99 2:43p Yasu
    Replace GXSetColorMode() to GXSetBlendMode().
    
    10    8/20/99 5:09p Hirose
    
    9     8/16/99 10:36p Hirose
    
    8     8/13/99 4:34p Hirose
    
    7     8/13/99 12:14a Hirose
    
    6     8/06/99 2:09p Hirose
    turned light position into camera space
    
    5     7/23/99 2:53p Ryan
    changed dolphinDemo.h to demo.h
    
    4     7/23/99 12:16p Ryan
    included dolphinDemo.h
    
    3     7/21/99 11:33p Hirose
    
    2     7/21/99 12:42p Hirose
    
    1     7/20/99 7:42p Hirose
    Created (Simply copied lit-test01.c)
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-alpha
     One RGB light and one alpha light test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI             3.14159265358979323846F
#define NUM_LIGHTS     4
#define NUM_COLORMODES 3

#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
  Structure Definitions
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
    GXLightObj  lobj;
    s32         theta;
    s32         phi;
    u32         color;
} MyLightObj;

// for entire scene control
typedef struct
{
    MyCameraObj cam;
    MyLightObj  lightCtrl[NUM_LIGHTS];
    u32         modelType;
    Mtx         modelCtrl;
    u32         current;
    u32         colorMode;
    s32         axesFlag;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void DrawModel       ( u32 model );
static void DrawAxes        ( void );
static void DrawLightMark   ( u8 col );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MyLightObj* lightCtrl, Mtx v );
static void DisableLight    ( void );
static void SetColorMode    ( u32 mode );
static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
  Model & Lighting Data
 *---------------------------------------------------------------------------*/
#define REG_AMBIENT0   MyColors[4]
#define REG_MATERIAL0  MyColors[5]
#define REG_AMBIENT1   MyColors[6]
#define REG_MATERIAL1  MyColors[7]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {0xff, 0xff, 0xff, 0x00},  // color light 0
    {0x00, 0x00, 0x00, 0xff},  // alpha light 0
    {0xff, 0x80, 0x80, 0x00},  // color light 1
    {0x00, 0x00, 0x00, 0xff},  // alpha light 1
    {0x20, 0x20, 0x20, 0x00},  // ambient for channel 0
    {0x60, 0xc0, 0xc0, 0xc0},  // material for channel 0
    {0x00, 0x00, 0xe0, 0x20},  // ambient for channel 1
    {0xff, 0xff, 0xff, 0xff}   // material for channel 1
};

#define SPHERE      0
#define CYLINDER    1
#define ICOSA       2
#define DODECA      3
#define OCTA        4
#define CUBE        5

#define MODELS      6

/*---------------------------------------------------------------------------*
   Strings for messages
 *---------------------------------------------------------------------------*/
static char* LightNameMsg[NUM_LIGHTS+1] =
{
    "Color Light on channel 0",
    "Alpha Light on channel 0",
    "Color Light on channel 1",
    "Alpha Light on channel 1",
    "Model"
};

static char* ColorModeMsg[NUM_COLORMODES] =
{
    "GX_COLOR0A0 only",
    "GX_COLOR1A1 only"
    "GX_COLOR0A0 and GX_COLOR1A1"
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 900.0F }, // location
    { 0.0F, 1.0F,   0.0F }, // up
    { 0.0F, 0.0F,   0.0F }, // tatget
    -320.0F, // left
    240.0F,  // top
    400.0F,  // near
    2000.0F  // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;    // scene control parameters

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
    
    DrawInit(&SceneCtrl);       // Initialize vertex formats, scene and etc.

    PrintIntro();  // Print demo directions
    
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);   // Draw the model.
        DEMODoneRender();
        DEMOPadRead();          // Update controller status
        AnimTick(&SceneCtrl);   // Update animation.
    }
    
    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format, and sets
                    the array pointers and strides for the indexed data.
                    This function also sets up default scene parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{
    u32 i;

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // set tev to use vertex color
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);


    // Default scene parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // light parameters
    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        sc->lightCtrl[i].theta  = 0;
        sc->lightCtrl[i].phi    = 0;
        sc->lightCtrl[i].color  = i;
    }
    
    // color channel to be displayed
    sc->colorMode = 0;

    // model type and current control number
    sc->modelType = 0;
    sc->current   = 0;
    sc->axesFlag  = 0;
    
    // model control matrix
    MTXScale(sc->modelCtrl, 160.0F, 160.0F, 160.0F);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mr;  // Rotate matrix.
    Mtx  mv;  // Modelview matrix.
    Mtx  mvi; // Modelview matrix.

    // disable lights and blend mode
    DisableLight();
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY);

    // set tev to use color channel 0
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // Draw light direction
    if ( sc->current < NUM_LIGHTS )
    {
        u8  col;
    
        MTXRotDeg(mr, 'y', sc->lightCtrl[sc->current].theta);
        MTXConcat(sc->cam.view, mr, mv);
        MTXRotDeg(mr, 'x', - sc->lightCtrl[sc->current].phi);
        MTXConcat(mv, mr, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
        
        col = (u8)(( sc->current % 2 == 0 ) ? 255 : 128);
        DrawLightMark(col);
    }

    // model view matrix
    MTXConcat(sc->cam.view, sc->modelCtrl, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi);
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    // Axes are drawn if the flag is set.
    if ( sc->axesFlag )
    {
        DrawAxes();
    }

    // set lights and choose a color channel to be displayed
    SetLight(sc->lightCtrl, sc->cam.view);
    SetColorMode(sc->colorMode);
    
    // draw a model ( opaque one )
    DrawModel(sc->modelType);

    // enable alpha blend mode
    GXSetBlendMode(GX_BM_BLEND, GX_BL_INVSRCALPHA, GX_BL_SRCALPHA, GX_LO_NOOP);

    // model view matrix for another scale
    MTXConcat(sc->cam.view, sc->modelCtrl, mv);
    MTXScale(mr, 1.5F, 1.5F, 1.5F);
    MTXConcat(mv, mr, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    // draw a model ( transparent one )
    DrawModel(sc->modelType);

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
    s32  sx, sy;
    u32  cur = sc->current;
    u16  down = DEMOPadGetButtonDown(0);

    // Model Rotation Calculation   
    sx =   DEMOPadGetSubStickX(0) / 24;
    sy = - DEMOPadGetSubStickY(0) / 24;    
    MTXRotDeg(mry, 'x', sy);
    MTXRotDeg(mrx, 'y', sx);
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);

    // Axes display mode on/off
    sc->axesFlag = ( sx == 0 && sy == 0 ) ? 0 : 1;


    // Light Position Calculation
    sc->lightCtrl[cur].theta += ( DEMOPadGetStickX(0) / 24 );
    sc->lightCtrl[cur].theta = sc->lightCtrl[cur].theta % 360;
        
    sc->lightCtrl[cur].phi += ( DEMOPadGetStickY(0) / 24 );
    Clamp(sc->lightCtrl[cur].phi, -90, 90);


    // Model Change
    if ( down & PAD_BUTTON_A )
    {
        sc->modelType = ( sc->modelType + 1 ) % MODELS;
    }

    // Select Color Channel to be displayed
    if ( down & PAD_BUTTON_B )
    {
        sc->colorMode = ( sc->colorMode + 1 ) % NUM_COLORMODES;
        OSReport("Displayed: %s\n", ColorModeMsg[sc->colorMode]);
    }

    // Light Select
    if ( down & PAD_TRIGGER_R )
    {
        sc->current = ( sc->current + 1 ) % NUM_LIGHTS;
        OSReport("%s\n", LightNameMsg[sc->current]);
    }
    if ( down & PAD_TRIGGER_L )
    {
        sc->current = ( sc->current + NUM_LIGHTS - 1 ) % NUM_LIGHTS;
        OSReport("%s\n", LightNameMsg[sc->current]);
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawModel
    
    Description:    Draws the specified model
                    
    Arguments:      model : specifies which model is to be displayed
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModel( u32 model )
{
    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);

    switch(model)
    {
        case CYLINDER :
            GXDrawCylinder(32);
            break;
        case SPHERE :
            GXDrawSphere(12, 24);
            break;
        case CUBE :
            GXDrawCube();
            break;
        case OCTA :
            GXDrawOctahedron();
            break;
        case ICOSA :
            GXDrawIcosahedron();
            break;
        case DODECA :
            GXDrawDodeca();
            break;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawAxes
    
    Description:    Draws xyz-axes which shows model's direction
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawAxes( void )
{
    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GXBegin(GX_LINES, GX_VTXFMT0, 6);
        GXPosition3f32(0.0F, 0.0F, -1000.0F);
        GXColor4u8(96, 96, 96, 255);
        GXPosition3f32(0.0F, 0.0F, 1000.0F);
        GXColor4u8(96, 96, 96, 255);

        GXPosition3f32(0.0F, -1000.0F, 0.0F);
        GXColor4u8(96, 96, 96, 255);
        GXPosition3f32(0.0F, 1000.0F, 0.0F);
        GXColor4u8(96, 96, 96, 255);

        GXPosition3f32(-1000.0F, 0.0F, 0.0F);
        GXColor4u8(96, 96, 96, 255);
        GXPosition3f32(1000.0F, 0.0F, 0.0F);
        GXColor4u8(96, 96, 96, 255);
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawLightMark
    
    Description:    Draws a mark which shows position of the light.
                    
    Arguments:      col : intensity parameter of the mark
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawLightMark( u8 col )
{
    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GXBegin(GX_LINES, GX_VTXFMT0, 8);
        GXPosition3f32(500.0F, 500.0F, -500.0F);
        GXColor4u8(col, col, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 500.0F);
        GXColor4u8(col, col, 255, 255);

        GXPosition3f32(500.0F, -500.0F, -500.0F);
        GXColor4u8(col, col, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 500.0F);
        GXColor4u8(col, col, 255, 255);

        GXPosition3f32(-500.0F, 500.0F, -500.0F);
        GXColor4u8(col, col, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 500.0F);
        GXColor4u8(col, col, 255, 255);

        GXPosition3f32(-500.0F, -500.0F, -500.0F);
        GXColor4u8(col, col, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 500.0F);
        GXColor4u8(col, col, 255, 255);
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    sets view matrix and loads projection matrix into hardware
                    
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
    
    Description:    Sets up light & channel parameters.
                    
    Arguments:      lightCtrl : pointer to array of MyLightObj structure.
                    view      : view matrix.
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetLight( MyLightObj* lightCtrl, Mtx view )
{
    f32 theta, phi;
    Vec lpos;
    u32 i;

    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        // Light Position
        theta = (f32)lightCtrl[i].theta * PI / 180.0F;
        phi   = (f32)lightCtrl[i].phi   * PI / 180.0F;
        lpos.x = 500.0F * cosf(phi) * sinf(theta);
        lpos.y = 500.0F * sinf(phi);
        lpos.z = 500.0F * cosf(phi) * cosf(theta);

        // convert light position into view space
        MTXMultVec(view, &lpos, &lpos);
    
        GXInitLightPos(&lightCtrl[i].lobj, lpos.x, lpos.y, lpos.z);
        GXInitLightColor(&lightCtrl[i].lobj, MyColors[lightCtrl[i].color]);
    }

    // load each light into hardware
    GXLoadLightObjImm(&lightCtrl[0].lobj, GX_LIGHT0);
    GXLoadLightObjImm(&lightCtrl[1].lobj, GX_LIGHT1);
    GXLoadLightObjImm(&lightCtrl[2].lobj, GX_LIGHT2);
    GXLoadLightObjImm(&lightCtrl[3].lobj, GX_LIGHT3);

    // this test uses two color channels
    GXSetNumChans(2);

    // light channel setting
    GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT1,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);
    GXSetChanCtrl(
        GX_COLOR1,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT2,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);
    GXSetChanCtrl(
        GX_ALPHA1,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT3,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);
    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0A0, REG_AMBIENT0);
    GXSetChanAmbColor(GX_COLOR1A1, REG_AMBIENT1);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, REG_MATERIAL0);
    GXSetChanMatColor(GX_COLOR1A1, REG_MATERIAL1);

    return;
}

/*---------------------------------------------------------------------------*
    Name:           DisableLight
    
    Description:    Disables lighting
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DisableLight( void )
{
    // use only one color channel to aviod wasting performance.
    GXSetNumChans(1);

    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,    // disable channel
        GX_SRC_VTX,    // amb source
        GX_SRC_VTX,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_NONE,    // diffuse function
        GX_AF_NONE);
    GXSetChanCtrl(
        GX_COLOR1A1,
        GX_DISABLE,    // disable channel
        GX_SRC_VTX,    // amb source
        GX_SRC_VTX,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_NONE,    // diffuse function
        GX_AF_NONE);
}

/*---------------------------------------------------------------------------*
    Name:           SetColorMode
    
    Description:    select color channel or set blend operation of two colors
                    
    Arguments:      mode : color mode
                        
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetColorMode( u32 mode )
{
    switch(mode)
    {
      case 0:
        GXSetNumTexGens(0);
        GXSetNumTevStages(1);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        break;
      case 1:
        GXSetNumTexGens(0);
        GXSetNumTevStages(1);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
        break;
      case 2:
        GXSetNumTexGens(0);
        GXSetNumTevStages(2);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE1, GX_PASSCLR);
        GXSetTevColorIn(GX_TEVSTAGE1,
                        GX_CC_ZERO, GX_CC_CPREV, GX_CC_ONE, GX_CC_RASC);
        GXSetTevAlphaIn(GX_TEVSTAGE1,
                        GX_CA_ZERO, GX_CA_APREV, GX_CA_ONE, GX_CA_RASA);
        
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
        break;
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
    OSReport("lit-alpha: one RGB light and one Alpha light\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick   : Move selected Light\n");
    OSReport("Sub  Stick   : Rotate the model\n");
    OSReport("L/R Triggers : Select a light\n");
    OSReport("A Button     : Change the Model\n");
    OSReport("B Button     : Select color channel to be used\n");
    OSReport("************************************************\n");
    OSReport("\n\n");

    OSReport("Displayed: %s\n", ColorModeMsg[0]);
    OSReport("%s\n", LightNameMsg[0]);
}

/*============================================================================*/
