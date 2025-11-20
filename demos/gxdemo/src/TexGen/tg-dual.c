/*---------------------------------------------------------------------------*
  Project:  Dolphin GX demo
  File:     tg-dual.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-dual.c $
    
    2     10/26/00 10:10a Hirose
    updated reference tpl file
    
    1     10/25/00 8:44p Hirose
    initial check in
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   tg-dual
     A simple demo of dual transform texgen feature
 *---------------------------------------------------------------------------*
     NOTE: This feature is only available on HW2 (or MAC2)
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI          3.14159265358979323846F

#define MODE_POS    0
#define MODE_NRM    1
#define NUM_MODES   2

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
    s32           theta;
    s32           phi;
    f32           distance;
} MyCameraObj;

typedef struct
{
    GXLightObj      lobj;
    MyCameraObj     cam;
} MyLightObj;

// for entire scene control
typedef struct
{
    MyCameraObj     cam;
    MyLightObj      light;
    GXTexObj        texture[NUM_MODES];
    Mtx             modelCtrl;
    u32             mode;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );

#if ( GX_REV != 1 ) // GX revision 2 or later only
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void SetCamera       ( MyCameraObj* cam );
static void DrawLightMark   ( MyLightObj* light );
static void SetLight        ( MyLightObj* light, Mtx view );
static void DisableLight    ( void );
static void SetTexGenPos    ( MySceneCtrlObj* sc );
static void SetTexGenNrm    ( MySceneCtrlObj* sc );
static void PrintIntro      ( void );
#endif // ( GX_REV != 1 )

/*---------------------------------------------------------------------------*
   Lighting parameters
 *---------------------------------------------------------------------------*/
#define COL_LIGHT       (GXColor){ 0xA0, 0xA0, 0xA0, 0xA0 }
#define COL_AMBIENT     (GXColor){ 0x60, 0x60, 0x60, 0x60 }
#define COL_MATERIAL    (GXColor){ 0x80, 0xFF, 0x40, 0xFF }

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 10.0F }, // location (actually not used)
    { 0.0F, 1.0F,  0.0F }, // up
    { 0.0F, 0.0F,  0.0F }, // tatget
    -160.0F, // left
     120.0F, // top
     500.0F, // near
    5000.0F  // far
};

static CameraConfig DefaultLightCamera =
{
    { 10.0F, 10.0F, 10.0F }, // location (actually not used)
    {  0.0F,  1.0F,  0.0F }, // up
    {  0.0F,  0.0F,  0.0F }, // target
    -32.0F,   // left
     32.0F,   // top   
    300.0F,   // near
   5000.0F    // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;    // scene control parameters
static TEXPalettePtr   MyTplObj = NULL;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);    // Init the OS, game pad, graphics and video.

#if ( GX_REV == 1 ) // GX revision 1

    OSReport("\n");
    OSReport("Sorry. This demo is only available on the platform\n");
    OSReport("which supports GX revision 2 features. Please try\n");
    OSReport("it on HW2 or MAC2 build platforms.\n\n");

#else // GX revision 2 or later
    
    DrawInit(&SceneCtrl);       // Initialize vertex formats and scene parameters.
    PrintIntro();               // Print demo directions
    
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);   // Draw the model.
        DEMODoneRender();
        DEMOPadRead();          // Update pad status.
        AnimTick(&SceneCtrl);   // Update animation.
    }

#endif // ( GX_REV == 1 )

    OSHalt("End of test");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
#if ( GX_REV != 1 ) // GX revision 2 or later
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
    u32     i;

    GXSetCopyClear((GXColor){ 0x40, 0x40, 0x40, 0xFF}, 0xFFFFFF);

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // textures
    TEXGetPalette(&MyTplObj, "gxTests/tg-dual.tpl");
    for ( i = 0 ; i < NUM_MODES ; ++i )
    {
        tdp = TEXGet(MyTplObj, i);
        GXInitTexObj(
            &sc->texture[i],
            tdp->textureHeader->data,
            tdp->textureHeader->width,
            tdp->textureHeader->height,
            (GXTexFmt)tdp->textureHeader->format,
            GX_CLAMP, // s
            GX_CLAMP, // t
            GX_FALSE );
    }


    // Default scene parameter settings

    // camera
    sc->cam.cfg      = DefaultCamera;
    sc->cam.theta    = 0;
    sc->cam.phi      = 0;
    sc->cam.distance = 2000.0F;

    // light parameters
    sc->light.cam.cfg      = DefaultLightCamera;
    sc->light.cam.theta    = 0;
    sc->light.cam.phi      = 60;
    sc->light.cam.distance = 1000.0F;

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
    s32  i;
    Mtx  mv, mvi, mr, ms, mm;

    // camera
    SetCamera(&sc->cam);
    GXSetProjection(sc->cam.proj, GX_PERSPECTIVE);

    // lighting
    SetLight(&sc->light, sc->cam.view);

    // set texture
    GXLoadTexObj(&sc->texture[sc->mode], GX_TEXMAP0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0); 
    
    // set up texture coord generation and TEV operation
    switch( sc->mode )
    {
      case MODE_POS :
        SetTexGenPos(sc);
        GXSetTevOp(GX_TEVSTAGE0, GX_DECAL);
        break;
      case MODE_NRM :
        SetTexGenNrm(sc);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        break;
    }

    // draw models
    MTXConcat(sc->cam.view, sc->modelCtrl, mm);
    MTXScale(ms, 150.0F, 150.0F, 150.0F);
    MTXConcat(mm, ms, mm);
    for ( i = 0 ; i < 5 ; ++i )
    {
        MTXTrans(ms, i-2, 0.0F, 0.0F);
        MTXRotDeg(mr, 'x', i*90);
        MTXConcat(ms, mr, mv);
        MTXConcat(mm, mv, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
        
        MTXInverse(mv, mvi);
        MTXTranspose(mvi, mv);
        GXLoadNrmMtxImm(mv, GX_PNMTX0);
        if ( sc->mode == MODE_NRM )
        {
            // A copy of normal transform matrix
            GXLoadTexMtxImm(mv, GX_TEXMTX0, GX_MTX3x4);
        }
    
        GXDrawTorus(0.25F, 12, 16);
    }

    // light mark
    if ( sc->mode == MODE_POS )
    {
        // disable lighting
        DisableLight();
        // set texture environments
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

        // transform matrix
        MTXRotDeg(mr, 'y', sc->light.cam.theta);
        MTXConcat(sc->cam.view, mr, mv);
        MTXRotDeg(mr, 'x', - sc->light.cam.phi);
        MTXConcat(mv, mr, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);

        // draw a light mark
        DrawLightMark(&sc->light);
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

    // Light position
    sc->light.cam.theta += ( DEMOPadGetSubStickX(0) / 24 );
    if ( sc->light.cam.theta > 360 )
    {
        sc->light.cam.theta -= 360;
    }
    if ( sc->light.cam.theta < 0 )
    {
        sc->light.cam.theta += 360;
    }
    sc->light.cam.phi += ( DEMOPadGetSubStickY(0) / 24 );
    Clamp(sc->light.cam.phi, -85, 85);
    
    // Model Rotation Calculation
    MTXRotDeg(mry, 'x', -(f32)DEMOPadGetStickY(0) / 24.0F);
    MTXRotDeg(mrx, 'y', (f32)DEMOPadGetStickX(0) / 24.0F);
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);

    // Projection control
    sc->light.cam.cfg.znear +=
        ( DEMOPadGetTriggerR(0) - DEMOPadGetTriggerL(0) ) / 16;
    Clamp(sc->light.cam.cfg.znear, 200.0F, 600.0F);

    // change mode
    if ( down & PAD_BUTTON_A )
    {
        sc->mode = ( sc->mode + 1 ) % NUM_MODES;
        switch(sc->mode)
        {
          case MODE_POS :
            OSReport("Position based projection\n");
            break;
          case MODE_NRM :
            OSReport("Normal based environment mapping\n");
            break;
        }
    }
    
}

/*---------------------------------------------------------------------------*
    Name:           DrawLightMark
    
    Description:    Draws a mark which shows position of the light.
                    
    Arguments:      light : light object
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawLightMark( MyLightObj* light )
{
    f32  dst, wd;

    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    dst = light->cam.distance;
    wd  = 64.0F * dst / light->cam.cfg.znear;

    GXBegin(GX_LINES, GX_VTXFMT0, 8);
        GXPosition3f32(wd, wd, -dst);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, dst);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(wd, -wd, -dst);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, dst);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(-wd, wd, -dst);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, dst);
        GXColor4u8(0, 255, 255, 255);

        GXPosition3f32(-wd, -wd, -dst);
        GXColor4u8(0, 255, 255, 255);
        GXPosition3f32(0.0F, 0.0F, dst);
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
    f32  r_theta, r_phi;
    
    r_theta = (f32)cam->theta * PI / 180.0F;
    r_phi   = (f32)cam->phi   * PI / 180.0F;

    cam->cfg.location.x =
        cam->distance * sinf(r_theta) * cosf(r_phi);
    cam->cfg.location.y =
        cam->distance * sinf(r_phi);
    cam->cfg.location.z =
        cam->distance * cosf(r_theta) * cosf(r_phi);

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

    // Light Position
    SetCamera(&light->cam);
    lpos = light->cam.cfg.location;

    // Convert light position into view space
    MTXMultVec(view, &lpos, &lpos);

    GXInitLightPos(&light->lobj, lpos.x, lpos.y, lpos.z);
    GXInitLightColor(&light->lobj, COL_LIGHT);
    GXLoadLightObjImm(&light->lobj, GX_LIGHT0);

    // Lighting channel
	GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);
    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0A0, COL_AMBIENT);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, COL_MATERIAL);

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
        GX_DISABLE,     // disable channel
        GX_SRC_VTX,     // amb source
        GX_SRC_VTX,     // mat source
        GX_LIGHT_NULL,  // light mask
        GX_DF_NONE,     // diffuse function
        GX_AF_NONE );
}

/*---------------------------------------------------------------------------*
    Name:           SetTexGenPos
    
    Description:    Set up texgen for position based projection
                    (by dual-transform)
                    
    Arguments:      sc : a pointer to MySceneCtrlObj structure
    
    Returns:        none
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Position transform matrix usually contains transformation as:

        Cv * Mv (where Cv = camera view, Mv = model view.)

    Now, texture projection requires the transformation like:

        Lp * Lv * Mv (where Lv = light view, Lp = projection.)

    In such case, if we set the matrix:

        Lp * Lv * Inv(Cv)

    for the second texgen transform matrix, we can re-use position matrix
    directly for the first texgen transform. This method allows us to do
    texgen matrix setting only once at the beginning. This is useful for
    doing projection on skinned surfaces too.
 *---------------------------------------------------------------------------*/
static void SetTexGenPos( MySceneCtrlObj* sc )
{
    Mtx  proj;
    Mtx  mci, mp, mv;

    MTXLightFrustum(
        proj,
        - (sc->light.cam.cfg.top),
        sc->light.cam.cfg.top,
        sc->light.cam.cfg.left,
        - (sc->light.cam.cfg.left),
        sc->light.cam.cfg.znear,
        0.5F,
        0.5F,
        0.5F,
        0.5F );
    
    MTXInverse(sc->cam.view, mci);
    MTXConcat(sc->light.cam.view, mci, mv);
    MTXConcat(proj, mv, mp);

    // Load as second texgen transform matrix
    GXLoadTexMtxImm(mp, GX_PTTEXMTX0, GX_MTX3x4);

    GXSetNumTexGens(1);
    GXSetTexCoordGen2(
        GX_TEXCOORD0,   // Output tex coord id
        GX_TG_MTX3x4,   // Texgen function type
        GX_TG_POS,      // Texgen source
        GX_PNMTX0,      // 1st. transform matrix ( re-use position matrix )
        GX_FALSE,       // normalization
        GX_PTTEXMTX0 ); // 2nd. transform matrix
}

/*---------------------------------------------------------------------------*
    Name:           SetTexGenNrm
    
    Description:    Set up texgen for normal based environment mapping
                    (by dual-transform)
                    
    Arguments:      sc : a pointer to MySceneCtrlObj structure
    
    Returns:        none
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Dual transform texgen is also useful for environment mapping.
    
    Unfortunately we can't access normal transform matrices for texgen directly.
    So this demo should load the same matrix into (first) texgen matrix space.
    
    But still there are some advantages because dual texgen can perform
    normalization after the first transform. It can allow unnormalized
    vertex normal sources or non-uniform scale transformations. And it
    saves additional matrix concatenations that should be done by the CPU.
 *---------------------------------------------------------------------------*/
static void SetTexGenNrm( MySceneCtrlObj* sc )
{
#pragma unused(sc)

    Mtx  mt, ms;
    
    // Just scaling and bias to adjust sphere map
    // Third row simply returns 1 for q coordinate
    MTXScale(ms, 0.50F, 0.50F, 0.0F);
    MTXTrans(mt, 0.50F, 0.50F, 1.0F);
    MTXConcat(mt, ms, ms);

    // Load as second texgen transform matrix
    GXLoadTexMtxImm(ms, GX_PTTEXMTX1, GX_MTX3x4);

    GXSetNumTexGens(1);
    GXSetTexCoordGen2(
        GX_TEXCOORD0,   // Output tex coord id
        GX_TG_MTX3x4,   // Texgen function type
        GX_TG_NRM,      // Texgen source
        GX_TEXMTX0,     // 1st. transform matrix
        GX_TRUE,        // normalization
        GX_PTTEXMTX1 ); // 2nd. transform matrix
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
    OSReport("tg-dual : HW2 dual transform texgen demo\n");
    OSReport("**********************************************\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
    OSReport("Main stick   : Rotate Models\n");
    OSReport("Sub stick    : Move Light Location\n");
    OSReport("L/R Triggers : Control Light Frustum\n");
    OSReport("A Button     : Change texgen mode\n");
    OSReport("**********************************************\n");
}

/*---------------------------------------------------------------------------*/
#endif // ( GX_REV != 1 )

/*============================================================================*/
