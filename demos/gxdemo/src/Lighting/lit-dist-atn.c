/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-dist-atn.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-dist-atn.c $
    
    4     5/12/01 11:25p Hirose
    removed obsolete flags
    
    3     3/23/00 4:56p Hirose
    updated to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    6     2/25/00 12:51a Hirose
    updated pad control functions to match actual HW1 gamepad
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     1/25/00 2:54p Carl
    Changed to standardized end of test message
    
    3     1/18/00 4:10p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    15    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    14    11/12/99 4:29p Yasu
    Add GXSetNumTexGens(0)  for GX_PASSCLR
    
    13    11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    12    10/18/99 5:28p Hirose
    changed the value for light mask
    
    11    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    10    9/30/99 7:47p Hirose
    avoided to use array data / changed camera portion
    
    9     9/30/99 12:06a Hirose
    added single frame test
    
    8     9/20/99 11:46a Ryan
    update to use new DEMO calls
    
    7     9/13/99 11:23p Hirose
    confirmed alignment/coherency
    removed all warnings
    
    6     9/02/99 4:57p Hirose
    fixed camera setting
    
    5     8/31/99 4:57p Hirose
    turned all math functions into f32 version
    eliminated all tab characters
    
    4     8/28/99 12:28a Hirose
    added D-cache coherency function
    
    3     8/19/99 3:12p Hirose
    fixed the part which rises up compiler bug
    
    2     8/18/99 1:49a Hirose
    
    1     7/23/99 5:09p Hirose
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-dist-atn
     Distance attenuation test
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

/*---------------------------------------------------------------------------*
  Structure definitions
 *---------------------------------------------------------------------------*/
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
    s32           zoom;
} MyCameraObj;

typedef struct
{
    f32    k0;
    f32    k1;
    f32    k2;
} DistAttn;

typedef struct
{
    GXLightObj    lobj;
    u32           attnType;
} MyLightObj;

typedef struct
{
    MyCameraObj   cam;
    MyLightObj    light;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void DrawTessPanel   ( void );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MyLightObj* light, Mtx v );
static void DisableLight    ( void );
static void PrintIntro      ( void );
static void StatusMessage   ( u32 index );

/*---------------------------------------------------------------------------*
   Lighting parameters
 *---------------------------------------------------------------------------*/
#define BLACK   MyColors[2]
#define WHITE   MyColors[3]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {0x00, 0x00, 0x40, 0xff},  // blue
    {0x40, 0x00, 0x00, 0xff},  // red
    {0x00, 0x00, 0x00, 0xff},  // black
    {0xff, 0xff, 0xff, 0xff},  // white
};

// fixed normal vector
static f32 FixedNormal[] ATTRIBUTE_ALIGN(32) =
{
    0.0F, 0.0F, 1.0F
};

#define NUM_OF_ATTNS    14
static DistAttn LightAttnSamples[NUM_OF_ATTNS] =
{
    { 1.0F,  0.1F,   0.0F   },
    { 1.0F,  0.02F,  0.0F   },
    { 1.0F,  0.01F,  0.0F   },
    { 1.0F,  0.005F, 0.0F   },
    { 1.0F,  0.002F, 0.0F   },
    { 1.0F,  0.001F, 0.0F   },
    { 1.0F,  0.0F,   0.0F   },
    { 2.0F,  0.0F,   0.0F   },
    { 2.0F,  0.001F, 0.0F   },
    { 1.0F,  0.0F,   0.0001F },
    { 1.0F, -0.01F,  0.0001F },
    { 1.0F, -0.05F,  0.0005F },
    { 1.0F, -0.005F, 0.000025F },
    { 4.0F, -0.002F, 0.0F  }
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 0.1F }, // location
    { 0.0F, 1.0F, 0.0F }, // up
    { 0.0F, 0.0F, 0.0F }, // target
    -320.0F,  // left
    240.0F,   // top   
    400.0F,   // near
    1800.0F   // far
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
    DEMOInit(NULL);    // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl); // Initialize vertex formats and scene parameters etc.

    PrintIntro();  // Print demo directions
         
    StatusMessage(SceneCtrl.light.attnType);
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
    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);

    // culling off
    GXSetCullMode(GX_CULL_NONE);


    // Default scene parameter settings

    // camera
    sc->cam.cfg   = DefaultCamera;
    sc->cam.theta = 45;
    sc->cam.zoom  = 1000;

    // light parameters
    sc->light.attnType = 0;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  ms;  // Scale matrix.
    Mtx  mt;  // Translation matrix
    Mtx  mv;  // Modelview matrix.
    Mtx  mvi; // Modelview matrix.
    s32  iz;

    // render mode = one color / no texture
    GXSetNumTexGens(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    
    // Set camera
    SetCamera(&sc->cam);

    // Set light
    SetLight(&sc->light, sc->cam.view);

    // Draw panels
    for ( iz = 0 ; iz < 10 ; ++iz )
    {
        f32 width = 10.0F - iz * 0.6F;
        MTXScale(ms, width * 2.0F, width * 2.0F, 1.0F);
        MTXTrans(mt, -7.5F, -7.5F, iz * 100.0F - 500.0F );
        MTXConcat(sc->cam.view, ms, mv);
        MTXConcat(mv, mt, mv);

        GXLoadPosMtxImm(mv, GX_PNMTX0);
        MTXInverse(mv, mvi); 
        MTXTranspose(mvi, mv); 
        GXLoadNrmMtxImm(mv, GX_PNMTX0);

        GXSetChanAmbColor(GX_COLOR0A0, MyColors[iz%2]);
        DrawTessPanel();
    }

    // Disable light
    DisableLight();
    // use ambient color for drawing
    GXSetChanAmbColor(GX_COLOR0A0, WHITE);

    // Draw ball light object
    MTXTrans(mt, 0.0F, 0.0F, 500.0F);
    MTXConcat(sc->cam.view, mt, mv);
    MTXScale(ms, 20.0F, 20.0F, 20.0F);
    MTXConcat(mv, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    GXSetChanMatColor(GX_COLOR0A0, WHITE);
    GXDrawSphere(12, 24);

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

    // Camera Position Calculation
    sc->cam.theta -= ( DEMOPadGetStickX(0) / 24 );
    if ( sc->cam.theta < 0 )
        sc->cam.theta = 0;
    if ( sc->cam.theta > 180 )
        sc->cam.theta = 180;

    sc->cam.zoom -= DEMOPadGetStickY(0);
    if ( sc->cam.zoom < 600 )
        sc->cam.zoom = 600;
    if ( sc->cam.zoom > 1200 )
        sc->cam.zoom = 1200;

    // Light Attenuation change
    if ( down & PAD_TRIGGER_R )
    {
        sc->light.attnType = ( sc->light.attnType + 1 ) % NUM_OF_ATTNS;
        StatusMessage(sc->light.attnType);
    }
    if ( down & PAD_TRIGGER_L )
    {
        sc->light.attnType = ( sc->light.attnType + NUM_OF_ATTNS - 1 ) % NUM_OF_ATTNS;
        StatusMessage(sc->light.attnType);
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawTessPanel
    
    Description:    Draws 16x16 vertices panel
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTessPanel( void )
{
    s8 x, y;

    // set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);

    // normal array (actualy contains only one item)
    GXSetArray(GX_VA_NRM, FixedNormal, 3 * sizeof(f32));
    
    for ( y = 0 ; y < 15 ; ++y )
    {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 32);
            for ( x = 0 ; x < 16 ; ++x )
            {
                GXPosition3s8(x, y, 0);
                GXNormal1x8(0);
                GXPosition3s8(x, (s8)(y+1), 0);
                GXNormal1x8(0);
            }
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    set view matrix and load projection matrix into hardware
                    
    Arguments:      cam : pointer to the MyCameraObj structure
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetCamera( MyCameraObj* cam )
{
    cam->cfg.location.x =
        (f32)cam->zoom * cosf((f32)cam->theta * PI / 180.0F);
    cam->cfg.location.y =
        (f32)cam->zoom * 0.2F;
    cam->cfg.location.z =
        (f32)cam->zoom * sinf((f32)cam->theta * PI / 180.0F);
    
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
    Vec lpos = { 0.0F, 0.0F, 500.0F };
    
    DistAttn attn = LightAttnSamples[light->attnType];
        
    // Multipled by view matrix
    MTXMultVec(v, &lpos, &lpos);

    GXInitLightPos(&light->lobj, lpos.x, lpos.y, lpos.z);
    GXInitLightAttn(
        &light->lobj,
        1.0F,
        0.0F,
        0.0F,
        attn.k0,
        attn.k1,
        attn.k2 );
    GXInitLightColor(&light->lobj, WHITE);
    GXLoadLightObjImm(&light->lobj, GX_LIGHT0);

    GXSetNumChans(1); // number of active color channels
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_SPOT);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, WHITE);
}

/*---------------------------------------------------------------------------*
    Name:           DisableLight
    
    Description:    Disables lighting
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void DisableLight( void )
{
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,  // disable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        0,           // light mask
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
    OSReport("lit-dist-atn: distance attenuation test\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Stick        : Move camera position\n");
    OSReport("L/R Triggers : Change the distance attenuation value set\n");
    OSReport("******************************************************\n");
    OSReport("\n\n");
}

/*---------------------------------------------------------------------------*
    Name:           StatusMessage
    
    Description:    Prints the current status.
 *---------------------------------------------------------------------------*/
static void StatusMessage( u32 index )
{
    OSReport(" k0 = %f ", LightAttnSamples[index].k0);
    OSReport(" k1 = %f ", LightAttnSamples[index].k1);
    OSReport(" k2 = %f ", LightAttnSamples[index].k2);

    OSReport("\n");
}

/*============================================================================*/
