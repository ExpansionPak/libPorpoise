/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-atn-func.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-atn-func.c $
    
    6     5/12/01 11:25p Hirose
    removed obsolete flags
    
    5     6/15/00 2:48p Hirose
    fixed to match DEMOPad change
    
    4     3/23/00 4:56p Hirose
    updated to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    9     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    8     2/25/00 3:59a Hirose
    updated pad control function to match actual HW1 gamepad
    
    7     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    6     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    5     1/25/00 2:54p Carl
    Changed to standardized end of test message
    
    4     1/19/00 5:09p Hirose
    added new GXSpotFn enums
    
    3     1/18/00 4:32p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    13    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    12    11/15/99 4:49p Yasu
    Change datafile name
    
    11    10/18/99 5:28p Hirose
    changed default function type
    
    10    10/11/99 10:34a Hirose
    fixed number of vertices set to GXBegin in DrawModel1
    
    9     10/02/99 11:14p Hirose
    fixed bug on distance attenuation test
    
    8     9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    7     9/28/99 8:37p Hirose
    added single frame test
    
    6     9/28/99 8:15p Hirose
    deleted codes for expriment and cleaned up 
    
    5     9/20/99 11:46a Ryan
    update to use new DEMO calls
    
    4     9/13/99 4:36p Hirose
    
    3     9/13/99 4:33p Hirose
    changed to call real GX functions (non-experiment version)
    
    2     9/09/99 1:15p Hirose
    
    1     9/08/99 5:57p Hirose
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-atn-func
     Convienance attenuation function test
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

// for entire scene control
typedef struct
{
    MyCameraObj cam;
    Mtx         modelCtrl;
    GXLightObj  light;
    GXTexObj    texture;
    u32         funcType;
    s32         param0;
    s32         param1;
} MySubSceneCtrlObj;

typedef struct
{
    MySubSceneCtrlObj  sub[2];
    u32                mode;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void DrawModel0      ( void );
static void DrawModel1      ( void );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( GXLightObj* lobj );
static void PrintIntro      ( void );
static void StatusMessage   ( MySceneCtrlObj* sc );

/*---------------------------------------------------------------------------*
   Lighting and model parameters
 *---------------------------------------------------------------------------*/
#define LIGHT_COLOR     MyColors[0]
#define REG_MATERIAL    MyColors[1]
#define REG_AMBIENT     MyColors[2]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {0xff, 0xff, 0xff, 0xff},  // white
    {0xff, 0xff, 0xff, 0xff},  // material
    {0x00, 0x00, 0x00, 0x00}   // ambient
};

#define NUM_SPFUNC    7
static GXSpotFn SpFnTable[NUM_SPFUNC] =
{
    GX_SP_OFF,
    GX_SP_FLAT,
    GX_SP_COS,
    GX_SP_COS2,
    GX_SP_SHARP,
    GX_SP_RING1,
    GX_SP_RING2
};

#define NUM_DAFUNC    4
static GXDistAttnFn DaFnTable[NUM_DAFUNC] =
{
    GX_DA_OFF,
    GX_DA_GENTLE,
    GX_DA_MEDIUM,
    GX_DA_STEEP
};

/*---------------------------------------------------------------------------*
   Strings for messages
 *---------------------------------------------------------------------------*/
static char* SpFnStr[NUM_SPFUNC] =
{
    "GX_SP_OFF",
    "GX_SP_FLAT",
    "GX_SP_COS",
    "GX_SP_COS2",
    "GX_SP_SHARP",
    "GX_SP_RING1",
    "GX_SP_RING2"
};

static char* DaFnStr[NUM_DAFUNC] =
{
    "GX_DA_OFF",
    "GX_DA_GENTLE",
    "GX_DA_MEDIUM",
    "GX_DA_STEEP"
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera0 =
{
    { 0.0F, 0.0F, 500.0F }, // location
    { 0.0F, 1.0F, 0.0F },   // up
    { 0.0F, 0.0F, 0.0F },   // target
    -320.0F,  // left
    240.0F,   // top   
    400.0F,   // near
    2000.0F   // far
};

static CameraConfig DefaultCamera1 =
{
    { 0.0F, 0.0F, 750.0F }, // location
    { 0.0F, 1.0F, 0.0F },   // up
    { 0.0F, 0.0F, 0.0F },   // target
    -320.0F,  // left
    240.0F,   // top   
    400.0F,   // near
    2000.0F   // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;       // scene control parameters
static TEXPalettePtr   MyTplObj = 0;    // external texture palette

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main( void )
{
    DEMOInit(NULL);    // Init the OS, game pad, graphics and video.
    
    DrawInit(&SceneCtrl); // Initialize vertex formats and scene parameters.

    PrintIntro();  // Print demo directions
         
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);    // Draw the model.
        DEMODoneRender();   
        DEMOPadRead();           // Update controller status
        AnimTick(&SceneCtrl);    // Update animation.
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format and textures.
                    This function also sets up default scene parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{ 
    Mtx ms;
    Vec lpos;

    // set up a vertex attribute.
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);


    // set up default scene settings

    // camera
    sc->sub[0].cam.cfg = DefaultCamera0;
    SetCamera(&sc->sub[0].cam);
    sc->sub[1].cam.cfg = DefaultCamera1;
    SetCamera(&sc->sub[1].cam);
    
    // lighting function parameters
    sc->sub[0].funcType = 1;
    sc->sub[0].param0   = 30;  // cutoff
    sc->sub[1].funcType = 1;
    sc->sub[1].param0   = 500; // reference distance
    sc->sub[1].param1   = 50;  // reference brightness

    // fixed parameters in light objects
    lpos.x = lpos.y = lpos.z = 0.0F;
    MTXMultVec(sc->sub[0].cam.view, &lpos, &lpos);
    GXInitLightPos(&sc->sub[0].light, lpos.x, lpos.y, lpos.z);
    GXInitLightDir(&sc->sub[0].light, 0.0F, 0.0F, -1.0F);
    GXInitLightColor(&sc->sub[0].light, LIGHT_COLOR);
    GXInitLightDistAttn(&sc->sub[0].light, 0.0F, 0.0F, GX_DA_OFF );

    lpos.x = -500.0F;
    lpos.y = lpos.z = 0.0F;
    MTXMultVec(sc->sub[1].cam.view, &lpos, &lpos);
    GXInitLightPos(&sc->sub[1].light, lpos.x, lpos.y, lpos.z);
    GXInitLightDir(&sc->sub[1].light, 1.0F, 0.0F, 0.0F);
    GXInitLightColor(&sc->sub[1].light, LIGHT_COLOR);
    GXInitLightSpot(&sc->sub[1].light, 0.0F, GX_SP_OFF );

    // model control matrix ( never changed in this test )
    MTXScale(ms, 350.0F, 100.0F, 350.0F);
    MTXConcat(sc->sub[0].cam.view, ms, sc->sub[0].modelCtrl);
    MTXScale(ms, 1000.0F, 400.0F, 1.0F);
    MTXConcat(sc->sub[1].cam.view, ms, sc->sub[1].modelCtrl);

    // textures from tpl
    TEXGetPalette(&MyTplObj, "gxTests/lit-10.tpl");
    TEXGetGXTexObjFromPalette(MyTplObj, &sc->sub[0].texture, 0);
    TEXGetGXTexObjFromPalette(MyTplObj, &sc->sub[1].texture, 1);
    GXInvalidateTexAll();
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx mv, mvi;

    // reflects controlable parameters into light objects.
    GXInitLightSpot(
        &sc->sub[0].light,
        (f32)(sc->sub[0].param0),
        SpFnTable[sc->sub[0].funcType] );
    GXInitLightDistAttn(
        &sc->sub[1].light,
        (f32)(sc->sub[1].param0),
        (f32)(sc->sub[1].param1 * 0.01F),
        DaFnTable[sc->sub[1].funcType] );

    // set up Tev mode = one color / one texture
    GXSetNumTexGens(1); // number of texture coord generators
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    
    // load texture
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTexGens(1);

    GXLoadTexObj(&sc->sub[sc->mode].texture, GX_TEXMAP0);
    // set up lights
    SetLight(&sc->sub[sc->mode].light);
    
    // set up modelview matrix
    GXLoadPosMtxImm(sc->sub[sc->mode].modelCtrl, GX_PNMTX0);
    MTXInverse(sc->sub[sc->mode].modelCtrl, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    if ( sc->mode == 0 )
        DrawModel0();
    else
        DrawModel1();
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    u32  m = sc->mode;
    s32  d, min, max;
    u16  dirs = DEMOPadGetDirs(0);
    u16  down = DEMOPadGetButtonDown(0);

    // change parameter 0
    d   = ( m == 0 ) ? 1 : 10;
    min = ( m == 0 ) ? 5 : 10;
    max = ( m == 0 ) ? 90 : 1000;
    
    if ( dirs & DEMO_STICK_LEFT )
    {
        sc->sub[m].param0 -= d;
        if ( sc->sub[m].param0 < min )
        {
            sc->sub[m].param0 = min;
        }
        StatusMessage(sc);
    }
    if ( dirs & DEMO_STICK_RIGHT )
    {
        sc->sub[m].param0 += d;
        if ( sc->sub[m].param0 > max )
        {
            sc->sub[m].param0 = max;
        }
        StatusMessage(sc);
    }

    // change parameter 1
    d   = ( m == 0 ) ? 0 : 5;
    min = ( m == 0 ) ? 0 : 5;
    max = ( m == 0 ) ? 1 : 95;
    
    if ( dirs & DEMO_STICK_DOWN )
    {
        sc->sub[m].param1 -= d;
        if ( sc->sub[m].param1 < min )
        {
            sc->sub[m].param1 = min;
        }
        StatusMessage(sc);
    }
    if ( dirs & DEMO_STICK_UP )
    {
        sc->sub[m].param1 += d;
        if ( sc->sub[m].param1 > max )
        {
            sc->sub[m].param1 = max;
        }
        StatusMessage(sc);
    }

    // change function type
    max = ( m == 0 ) ? NUM_SPFUNC : NUM_DAFUNC;

    if ( down & PAD_BUTTON_Y )
    {
        sc->sub[m].funcType = ( sc->sub[m].funcType + 1 ) % max;
        StatusMessage(sc);
    }
    if ( down & PAD_BUTTON_X )
    {
        sc->sub[m].funcType = ( sc->sub[m].funcType + max - 1 ) % max;
        StatusMessage(sc);
    }
    
    // change sub scene (angle test/distance test)
    if ( down & PAD_BUTTON_B )
    {
        sc->mode = 1 - sc->mode;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawModel0
    
    Description:    Draw the model for angluar attenuation test
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModel0( void )
{
    s32  x, y;
    f32  xp, yp, zp, s, t;

    // set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // drawing loop
    for ( y = 0 ; y < 4 ; ++y )
    {
        yp = 1.0F - (f32)y * 0.5F;
        t  = (f32)y * 0.25F;
    
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 181*2);
            for ( x = 0 ; x <= 180 ; ++x )
            {
                xp = - cosf((f32)x * PI / 180.0F);
                zp = - sinf((f32)x * PI / 180.0F);
                s  = (f32)x/180.0F;

                GXPosition3f32(xp, yp - 0.5F, zp);
                GXNormal3f32(-xp, 0.0F, -zp);
                GXTexCoord2f32(s, t + 0.25F);
            
                GXPosition3f32(xp, yp, zp);
                GXNormal3f32(-xp, 0.0F, -zp);
                GXTexCoord2f32(s, t);
            }
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawModel1
    
    Description:    Draw the model for distance attenuation test
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModel1( void )
{
    f32  xp, yp;

    // set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // drawing loop
    for ( yp = 0.0F ; yp < 1.0F ; yp += 0.25F )
    {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 100*2);
            for ( xp = 0.01F ; xp <= 1.0F ; xp += 0.01F )
            {
                GXPosition3f32(xp - 0.5F, yp - 0.5F, 0.0F);
                GXNormal3f32(-xp, 0.5F - yp, 0.0F);
                GXTexCoord2f32(xp, 1.0F - yp);

                GXPosition3f32(xp - 0.5F, yp - 0.25F, 0.0F);
                GXNormal3f32(-xp, 0.25F - yp, 0.0F);
                GXTexCoord2f32(xp, 0.75F - yp);
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
    
    Description:    sets light objects and color channels
                    
    Arguments:      lobj : pointer to GXLightObj structure to be used.
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetLight( GXLightObj* lobj )
{
    // loads given light object
    GXLoadLightObjImm(lobj, GX_LIGHT0);

    // channel setting
    GXSetNumChans(1); // number of color channels
    GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_SPOT);
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_DISABLE,  // disable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        0,           // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_SPOT);
    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0A0, REG_AMBIENT);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, REG_MATERIAL);
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
    OSReport("lit-atn-func: convienance light attn. function test\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("B Button     : Switch Angle test/Distance test\n");
    OSReport("X/Y Buttons  : Select Attenuation Function type\n");
    OSReport("Main Stick X : Change cutoff angle (angle test)\n");
    OSReport("             : Change ref. distance (distance test)\n");
    OSReport("Main Stick Y : Change ref. brightness (distance test)\n");
    OSReport("******************************************************\n");
    OSReport("\n");
}

/*---------------------------------------------------------------------------*
    Name:           StatusMessage
    
    Description:    Prints the current status.
 *---------------------------------------------------------------------------*/
static void StatusMessage( MySceneCtrlObj* sc )
{
    if ( sc->mode == 0 )
    {
        OSReport("Cutoff=%d  ", sc->sub[0].param0);
        OSReport("Function=%s ", SpFnStr[sc->sub[0].funcType]);
        OSReport("\n");
    }
    else
    {
        OSReport("Ref_Dist=%d  ", sc->sub[1].param0);
        OSReport("Ref_Br=%f  ", (f32)sc->sub[1].param1 / 100.0);
        OSReport("Function=%s ", DaFnStr[sc->sub[1].funcType]);
        OSReport("\n");
    }
}

/*============================================================================*/
