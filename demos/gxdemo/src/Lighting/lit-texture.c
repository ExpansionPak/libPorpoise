/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-texture.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-texture.c $
    
    5     5/12/01 11:25p Hirose
    removed obsolete flags
    
    4     10/25/00 9:44p Hirose
    fixed a flag. MAC -> MACOS
    
    3     3/23/00 5:28p Hirose
    updated to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    8     2/25/00 3:40a Hirose
    updated pad control functions to match actual HW1 gamepad
    
    7     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    6     2/02/00 6:13p Alligator
    fix warnings, finish ArtX integration, update regression script
    
    5     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    4     1/19/00 7:05p Alligator
    fix EPPC errors
    
    3     1/18/00 7:07p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls
    added TevModeTable and enabled GX_BLEND for non MAC
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    15    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    14    10/07/99 3:55p Hirose
    changed camera portion / fixed some comments
    
    13    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    12    9/30/99 9:41p Hirose
    turned to use test texture (for GX_DECAL mode check)
    
    11    9/30/99 3:22p Hirose
    added single frame test
    
    10    9/20/99 11:46a Ryan
    update to use new DEMO calls
    
    9     9/13/99 4:21p Hirose
    confirmed alignment/coherency
    avoided all warnings
    
    8     9/02/99 5:35p Hirose
    
    7     8/31/99 4:58p Hirose
    turned all math functions into f32 version
    eliminated all tab characters
    
    6     8/28/99 9:41p Hirose
    fixed some comments
    
    5     8/28/99 12:08a Yasu
    Change enum name GX_MAT_3x4 -> GX_MTX3x4
    
    4     8/23/99 6:48p Hirose
    
    3     8/18/99 10:52p Hirose
    
    2     8/06/99 11:30p Hirose
    
    1     8/06/99 3:21p Hirose
    Created (copied lit-test06.c)
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-texture
     Texture with lighting test
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define  PI    3.14159265358979323846F

#ifdef   MACOS  // MAC emulator doesn't support GX_BLEND mode
#define  NUM_TEVMODES    4
#else
#define  NUM_TEVMODES    5
#endif

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
    GXLightObj  lobj;
    s32         theta;
    s32         phi;
    f32         colorCtrl[3][3];
} MyLightEnvObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    MyLightEnvObj  lightEnv;
    Mtx            modelCtrl;
    u32            modelType;
    GXTexObj       texture;
    u32            tevMode;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void DrawModel       ( u32 model );
static void DrawLightMark   ( void );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MyLightEnvObj * le, Mtx view );
static void DisableLight    ( void );
static void PrintIntro      ( void );
static void PrintParam      ( u32 cur, f32 param );

/*---------------------------------------------------------------------------*
   Model data and strings for message
 *---------------------------------------------------------------------------*/
#define SPHERE      0
#define CYLINDER    1
#define TORUS       2
#define ICOSA       3
#define DODECA      4
#define OCTA        5
#define CUBE        6

#define MODELS      7

static GXTevMode TevModeTable[] =
{
    GX_MODULATE,
    GX_DECAL,
    GX_PASSCLR,
    GX_REPLACE,
    GX_BLEND     // not supported on the MAC emulator
};

static char* TevModeStr[] =
{
    "GX_MODULATE",
    "GX_DECAL",
    "GX_PASSCLR",
    "GX_REPLACE",
    "GX_BLEND"
};

static char* ParamTitle[] =
{
    "Ambient",
    "Material",
    "Light"
};

/*---------------------------------------------------------------------------*
   Test texture data
 *---------------------------------------------------------------------------*/
#define TEST_TEXTURE_WIDTH   8
#define TEST_TEXTURE_HEIGHT  8
#define TEST_TEXTURE_FORMAT  GX_TF_RGB5A3

static u16 TestTextureData[] ATTRIBUTE_ALIGN(32) =
{
    // RGB5A3 format 8x8
    0x7FFF, 0x7F00, 0x7F88, 0x70FF,
    0x78FF, 0x700F, 0x78F8, 0x7888,
    0x7CCC, 0x7FF8, 0x7F8F, 0x7FF0,
    0x7F0F, 0x788F, 0x70F0, 0x7000,

    0x1FFF, 0x1F00, 0x1F88, 0x10FF,
    0x18FF, 0x100F, 0x18F8, 0x1888,
    0x1CCC, 0x1FF8, 0x1F8F, 0x1FF0,
    0x1F0F, 0x188F, 0x10F0, 0x1000,

    0x0FFF, 0x0F00, 0x0F88, 0x00FF,
    0x08FF, 0x000F, 0x08F8, 0x0888,
    0x0CCC, 0x0FF8, 0x0F8F, 0x0FF0,
    0x0F0F, 0x088F, 0x00F0, 0x0000,

    0x6FFF, 0x6F00, 0x6F88, 0x60FF,
    0x68FF, 0x600F, 0x68F8, 0x6888,
    0x6CCC, 0x6FF8, 0x6F8F, 0x6FF0,
    0x6F0F, 0x688F, 0x60F0, 0x6000,
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
static MySceneCtrlObj  SceneCtrl;    // scene control parameters

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);      // Init the OS, game pad, graphics and video.
    
    DrawInit(&SceneCtrl);       // Initialize vertex formats and scene parameters.

    PrintIntro();    // Print demo directions
         
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
    u32 i;
    Mtx mv,mr;

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // set up texture coord generation matrix
    // because GXDraw Objects have no texture coords.
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    MTXLightFrustum(mv, -1.0F, 1.0F, -1.0F, 1.0F, 8.0F, 1.0F, 1.0F, 0.5F, 0.5F);
    MTXTrans(mr, 0.0F, 0.0F, -5.0F);
    MTXConcat(mv, mr, mv);
    GXLoadTexMtxImm(mv, GX_TEXMTX0, GX_MTX3x4);

    GXInvalidateTexAll();


    // Default scene parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // texture
    GXInitTexObj(
        &sc->texture,
        TestTextureData,
        TEST_TEXTURE_WIDTH,
        TEST_TEXTURE_HEIGHT,
        TEST_TEXTURE_FORMAT,
        GX_REPEAT, // s
        GX_REPEAT, // t
        GX_FALSE );
    GXInitTexObjLOD(
        &sc->texture,
        GX_NEAR,
        GX_NEAR,
        0,
        0,
        0,
        GX_FALSE,
        GX_FALSE,
        GX_ANISO_1 );

    // light parameters
    sc->lightEnv.theta  = 0;
    sc->lightEnv.phi    = 0;
    
    // color of Ambient/Material/Light
    for ( i = 0 ; i < 3 ; ++i )
    {
        sc->lightEnv.colorCtrl[0][i] = 0.1F;
        sc->lightEnv.colorCtrl[1][i] = 1.0F;
        sc->lightEnv.colorCtrl[2][i] = 0.9F;
    }

    // initialize model control matrix
    MTXScale(sc->modelCtrl, 250.0F, 250.0F, 250.0F);
    sc->modelType = 0;

    // tev mode
    sc->tevMode = 0;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mv;  // Modelview matrix.
    Mtx  mr;  // Rotate matrix
    Mtx  mvi; // Modelview matrix.

    // Tev mode = one color / one texture
    GXSetNumTexGens(1); // use one texture coord
    GXSetTevOp(GX_TEVSTAGE0, TevModeTable[sc->tevMode]);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    // texture and lights
    GXLoadTexObj(&sc->texture, GX_TEXMAP0);
    SetLight(&sc->lightEnv, sc->cam.view);

    // Draw a model
    MTXConcat(sc->cam.view, sc->modelCtrl, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi);
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    DrawModel(sc->modelType);


    // Tev mode = one color / no texture
    GXSetNumTexGens(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    // disable lighting
    DisableLight();

    // draw a light mark
    MTXRotDeg(mr, 'y', sc->lightEnv.theta);
    MTXConcat(sc->cam.view, mr, mv);
    MTXRotDeg(mr, 'x', - sc->lightEnv.phi);
    MTXConcat(mv, mr, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    DrawLightMark();
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    static u32  cursor = 0;
    u16  down;
    Mtx  mrx, mry;

    // PAD
    down = DEMOPadGetButtonDown(0);

    // Light Position Calculation
    sc->lightEnv.theta += (DEMOPadGetStickX(0) / 24);
    sc->lightEnv.theta %= 360;
    sc->lightEnv.phi += (DEMOPadGetStickY(0) / 24);
    Clamp(sc->lightEnv.phi, -90, 90);
    

    // Model Rotation Calculation
    MTXRotDeg(mry, 'x', -(DEMOPadGetSubStickY(0) / 24));
    MTXRotDeg(mrx, 'y', (DEMOPadGetSubStickX(0) / 24));
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);


    // Select Ambient/Material/Light (also R/G/B)
    if ( down & PAD_BUTTON_Y )
    {
        cursor = ( cursor + 1 ) % 9;
        PrintParam(cursor, sc->lightEnv.colorCtrl[cursor/3][cursor%3]);
    }
    if ( down & PAD_BUTTON_X )
    {
        cursor = ( cursor + 8 ) % 9;
        PrintParam(cursor, sc->lightEnv.colorCtrl[cursor/3][cursor%3]);
    }
    // Color Parameter Tunung
    if ( down & PAD_TRIGGER_L )
    {
        sc->lightEnv.colorCtrl[cursor/3][cursor%3] -= 0.05F;
        if ( sc->lightEnv.colorCtrl[cursor/3][cursor%3] < 0.0F )
        {
            sc->lightEnv.colorCtrl[cursor/3][cursor%3] = 0.0F;
        }
        PrintParam(cursor, sc->lightEnv.colorCtrl[cursor/3][cursor%3]);
    }
    if ( down & PAD_TRIGGER_R )
    {
        sc->lightEnv.colorCtrl[cursor/3][cursor%3] += 0.05F;
        if ( sc->lightEnv.colorCtrl[cursor/3][cursor%3] > 1.0F )
        {
            sc->lightEnv.colorCtrl[cursor/3][cursor%3] = 1.0F;
        }
        PrintParam(cursor, sc->lightEnv.colorCtrl[cursor/3][cursor%3]);
    }

    // Change tev mode
    if ( down & PAD_BUTTON_B )
    {
        sc->tevMode = ( sc->tevMode + 1 ) % NUM_TEVMODES;
        OSReport("TevMode : %s\n", TevModeStr[sc->tevMode]);
    }

    // Model Select
    if ( down & PAD_BUTTON_A )
    {
        sc->modelType = ( sc->modelType + 1 ) % MODELS;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawModel
    
    Description:    Draws specified model
                    
    Arguments:      model : specifies which model is to be displayed
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModel(u32 model)
{
    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM,  GX_DIRECT);
    
    // draws a GXDraw model
    switch(model)
    {
        case CYLINDER :
            GXDrawCylinder(32);
            break;
        case TORUS :
            GXDrawTorus(0.25F, 12, 16);
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
        GXPosition3f32(0.0F, 0.0F, 500.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(500.0F, -500.0F, -500.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 500.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(-500.0F, 500.0F, -500.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 500.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(-500.0F, -500.0F, -500.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 500.0F);
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
                    
    Arguments:      le   : pointer to a MyLightEnvObj structure
                    view : view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetLight( MyLightEnvObj* le, Mtx view )
{
    GXColor colorBuf[3];
    Vec lpos;
    f32 theta, phi;
    u32 i;

    // Light Position
    theta = (f32)le->theta * PI / 180.0F;
    phi   = (f32)le->phi   * PI / 180.0F;
    lpos.x = 500.0F * cosf(phi) * sinf(theta);
    lpos.y = 500.0F * sinf(phi);
    lpos.z = 500.0F * cosf(phi) * cosf(theta);

    // Convert light position into view space
    MTXMultVec(view, &lpos, &lpos);

    // Light/Material Color
    for ( i = 0 ; i < 3 ; ++i )
    {
        colorBuf[i].r = (u8)(le->colorCtrl[i][0] * 255.0F);
        colorBuf[i].g = (u8)(le->colorCtrl[i][1] * 255.0F);
        colorBuf[i].b = (u8)(le->colorCtrl[i][2] * 255.0F);
        colorBuf[i].a = 255;
    }

    GXInitLightPos(&le->lobj, lpos.x, lpos.y, lpos.z);
    GXInitLightColor(&le->lobj, colorBuf[2]);
    GXLoadLightObjImm(&le->lobj, GX_LIGHT0);

    // Lighting channel
    GXSetNumChans(1); // number of active color channels
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);
    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0A0, colorBuf[0]);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, colorBuf[1]);

}

/*---------------------------------------------------------------------------*
    Name:           DisableLight
    
    Description:    Disables lighting
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DisableLight( void )
{
    GXSetNumChans(1);
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
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("lit-texture: texture with lighting test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick   : Move Light Position\n");
    OSReport("Sub  Stick   : Rotate the Moddel\n");
    OSReport("X/Y Buttons  : Select control parameter\n");
    OSReport("L/R Triggers : Change parameter value\n");
    OSReport("A Button     : Change Model\n");
    OSReport("B Button     : Change tev mode\n");
    OSReport("************************************************\n\n");
}

/*---------------------------------------------------------------------------*
    Name:           PrintParam
    
    Description:    Prints specified parameter.
 *---------------------------------------------------------------------------*/
static void PrintParam( u32 cur, f32 param )
{
    char rgb_chars[] = {'R','G','B'};

    OSReport("%s", ParamTitle[cur/3]);
    OSReport("[%c]", rgb_chars[cur%3]);
    OSReport(" = %f\n", param);
}

/*============================================================================*/
