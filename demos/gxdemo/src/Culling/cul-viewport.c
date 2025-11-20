/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     cul-viewport.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Culling/cul-viewport.c $
    
    11    2/15/01 11:30a Hirose
    removed obsolete ifdefs
    
    10    11/01/00 10:41a Hirose
    added value range limitation of scissor box
    
    9     7/07/00 5:57p Dante
    PC Compatibility
    
    8     7/01/00 1:04p Alligator
    added intro, fixed text location
    
    7     6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    6     5/17/00 5:12p Hirose
    fixed Z mode parametrer settings
    
    5     5/02/00 4:03p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    4     3/23/00 1:27a Hirose
    updated to use DEMOPad library and de-tab code
    
    3     3/13/00 3:20p Shiki
    Added M_NUMBEROF().

    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code

    1     3/06/00 12:07p Alligator
    move from gx/tests and rename

    10    3/03/00 4:21p Alligator
    integrate with ArtX source

    9     2/24/00 7:06p Yasu
    Rename gamepad key to match HW1

    8     2/01/00 7:22p Alligator
    second GX update from ArtX

    7     1/25/00 2:43p Carl
    Changed to standardized end of test message

    6     1/25/00 12:13p Carl
    Fixed spelling error.

    5     1/20/00 4:08p Carl
    Removed #ifdef EPPC stuff

    4     1/18/00 5:53p Carl
    Fixed aspect ratio problem, plus one more GXVerify issue.

    3     1/18/00 2:28p Hirose
    added GXSetNumChans( 1 ) for color channel setting

    2     1/13/00 8:53p Danm
    Added GXRenderModeObj * parameter to DEMOInit()

    9     11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port

    8     11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode

    7     10/29/99 3:46p Hirose
    replaced GXSetTevStages(GXTevStageID) by GXSetNumTevStages(u8)

    6     10/26/99 1:34p Alligator
    change GXSetDefaultMatrix to GXSetDefaultMtx

    5     10/22/99 4:45p Yasu
    Add GXSetTevStages and GXSetTevOrder

    4     10/21/99 3:01p Yasu
    Fixed a bug

    3     10/18/99 6:45p Yasu
    Fix up small error

    2     10/14/99 7:59p Yasu
    Change ViewPort, ScissorBox structures to fit API.

    1     10/13/99 10:19p Yasu
    Initial version
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>
#include "cmn-model.h"

#define SCREEN_WD       1       // Dummy - actual value filled at runtime
#define SCREEN_HT       1       // Dummy - actual value filled at runtime
#define SCREEN_DEPTH    128.0f
#define SCREEN_ZNEAR    0.0f    // near plane Z in screen coordinates
#define SCREEN_ZFAR 1.0f    // far  plane Z in screen coordinates
#define ZBUFFER_MAX 0x00ffffff

#define M_NUMBEROF(x)       (sizeof(x)/sizeof(*(x)))

#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
   Typedefs
 *---------------------------------------------------------------------------*/
typedef struct
{
    s32     xorg;
    s32     yorg;
    s32     width;
    s32     height;
}   ScissorBox;

typedef struct
{
    Camera* camera;
    ViewPort*   viewport;
    ScissorBox* scissor;
    GXColor*    bgcolor;
    s32         cursor;
    u32         screen_wd;
    u32         screen_ht;
}   Scene;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
static void    SceneDraw      ( Scene* );
static void    SceneDrawInfo  ( Scene* );
static void    ControlInit    ( Scene* );
static void    SceneControl   ( Scene* );
static void    myInitModel    ( void );
static void    myDrawModel    ( Camera* );
static void    myDrawRectangle( f32, f32, f32, f32, GXColor );
static void    PrintIntro     ( void );

/*---------------------------------------------------------------------------*
   Rendering parameters
 *---------------------------------------------------------------------------*/
static Camera   myCamera =
{
    { 0.0f,   0.0f, 0.0f }, // position
    { 0.0f,1000.0f, 0.0f }, // target
    { 0.0f,   0.0f, 1.0f }, // upVec
       33.3f,           // fovy
       16.0f,           // near plane Z in camera coordinates
     1024.0f,           // far  plane Z in camera coordinates
};

static ViewPort    myViewPort = { 0, 0, SCREEN_WD, SCREEN_HT };
static ScissorBox  myScissor  = { 0, 0, SCREEN_WD, SCREEN_HT };
static GXColor     myBgColor  = {  32,  32, 255, 255};
static Scene       myScene    =
   { &myCamera, &myViewPort, &myScissor, &myBgColor, 0, SCREEN_WD, SCREEN_HT };

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void    main ( void )
{
    GXRenderModeObj *rmp;

    // initialize render settings and set clear color for first frame
    DEMOInit(NULL);
    GXInvalidateTexAll( );

    PrintIntro();

    // Perform dummy copy to clear EFB by specified color
    GXSetCopyClear( *(myScene.bgcolor), GX_MAX_Z24 );
    GXCopyDisp( DEMOGetCurrentBuffer(), GX_TRUE );

    myInitModel( );

    rmp = DEMOGetRenderModeObj();

    myScene.screen_wd = rmp->fbWidth;
    myScene.screen_ht = rmp->efbHeight;
    myViewPort.width  = (s16) myScene.screen_wd;
    myViewPort.height = (s16) myScene.screen_ht;
    myScissor.width   = (u16) myScene.screen_wd;
    myScissor.height  = (u16) myScene.screen_ht;
    ControlInit( &myScene );

    while ( ! ( DEMOPadGetButton(0) & PAD_BUTTON_MENU ) )
    {
        // get pad status
        DEMOPadRead( );

        // General control & model animation
        SceneControl( &myScene );
        // No animation for this model.

        // Draw scene
        DEMOBeforeRender( );
        SceneDraw( &myScene );
        DEMODoneRender( );
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

//============================================================================
//  Scene
//============================================================================

/*---------------------------------------------------------------------------*
    Name:           SceneDraw
    Description:    Draw model
    Arguments:      Scene* s
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    SceneDraw( Scene* s )
{
    Camera* c  = s->camera;
    ViewPort*   v  = s->viewport;
    ScissorBox* si = s->scissor;
    float       aspect = (float) (4.0 / 3.0);
	GXColor color1 = {255,32,32,255};
	GXColor color2 = {32,255,32,255};

    // Pixel processing mode
    GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );
    GXSetZMode( GX_TRUE, GX_ALWAYS, GX_TRUE );

    // Culling mode
    GXSetCullMode( GX_CULL_BACK );

    // Set coordinates for rectangle drawing
    DEMOSetupScrnSpc( (s32) s->screen_wd, (s32) s->screen_ht, 100.0f );

    // Draw scissoring and viewport box
    myDrawRectangle( (f32) si->xorg, (f32) si->yorg,
                 (f32)(si->xorg + si->width  - 1),
                 (f32)(si->yorg + si->height - 1),
                 color1 );
    myDrawRectangle( (f32) v->xorg, (f32) v->yorg,
                 (f32)(v->xorg + v->width  - 1),
                 (f32)(v->yorg + v->height - 1),
                 color2 );

    // Set projection matrix
    MTXPerspective( c->projMtx, c->fovy, aspect, c->znear, c->zfar );
    GXSetProjection( c->projMtx, GX_PERSPECTIVE );

    // Set CameraView matrix
    MTXLookAt( c->viewMtx, &c->position, &c->up, &c->target );

    // Set Viewport
    GXSetViewport( v->xorg, v->yorg, v->width, v->height,
               SCREEN_ZNEAR, SCREEN_ZFAR );
    GXSetScissor( (u32)si->xorg, (u32)si->yorg, (u32)si->width, (u32)si->height );

    // Draw objects
    myDrawModel( c );

    // draw information
    SceneDrawInfo( s );
    return;
}

/*---------------------------------------------------------------------------*
    Name:           SceneDrawInfo
    Description:    Draw scene information
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    SceneDrawInfo( Scene* s )
{
    ViewPort*   v  = s->viewport;
    ScissorBox* si = s->scissor;
    s32     x, y;

    GXSetViewport( 0, 0, (f32) s->screen_wd, (f32) s->screen_ht,
                   SCREEN_ZNEAR, SCREEN_ZFAR );
    GXSetScissor ( 0, 0, (u32) s->screen_wd, (u32) s->screen_ht );
    
    // Draw paramters to the window
    DEMOInitCaption( DM_FT_XLU, (s32) s->screen_wd, (s32) s->screen_ht );
    DEMOPrintf( 15, 15, 0, "Scissor(Red)    (%5d,%5d)-(%5d,%5d)",
            si->xorg, si->yorg,
            (u16)(si->xorg + si->width), (u16)(si->yorg + si->height) );
    DEMOPrintf( 15, 25, 0, "ViewPort(Green) (%5d,%5d)-(%5d,%5d)",
            v->xorg,            v->yorg,
            v->xorg + v->width, v->yorg + v->height );

    // Draw cursor
    switch ( s->cursor )
    {
      case 0:  x = si->xorg;             y = si->yorg;               break;
      case 1:  x = si->xorg+si->width-1; y = si->yorg+si->height-1;  break;
      case 2:  x =  v->xorg;             y =  v->yorg;               break;
      case 3:  x =  v->xorg+v->width-1;  y =  v->yorg+v->height-1;   break;
    }
    DEMOPuts( (s16)(x-4), (s16)(y-5), 0, "o" );

    return;
}

/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static
ViewPort   myView[] =
{ { 0,           0,           SCREEN_WD*8, SCREEN_HT*8 },
  { SCREEN_WD*1, SCREEN_HT*1, SCREEN_WD*4, SCREEN_HT*4 } };

static
ScissorBox myScis[] =
{ { 0,           0,           SCREEN_WD*8, SCREEN_HT*8   },
  { SCREEN_WD*3, SCREEN_HT*3, SCREEN_WD*4, SCREEN_HT*4 } };

static
struct {
    ViewPort*   view;
    ScissorBox* scis;
} myWinPos[] =
  { { &myView[0], &myScis[0] },
    { &myView[0], &myScis[1] },
    { &myView[1], &myScis[1] },
    { &myView[1], &myScis[0] } };

/*---------------------------------------------------------------------------*
    Name:           ControlInit
    Description:    init parameter control
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    ControlInit( Scene* s )
{
    int i;

    for(i=0; i<2; i++)
    {
        myView[i].xorg   *= (s16) (s->screen_wd / 8);
        myView[i].yorg   *= (s16) (s->screen_ht / 8);
        myView[i].width  *= (s16) (s->screen_wd / 8);
        myView[i].height *= (s16) (s->screen_ht / 8);

        myScis[i].xorg   *= (u16) (s->screen_wd / 8);
        myScis[i].yorg   *= (u16) (s->screen_ht / 8);
        myScis[i].width  *= (u16) (s->screen_wd / 8);
        myScis[i].height *= (u16) (s->screen_ht / 8);
    }
}

/*---------------------------------------------------------------------------*
    Name:           SceneControl
    Description:    user interface for parameter control
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    SceneControl( Scene* s )
{
    static u32  predef = 0;
    ViewPort*   v  = s->viewport;
    ScissorBox* si = s->scissor;
    s32     step, dx, dy;

    // Modify box by Button A
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_A )
    {
        predef = ( predef + 1 ) % M_NUMBEROF(myWinPos);
        
        *v  = *(myWinPos[predef].view);
        *si = *(myWinPos[predef].scis);
    }

    // Change cursor by Button B
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_B )
    {
        s->cursor = ( s->cursor + 1 ) % 4;
    }

    // Move edge
    step = ( DEMOPadGetButton(0) & PAD_BUTTON_X ) ? 6 : 1;
    dy = - ( DEMOPadGetStickY(0) / 24 ) * step;
    dx =   ( DEMOPadGetStickX(0) / 24 ) * step;

    // Get pointer for values specified by cursor
    switch ( s->cursor )
    {
      case 0:   si->xorg  += dx; si->yorg   += dy;
            si->width -= dx; si->height -= dy;  break;
      case 1:   si->width += dx; si->height += dy;  break;
      case 2:    v->xorg  += dx;  v->yorg   += dy;
             v->width -= dx;  v->height -= dy;  break;
      case 3:    v->width += dx;  v->height += dy;  break;
    }

    Clamp(si->xorg, 0, (s32)s->screen_wd);
    Clamp(si->yorg, 0, (s32)s->screen_ht);
    Clamp(si->width,  0, (s32)(s->screen_wd - si->xorg));
    Clamp(si->height, 0, (s32)(s->screen_ht - si->yorg));

    return;
}

//============================================================================
//  Model
//  Rectangles
//============================================================================
typedef struct
{
    f32     x, y, z;
    f32     radius;
    GXColor color;
}   MySphere;

#define GRIDX   5
#define GRIDY   4

static MySphere    mySphere[GRIDX*GRIDY];
static GXLightObj  myLight;
static Point3d     myLightPos = { 1000.0f, -1000.0f, 1000.0f }; // in world space
/*---------------------------------------------------------------------------*
    Name:           myDrawModel
    Description:    draw model
    Arguments:      Camera *c
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    myInitModel( void )
{
    s32     i;
	GXColor white = {255,255,255,255};

    GXInitLightColor( &myLight, white );
    GXInitLightSpot( &myLight, 0.0f, GX_SP_OFF );
    GXInitLightDistAttn( &myLight, 0.0f, 0.0f, GX_DA_OFF );

    for ( i = 0; i < M_NUMBEROF(mySphere); i ++ )
    {
        mySphere[i].x = (float)(((i % GRIDX) * 2 - GRIDX + 1) * 10);
        mySphere[i].y = 90.0f;
        mySphere[i].z = (float)(((i / GRIDX) * 2 - GRIDY + 1) * 10);
        mySphere[i].radius = 10.0f;
        mySphere[i].color.r = (u8)((i & 1) ? 255 : 64);
        mySphere[i].color.g = (u8)((i & 2) ? 255 : 64);
        mySphere[i].color.b = (u8)((i & 4) ? 255 : 64);
        mySphere[i].color.a = 255;
    }
}

/*---------------------------------------------------------------------------*
    Name:           myDrawModel
    Description:    draw model
    Arguments:      Camera *c
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    myDrawModel( Camera *c )
{
    Point3d lpos;
    Mtx     mvmtx, mmtx;
    s32     i;
	GXColor black = { 0, 0, 0, 0 };

    // use constant material color
    GXSetNumChans( 1 );         // # of color channels
    GXSetChanCtrl( GX_COLOR0,
               GX_ENABLE,       // enable Channel
               GX_SRC_REG,      // amb source
               GX_SRC_REG,      // mat source
               GX_LIGHT0,       // light mask
               GX_DF_CLAMP,     // diffuse function
               GX_AF_NONE );    // atten   function
    GXSetChanCtrl( GX_ALPHA0,
               GX_DISABLE,      // enable Channel
               GX_SRC_REG,      // amb source
               GX_SRC_REG,      // mat source
               GX_LIGHT0,       // light mask
               GX_DF_NONE,      // diffuse function
               GX_AF_NONE );    // atten   function
    GXSetChanAmbColor( GX_COLOR0, black );

    // Set material color to go through PE stage.
    GXSetNumTexGens( 0 );       // # of Tex gens
    GXSetNumTevStages( 1 );     // # of Tev Stage
    GXSetTevOrder( GX_TEVSTAGE0,
                   GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );

    // set modelview matrix
    GXSetCurrentMtx( GX_PNMTX0 );

    // set a light object
    MTXMultVec( c->viewMtx, &myLightPos, &lpos );
    GXInitLightPos( &myLight, lpos.x, lpos.y, lpos.z ); // in view space
    GXLoadLightObjImm( &myLight, GX_LIGHT0 );

    // Set vtx desc and Draw primitives
    GXClearVtxDesc( );
    GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
    GXSetVtxDesc( GX_VA_NRM, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0 );

    mmtx[0][1] = mmtx[0][2] = mmtx[1][2] = 0.0f;
    mmtx[1][0] = mmtx[2][0] = mmtx[2][1] = 0.0f;
    for ( i = 0; i < M_NUMBEROF(mySphere); i ++ )
    {
        GXSetChanMatColor( GX_COLOR0, mySphere[i].color );
        mmtx[0][0] = mmtx[1][1] = mmtx[2][2] = mySphere[i].radius;
        mmtx[0][3] = mySphere[i].x;
        mmtx[1][3] = mySphere[i].y;
        mmtx[2][3] = mySphere[i].z;
        MTXConcat( c->viewMtx, mmtx, mvmtx );
    
        // Load ModelView matrix for position transformation into view space
        GXLoadPosMtxImm( mvmtx, GX_PNMTX0 );
    
        // Load ModelView matrix for normal transformation into view space
        // Note: mvmtx is uniform transform here, so that (mvmtx)^(-T) = mvmtx
        GXLoadNrmMtxImm( mvmtx, GX_PNMTX0 );
    
        GXDrawSphere1(1);
    }
    return;
}

/*---------------------------------------------------------------------------*
    Name:           myFillRectangle
    Description:    fill rectangle
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    myDrawRectangle( f32 x0, f32 y0, f32 x1, f32 y1, GXColor color )
{
    // Draw rectangle
    x0 += 0.5f;     // center of pixel
    y0 += 0.5f;
    x1 += 0.5f;
    y1 += 0.5f;

    // Set rendering mode
    GXSetNumChans( 1 );         // # of color channels
    GXSetChanCtrl( GX_COLOR0,
               GX_FALSE,        // enable Channel
               GX_SRC_REG,      // amb source
               GX_SRC_REG,      // mat source
               GX_LIGHT0,       // light mask
               GX_DF_NONE,      // diffuse function
               GX_AF_NONE );    // atten   function
    GXSetChanMatColor( GX_COLOR0, color );

    GXSetNumTexGens( 0 );       // # of Tex gens
    GXSetNumTevStages( 1 );     // # of Tev Stage
    GXSetTevOrder( GX_TEVSTAGE0,
                   GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );

    // Draw rectangle
    GXClearVtxDesc( );
    GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
    GXBegin( GX_LINESTRIP, GX_VTXFMT0, 5 );
        GXPosition3f32( x0, y0, 0.0f );
        GXPosition3f32( x1, y0, 0.0f );
        GXPosition3f32( x1, y1, 0.0f );
        GXPosition3f32( x0, y1, 0.0f );
        GXPosition3f32( x0, y0, 0.0f );
    GXEnd( );
    return;
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
    OSReport("*******************************************************\n");
    OSReport("cul-viewport: demonstrate viewport/scissor controls\n");
    OSReport("*******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("  Stick X/Y    : drag corner of viewport/scissor window\n");
    OSReport("  X button     : hold down to increase stick speed\n");
    OSReport("  A button     : step through different cases\n");
    OSReport("  B button     : move cursor\n");
    OSReport("*******************************************************\n\n");
}

/*======== End of cul-viewport.c ========*/
