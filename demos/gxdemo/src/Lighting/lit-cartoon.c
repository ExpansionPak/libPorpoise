/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-cartoon.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-cartoon.c $
    
    7     5/12/01 11:25p Hirose
    removed obsolete flags
    
    6     5/31/00 12:13p Alligator
    fixed warning
    
    5     5/27/00 1:24p Alligator
    added button so all texture coordinates can be used
    
    4     3/23/00 4:56p Hirose
    updated to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    6     2/25/00 3:40a Hirose
    updated pad control functions to match actual HW1 gamepad
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    3     1/18/00 4:10p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    19    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    18    11/15/99 4:49p Yasu
    Change datafile name
    
    17    10/07/99 3:54p Hirose
    changed camera portion / fixed some comments
    
    16    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    15    9/30/99 4:29p Hirose
    
    14    9/29/99 7:22p Hirose
    added single frame test
    
    13    9/20/99 11:46a Ryan
    update to use new DEMO calls
    
    12    9/13/99 4:43p Hirose
    confirmed alignment/coherency
    removed all warnings
    
    11    9/02/99 4:57p Hirose
    fixed camera setting
    
    10    9/01/99 5:45p Ryan
    
    9     8/31/99 4:58p Hirose
    turned all math functions into f32 version
    eliminated all tab characters
    
    8     8/28/99 12:33a Yasu
    Append explicit casting to GXTexFmt
    
    7     8/27/99 11:32p Hirose
    cleaned up
    
    6     8/23/99 5:29p Hirose
    
    5     8/13/99 4:45p Hirose
    changed TPL file location
    
    4     8/13/99 12:14a Hirose
    
    3     8/05/99 9:31p Hirose
    
    2     8/05/99 7:27p Hirose
    
    1     8/04/99 2:19p Hirose
    Created (Copied lit-test05.c)
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-cartoon
     Unclamped diffuse function with texture coord generation
     (Cartoon rendering)
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI    3.14159265358979323846F

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
    f32         brightCtrl[3];
    u32         cartoonType;
} MyLightEnvObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    MyLightEnvObj  lightEnv;
    Mtx            modelCtrl;
    u32            modelType;
    GXTexObj       texture;
    GXTevMode      tevMode;
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
   Model data and strings for message
 *---------------------------------------------------------------------------*/
#define TORUS       0
#define SPHERE      1
#define CUBE        2
#define ICOSA       3

#define MODELS      4

static char* ParamTitle[] =
{
    "Ambient",
    "Material",
    "Light"
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
static MySceneCtrlObj  SceneCtrl;       // scene control parameters
static TEXPalettePtr   MyTplObj = 0;    // texture palette file
static GXTexCoordID    CoordCtrl = GX_TEXCOORD0; // select one of 7 tex coords

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
    TEXDescriptorPtr tdp;

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // texture from Tpl file
    TEXGetPalette(&MyTplObj, "gxTests/lit-06.tpl");
    tdp = TEXGet(MyTplObj, 0);
    GXInitTexObj(
        &sc->texture,
        tdp->textureHeader->data,
        tdp->textureHeader->width,
        tdp->textureHeader->height,
        (GXTexFmt)tdp->textureHeader->format,
        GX_CLAMP,   // s
        GX_CLAMP,   // t
        GX_FALSE ); // Mipmap
    GXInitTexObjLOD(
        &sc->texture,
        tdp->textureHeader->minFilter,
        tdp->textureHeader->magFilter,
        tdp->textureHeader->minLOD,
        tdp->textureHeader->maxLOD,
        tdp->textureHeader->LODBias,
		GX_FALSE,
        tdp->textureHeader->edgeLODEnable, 
		GX_ANISO_1);
    GXInvalidateTexAll();


    // Default scene parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // light parameters
    sc->lightEnv.theta  = 0;
    sc->lightEnv.phi    = 0;

    // brightness of Ambient/Material/Light
    sc->lightEnv.brightCtrl[0] = 0.5F;
    sc->lightEnv.brightCtrl[1] = 1.0F;
    sc->lightEnv.brightCtrl[2] = 0.5F;

    // type of cartoon lighting (1D texture lookup index)
    sc->lightEnv.cartoonType = 0;

    // model control matrix
    MTXScale(sc->modelCtrl, 250.0F, 250.0F, 250.0F);
    sc->modelType = 0;

    // tev mode
    sc->tevMode = GX_REPLACE;
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
    Mtx  mr;  // Rotate matrix.
    Mtx  mvi; // Modelview matrix.
    u32  i;

    // enable lighting
    SetLight(&sc->lightEnv, sc->cam.view);

    // set texture
    GXLoadTexObj(&sc->texture, GX_TEXMAP0);
    
    // texture coord generation mode for cartoon lighting
    for (i = 0; i < CoordCtrl; i++)
        GXSetTexCoordGen((GXTexCoordID)i, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY);

    GXSetTexCoordGen(CoordCtrl, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY);

    // set Tev mode = one color / one texture
    GXSetTevOp(GX_TEVSTAGE0, sc->tevMode);
    GXSetTevOrder(GX_TEVSTAGE0, CoordCtrl, GX_TEXMAP0, GX_COLOR0A0);
    GXSetNumTexGens((u8)(CoordCtrl+1));
    
    // draw a model
    MTXConcat(sc->cam.view, sc->modelCtrl, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    DrawModel(sc->modelType);
    
    
    // disable lights
    DisableLight();
    
    // set Tev mode = one color / no texture
    GXSetNumTexGens(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

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
    u32  pr = 0;

    // PAD
    down = DEMOPadGetButtonDown(0);

    // Light Position Calculation
    sc->lightEnv.theta += (DEMOPadGetStickX(0) / 32);
    sc->lightEnv.theta %= 360;
    sc->lightEnv.phi += (DEMOPadGetStickY(0) / 32);
    Clamp(sc->lightEnv.phi, -90, 90);
    

    // Model Rotation Calculation
    MTXRotDeg(mry, 'x', -(DEMOPadGetSubStickY(0) / 32));
    MTXRotDeg(mrx, 'y', (DEMOPadGetSubStickX(0) / 32));
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);


    // Select Ambient/Material/Light
    if ( down & PAD_BUTTON_Y )
    {
        cursor = ( cursor + 1 ) % 3;
        pr = 1;
    }
    if ( down & PAD_BUTTON_X )
    {
        CoordCtrl = (GXTexCoordID) (CoordCtrl + 1);
        if (CoordCtrl == GX_MAX_TEXCOORD)
            CoordCtrl = GX_TEXCOORD0;
        OSReport("Tex Coord %d being tested\n", CoordCtrl);
    }
    // Ambient/Material/Light brightness tuning
    if ( down & PAD_TRIGGER_L )
    {
        sc->lightEnv.brightCtrl[cursor] -= 0.05F;
        if ( sc->lightEnv.brightCtrl[cursor] < 0.0F )
        {
            sc->lightEnv.brightCtrl[cursor] = 0.0F;
        }
        pr = 1;
    }
    if ( down & PAD_TRIGGER_R )
    {
        sc->lightEnv.brightCtrl[cursor] += 0.05F;
        if ( sc->lightEnv.brightCtrl[cursor] > 1.0F )
        {
            sc->lightEnv.brightCtrl[cursor] = 1.0F;
        }
        pr = 1;
    }

    // print selected/changed parameter
    if ( pr )
    {
        OSReport(
            "%s = %f\n",
            ParamTitle[cursor],
            sc->lightEnv.brightCtrl[cursor] );
    }

    // change cartoon texture type
    if ( down & PAD_BUTTON_B )
    {
        sc->lightEnv.cartoonType = ( sc->lightEnv.cartoonType + 1 ) % 8;
    }

    // model select
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
static void DrawModel( u32 model )
{
    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM,  GX_DIRECT);
    
    // draws a GXDraw model
    switch(model)
    {
        case TORUS :
            GXDrawTorus(0.25F, 16, 24);
            break;
        case SPHERE :
            GXDrawSphere(16, 24);
            break;
        case CUBE :
            GXDrawCube();
            break;
        case ICOSA :
            GXDrawIcosahedron();
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
                    
    Arguments:      le    : pointer to a MyLightEnvObj structure
                    view  : view matrix
    
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

    // Ambient/Material/Light color (red value only)
    for ( i = 0 ; i < 3 ; ++i )
    {
        colorBuf[i].r = (u8)(le->brightCtrl[i] * 255.0F);
        colorBuf[i].b = 0;
        colorBuf[i].g = 0;
        colorBuf[i].a = 255;
    }
    // The green component of ambient color is used to lookup
    // 1D cartoon texture from 2D texture map.
    colorBuf[0].g = (u8)(le->cartoonType * 0x20 + 0x10);
    colorBuf[1].g = 255;

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
        GX_DF_SIGN,  // diffuse function
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
    OSReport("**********************************************\n");
    OSReport("lit-cartoon: unclamped diffuse function with\n");
    OSReport("            texture lookup (cartoon rendering)\n");
    OSReport("**********************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick   : Move Light Position\n");
    OSReport("Sub  Stick   : Rotate the Model\n");
    OSReport("Y Button     : Select control parameter\n");
    OSReport("X Buttons    : Select texture coordinate\n");
    OSReport("L/R Triggers : Change parameter value\n");
    OSReport("A Button     : Change Model\n");
    OSReport("B Button     : Change 1D texture\n");
    OSReport("**********************************************\n");
}

/*============================================================================*/
