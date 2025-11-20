/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-specular.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-specular.c $
    
    9     5/12/01 11:25p Hirose
    removed obsolete flags
    
    8     10/18/00 10:56a Hirose
    deleted unnecessary test portion
    
    7     8/27/00 5:45p Alligator
    print warning messages through a callback function
    
    6     8/21/00 2:37p Hirose
    fixed wrong TEV setting
    deleted attanuation select because there was no meaning
    
    5     5/12/00 6:09p Hirose
    fix due to GXInitLight*v macro definition change
    
    4     3/23/00 5:28p Hirose
    updated to use DEMOPad library
    
    3     3/06/00 7:38p Alligator
    ifdef  _DEBUG arounf GXSetVerifyLevel
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    8     3/03/00 10:31p Hirose
    deleted unnecessary stuff
    
    7     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    6     2/25/00 3:59a Hirose
    updated pad control function to match actual HW1 gamepad
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     2/15/00 3:04p Carl
    Added GXSetNumTexGens(0) for EPPC code
    
    3     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/13/99 4:54p Hirose
    moved GXInitLightShininess macro to public header file
    
    6     11/17/99 4:33p Hirose
    changed to use GXInitLightPosv/Dirv macros
    
    5     11/17/99 4:03p Hirose
    fixed misspelling
    
    4     11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    3     11/12/99 4:29p Yasu
    Add GXSetNumTexGens(0)  for GX_PASSCLR
    
    2     11/12/99 1:44p Hirose
    added single frame test and some comments
    
    1     11/11/99 11:55p Hirose
    Initial version
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-specular
     specular lighting test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI     3.14159265358979323846F

#define NUM_RENDER_MODES  4

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
    GXLightObj     lobj;
    s32            theta;
    s32            phi;
    f32            shininess;
} MyLightEnvObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    MyLightEnvObj  lightEnv;
    Mtx            modelCtrl;
    u32            modelType;
    u32            renderMode;
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
static void SetLight        ( MyLightEnvObj* le, Mtx view );
static void DisableLight    ( void );
static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
   Model and color data
 *---------------------------------------------------------------------------*/
#define SPHERE      0
#define CYLINDER    1
#define TORUS       2
#define ICOSA       3
#define DODECA      4

#define MODELS      5


#define REG_AMBIENT0   MyColors[0]
#define REG_MATERIAL0  MyColors[1]
#define REG_AMBIENT1   MyColors[2]
#define REG_MATERIAL1  MyColors[3]
#define LIGHT_COLOR0   MyColors[4]

static GXColor MyColors[] =
{
    { 0x40, 0x40, 0x40, 0x40 }, // Ambient  for channel 0
    { 0x50, 0x20, 0x10, 0xFF }, // Material for channel 0
    { 0x00, 0x00, 0x00, 0x00 }, // Ambient  for channel 1
    { 0xE0, 0xE0, 0xE0, 0xE0 }, // Material for channel 1
    { 0xFF, 0xFF, 0xFF, 0xFF }  // Light color
};

/*---------------------------------------------------------------------------*
   Strings for messages
 *---------------------------------------------------------------------------*/
static char* RenModeStr[] =
{
    "Specular channel only",
    "Diffuse channel only",
    "Specular/Diffuse added",
    "Specular/Diffuse blended"
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
    400.0F,  // znear
    2000.0F  // zfar
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
    DEMOInit(NULL);
    
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
    
    Description:    Initializes the vertex attribute format and
                    sets up default scene parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{ 
    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetNumTexGens(0);

    // Z compare mode
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);


    // Default scene parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // light parameters
    sc->lightEnv.theta      = 0;
    sc->lightEnv.phi        = 0;
    sc->lightEnv.shininess  = 16.0F;
    
    // model control matrix
    MTXScale(sc->modelCtrl, 300.0F, 300.0F, 300.0F);
    sc->modelType = 0;

    // rendering mode
    sc->renderMode = 0;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    In the MAC emulator, we can't configure Tev to blend two color channels.
    So this test draws the object twice with using pixel blending.
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mv, mr, mvi;

    // enable lighting
    SetLight(&sc->lightEnv, sc->cam.view);


#ifdef EMU  // ---------- for emulator ----------

    // First pass (diffuse)

    // set tev mode
    GXSetNumTexGens( 0 );       // # of Tex Gens
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // set blend mode
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);

    // draw the model
    MTXConcat(sc->cam.view, sc->modelCtrl, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi);
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    
    DrawModel(sc->modelType);


    // Second pass (specular)
    
    // set tev mode
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);

    // set blend mode
    switch ( sc->renderMode )
    {
      case 0 :
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
        break;
      case 1 :
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_SET);
        break;
      case 2 :
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);
        break;
      case 3 :
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_INVSRCCLR, GX_LO_SET);
        break;
    }

    // draw the model
    DrawModel(sc->modelType);

    // reset blend mode
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);

#else  // -------------- actual code for real hardware --------------

    // set tev mode
    switch ( sc->renderMode )
    {
      case 0:
        {
            // Output = COLOR1
            GXSetNumTevStages(1);
            GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
            GXSetTevOrder(
                GX_TEVSTAGE0,
                GX_TEXCOORD_NULL,
                GX_TEXMAP_NULL,
                GX_COLOR1A1 );
        } break;
      case 1:
        {
            // Output = COLOR0
            GXSetNumTevStages(1);
            GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
            GXSetTevOrder(
                GX_TEVSTAGE0,
                GX_TEXCOORD_NULL,
                GX_TEXMAP_NULL,
                GX_COLOR0A0 );
        } break;
      case 2:
        {
            // Output = COLOR0 + COLOR1
            GXSetNumTevStages(2);
            GXSetTevOrder(
                GX_TEVSTAGE0,
                GX_TEXCOORD_NULL,
                GX_TEXMAP_NULL,
                GX_COLOR0A0 );
            GXSetTevOrder(
                GX_TEVSTAGE1,
                GX_TEXCOORD_NULL,
                GX_TEXMAP_NULL,
                GX_COLOR1A1 );
            GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
            GXSetTevColorOp(
                GX_TEVSTAGE1,
                GX_TEV_ADD,    // operator
                GX_TB_ZERO,    // bias
                GX_CS_SCALE_1, // scale
                GX_ENABLE,     // clamp
                GX_TEVPREV );  // output
            GXSetTevColorIn(
                GX_TEVSTAGE1,
                GX_CC_ZERO,    // a
                GX_CC_RASC,    // b
                GX_CC_ONE,     // c
                GX_CC_CPREV ); // d
        } break;
      case 3:
        {
            // Output = ( 1 - COLOR1 ) * COLOR0 + COLOR1
            GXSetNumTevStages(2);
            GXSetTevOrder(
                GX_TEVSTAGE0,
                GX_TEXCOORD_NULL,
                GX_TEXMAP_NULL,
                GX_COLOR0A0 );
            GXSetTevOrder(
                GX_TEVSTAGE1,
                GX_TEXCOORD_NULL,
                GX_TEXMAP_NULL,
                GX_COLOR1A1 );
            GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
            GXSetTevColorOp(
                GX_TEVSTAGE1,
                GX_TEV_ADD,    // operator
                GX_TB_ZERO,    // bias
                GX_CS_SCALE_1, // scale
                GX_ENABLE,     // clamp
                GX_TEVPREV );  // output
            GXSetTevColorIn(
                GX_TEVSTAGE1,
                GX_CC_CPREV,   // a
                GX_CC_ONE,     // b
                GX_CC_RASC,    // c
                GX_CC_ZERO );  // d
        } break;
    }
      
    // draw the model
    MTXConcat(sc->cam.view, sc->modelCtrl, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi);
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    
    DrawModel(sc->modelType);

#endif    // --------------------------------------

    // Draw light direction mark
    DisableLight();
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

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
    u16  button, down;
    Mtx  mrx, mry;

    // PAD
    button = DEMOPadGetButton(0);
    down   = DEMOPadGetButtonDown(0);

    // Light Position Calculation
    sc->lightEnv.theta += ( DEMOPadGetStickX(0) / 32 );
    sc->lightEnv.theta = sc->lightEnv.theta % 360;
    sc->lightEnv.phi += ( DEMOPadGetStickY(0) / 32 );
    Clamp(sc->lightEnv.phi, -90, 90);


    // Model Rotation Calculation
    MTXRotDeg(mry, 'x', -(DEMOPadGetSubStickY(0) / 32));
    MTXRotDeg(mrx, 'y', (DEMOPadGetSubStickX(0) / 32));
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);


    // Shininess parameter control
    if ( button & PAD_TRIGGER_L )
    {
        sc->lightEnv.shininess -= 4.0F;
    }
    if ( button & PAD_TRIGGER_R )
    {
        sc->lightEnv.shininess += 4.0F;
    }
    Clamp(sc->lightEnv.shininess, 4.0F, 256.0F);

    // Render mode
    if ( down & PAD_BUTTON_Y )
    {
        sc->renderMode = ( sc->renderMode + 1 ) % NUM_RENDER_MODES;
        OSReport("%s\n", RenModeStr[sc->renderMode]);
    }

    // Model Select
    if ( down & PAD_BUTTON_B )
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
static void DrawModel( u32 model )
{
    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    
    // draws a GXDraw model
    switch(model)
    {
        case CYLINDER :
            GXDrawCylinder(64);
            break;
        case TORUS :
            GXDrawTorus(0.25F, 24, 32);
            break;
        case SPHERE :
            GXDrawSphere1(3);
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

    GXBegin(GX_LINES, GX_VTXFMT0, 10);
        GXPosition3f32(0.0F, 0.0F, 350.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 0.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(20.0F, 20.0F, 450.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 350.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(-20.0F, 20.0F, 450.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 350.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(-20.0F, -20.0F, 450.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 350.0F);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(20.0F, -20.0F, 450.0F);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, 350.0F);
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
    GXDiffuseFn  dfunc[2];
    GXAttnFn     afunc[2];
    Vec  ldir;
    f32  theta, phi;

    // attenuation setting
    dfunc[0] = GX_DF_CLAMP;
    afunc[0] = GX_AF_NONE;
    dfunc[1] = GX_DF_NONE;
    afunc[1] = GX_AF_SPEC;

    // Light position
    theta = (f32)le->theta * PI / 180.0F;
    phi   = (f32)le->phi   * PI / 180.0F;
    ldir.x = - 1.0F * cosf(phi) * sinf(theta);
    ldir.y = - 1.0F * sinf(phi);
    ldir.z = - 1.0F * cosf(phi) * cosf(theta);

    // Convert light position into view space
    MTXMultVecSR(view, &ldir, &ldir);

    // Set up light parameters
    
    // This API can calculate infinite light position
    // and half-angle vector internally.
    GXInitSpecularDirv(&le->lobj, &ldir);
    
    GXInitLightShininess(&le->lobj, le->shininess);
    GXInitLightColor(&le->lobj, LIGHT_COLOR0);
    GXLoadLightObjImm(&le->lobj, GX_LIGHT0);

    // Lighting channel
    GXSetNumChans(2);    // use two color channels
    GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,       // enable channel
        GX_SRC_REG,      // amb source
        GX_SRC_REG,      // mat source
        GX_LIGHT0,       // light mask
        dfunc[0],        // diffuse function
        afunc[0]);       // attenuation
    GXSetChanCtrl(
        GX_COLOR1,
        GX_ENABLE,       // enable channel
        GX_SRC_REG,      // amb source
        GX_SRC_REG,      // mat source
        GX_LIGHT0,       // light mask
        dfunc[1],        // diffuse function
        afunc[1]);       // attenuation
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_DISABLE,      // disable channel
        GX_SRC_REG,      // amb source
        GX_SRC_REG,      // mat source
        GX_LIGHT_NULL,   // light mask
        GX_DF_NONE,      // diffuse function
        GX_AF_NONE);
    GXSetChanCtrl(
        GX_ALPHA1,
        GX_DISABLE,      // disable channel
        GX_SRC_REG,      // amb source
        GX_SRC_REG,      // mat source
        GX_LIGHT_NULL,   // light mask
        GX_DF_NONE,      // diffuse function
        GX_AF_NONE);

    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0, REG_AMBIENT0);
    GXSetChanAmbColor(GX_COLOR1, REG_AMBIENT1);
    // set up material color
    GXSetChanMatColor(GX_COLOR0, REG_MATERIAL0);
    GXSetChanMatColor(GX_COLOR1, REG_MATERIAL1);

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
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("lit-specular: specular lighting test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick   : Move Light Direction\n");
    OSReport("Sub  Stick   : Rotate the model\n");
    OSReport("L/R Triggers : Change shininess parameter\n");
    OSReport("Y Button     : Change rendering mode\n");
    OSReport("B Button     : Change Model\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
