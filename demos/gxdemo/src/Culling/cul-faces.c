/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     cul-faces.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Culling/cul-faces.c $
    
    9     2/15/01 11:30a Hirose
    removed obsolete ifdefs
    
    8     7/01/00 12:39p Alligator
    Added intro, made model text visible
    
    7     6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    6     5/17/00 5:11p Hirose
    fixed Z mode parametrer settings
    
    5     5/02/00 4:03p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    4     3/24/00 3:05p Carl
    Changed aspect ratio to correspond to new rendermode.
    
    3     3/23/00 1:27a Hirose
    updated to use DEMOPad library and de-tab code
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:07p Alligator
    move from gx/tests and rename
    
    12    3/03/00 4:21p Alligator
    integrate with ArtX source
    
    11    3/02/00 5:35p Alligator
    ifdef for BUG_TRIANGLE_FAN
    
    10    2/24/00 7:06p Yasu
    Rename gamepad key to match HW1
    
    9     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    8     1/25/00 2:43p Carl
    Changed to standardized end of test message
    
    7     1/20/00 6:46p Carl
    Fixed screen caption.
    
    6     1/20/00 2:24p Carl
    Fixed last fixes
    
    5     1/20/00 1:51p Carl
    Removed #ifdef EPPC code.
    
    4     1/18/00 5:07p Carl
    Fixed aspect ratio problem for EPPC
    
    3     1/18/00 2:28p Hirose
    added GXSetNumChans( 1 ) for color channel setting
    
    2     1/13/00 8:53p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    11    11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    10    11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    9     10/29/99 3:46p Hirose
    replaced GXSetTevStages(GXTevStageID) by GXSetNumTevStages(u8)
    
    8     10/22/99 4:45p Yasu
    Add GXSetTevStages and GXSetTevOrder
    
    7     10/14/99 7:57p Yasu
    Change ViewPort structure to fit ViewPort/Scissor API
    
    6     10/13/99 4:32p Alligator
    change GXSetViewport, GXSetScissor to use xorig, yorig, wd, ht
    
    5     10/11/99 1:12p Yasu
    Fixed up num of vertex
    
    4     10/05/99 10:04a Yasu
    Aviod a warning
    
    3     9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    2     9/29/99 11:25p Yasu
    Fixed small bug aboult _SINGLEFRAME

    1     9/29/99 9:41p Yasu
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

/*---------------------------------------------------------------------------*
   Typedefs
 *---------------------------------------------------------------------------*/
typedef struct
{
    Camera*     camera;
    ViewPort*   viewport;
    GXCullMode  cullmode;
    u32         flag;
}   Scene;

#define SCENE_DRAWN (1<<0)

typedef struct
{
    u32         modelId;
    u32         numScene;
    Scene*      scene;
    u32         screen_wd;
    u32         screen_ht;
}   GlobalScene;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
static void    GlobalSceneDraw( GlobalScene* );
static void    SceneDraw      ( GlobalScene*, Scene* );
static void    SceneDrawInfo  ( Scene* );
static void    SceneControl   ( GlobalScene* );
static void    myDrawModel    ( GlobalScene*, Camera* );
static void    PrintIntro     ( void );

/*---------------------------------------------------------------------------*
   Rendering parameters
 *---------------------------------------------------------------------------*/
Camera   myCamera =
{
    { 0.0f,   0.0f, 0.0f }, // position
    { 0.0f,1000.0f, 0.0f }, // target
    { 0.0f,   0.0f, 1.0f }, // upVec
       33.3f,               // fovy
       16.0f,               // near plane Z in camera coordinates
     1024.0f,               // far  plane Z in camera coordinates
};

ViewPort myViewPort[] =
{
    // quarter size (these are adjusted in main)
    { 0,         0,         SCREEN_WD, SCREEN_HT },
    { 0,         SCREEN_HT, SCREEN_WD, SCREEN_HT },
    { SCREEN_WD, 0,         SCREEN_WD, SCREEN_HT },
    { SCREEN_WD, SCREEN_HT, SCREEN_WD, SCREEN_HT },
};

GXColor  myBgColor  = { 32, 32, 128,255};

Scene    myScene[] =
{
    { &myCamera, &myViewPort[0], GX_CULL_NONE,  SCENE_DRAWN },
    { &myCamera, &myViewPort[1], GX_CULL_BACK,  SCENE_DRAWN },
    { &myCamera, &myViewPort[2], GX_CULL_FRONT, SCENE_DRAWN },
    { &myCamera, &myViewPort[3], GX_CULL_ALL,   SCENE_DRAWN },
};

GlobalScene myGlobal =
{
    0,
    (sizeof(myScene)/sizeof(Scene)),
    myScene,
    SCREEN_WD,
    SCREEN_HT,
};

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void    main ( void )
{
    GXRenderModeObj *rmp;
    int i;

    // initialize render settings and set clear color for first frame
    DEMOInit(NULL);

    PrintIntro();

    // Perform dummy copy to clear EFB by specified color
    GXSetCopyClear( myBgColor, GX_MAX_Z24 );
    GXCopyDisp( DEMOGetCurrentBuffer(), GX_TRUE );

    rmp = DEMOGetRenderModeObj();
    myGlobal.screen_wd = rmp->fbWidth;
    myGlobal.screen_ht = rmp->efbHeight;

    for(i = 0; i < 4; i++) 
    {
        myViewPort[i].xorg   *= myGlobal.screen_wd/2;
        myViewPort[i].yorg   *= myGlobal.screen_ht/2;
        myViewPort[i].width  *= myGlobal.screen_wd/2;
        myViewPort[i].height *= myGlobal.screen_ht/2;
    }
    
    while ( ! ( DEMOPadGetButton(0) & PAD_BUTTON_MENU ) )
    {
        // get pad status
        DEMOPadRead( );
        
        // General control & model animation
        SceneControl( &myGlobal );
        // No animation for this model.

        // Draw scene
        DEMOBeforeRender( );
        GlobalSceneDraw( &myGlobal );
        DEMODoneRender( );
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

//============================================================================
//  Global Scene
//============================================================================
static void    myDrawTriangles     ( s32 );
static void    myDrawQuads         ( s32 );
static void    myDrawTriangleStrip ( s32 );
static void    myDrawTriangleFan   ( s32 );

typedef struct
{
    void    (*draw)();
    char*   name;
}   MyModel;

MyModel     myModel[] =
{
 { myDrawTriangles,      "Triangles"      },
 { myDrawTriangleStrip,  "Triangle Strip" },
 { myDrawTriangleFan,    "Triangle Fan"   },
 { myDrawQuads,          "Quads"          },
};

#define NUMMODELS   (sizeof(myModel)/sizeof(MyModel))

static
void    GlobalSceneDraw( GlobalScene* g )
{
    u32     i;

    for ( i = 0; i < g->numScene; i ++ )
    {
    SceneDraw( g, &g->scene[i] );
    }

    // Draw primitive name onto the window
    GXSetViewport( 0, 0, (f32) g->screen_wd, (f32) g->screen_ht,
                   SCREEN_ZNEAR, SCREEN_ZFAR );
    GXSetScissor( 0, 0, g->screen_wd, g->screen_ht );
    DEMOInitCaption( DM_FT_XLU, (s32) g->screen_wd, (s32) g->screen_ht );
    DEMOPrintf( 10, 12, 0, myModel[g->modelId].name );

    return;
}

//============================================================================
//  Scene
//============================================================================
typedef struct
{
    GXCullMode  mode;
    char*       name;
}   CullMode;

CullMode    myCullMode[] =
{
 { GX_CULL_NONE,    "CULL NONE"  },
 { GX_CULL_FRONT,   "CULL FRONT" },
 { GX_CULL_BACK,    "CULL BACK"  },
 { GX_CULL_ALL,     "CULL ALL"   },
};

#define NUMCULLMODES    (sizeof(myCullMode)/sizeof(CullMode))

/*---------------------------------------------------------------------------*
    Name:           SceneDraw
    Description:    Draw model
    Arguments:      Scene* s
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    SceneDraw( GlobalScene* g, Scene* s )
{
    Camera* c = s->camera;
    ViewPort*   v = s->viewport;
    float       aspect = (float) (10.0 / 7.0);

    // Check if drawn flag
    if ( !(s->flag & SCENE_DRAWN) ) return;

    // Set Perspective Viewing frustum
    MTXPerspective( c->projMtx, c->fovy, aspect, c->znear, c->zfar );
    GXSetProjection( c->projMtx, GX_PERSPECTIVE );

    // Set CameraView matrix
    MTXLookAt( c->viewMtx, &c->position, &c->up, &c->target );

    // Set Viewport
    GXSetViewport( v->xorg, v->yorg, v->width, v->height,
               SCREEN_ZNEAR, SCREEN_ZFAR );
    GXSetScissor( (u32)v->xorg, (u32)v->yorg, (u32)v->width, (u32)v->height );

    // Pixel processing mode
    GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );
    GXSetZMode( GX_TRUE, GX_ALWAYS, GX_TRUE );
    
    // Culling mode
    GXSetCullMode( s->cullmode );

    // Draw objects
    myDrawModel( g, c );

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
    ViewPort*   v = s->viewport;
    u32     lid;
    s16     sx, sy;

    // Draw paramters to the window
    DEMOInitCaption( DM_FT_XLU, v->width, v->height );

    // Temporary cullmode for displaying captions
    GXSetCullMode( GX_CULL_NONE );

    // get cullmode id
    for ( lid = NUMCULLMODES-1; lid > 0; lid -- )
    {
        if ( s->cullmode == myCullMode[lid].mode ) break;
    }
    sx = (s16)( v->width / 2 - 36 );
    sy = (s16)( v->height * 0.9 );
    DEMOPrintf( sx, sy, 0, myCullMode[lid].name );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           SceneControl
    Description:    user interface for paramter control
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    SceneControl( GlobalScene* g )
{
    // Change model by Button A
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_A )
    {
        g->modelId = ( g->modelId + 1 ) % NUMMODELS;
    }
    return;
}

//============================================================================
//  Model
//  Rectangles
//============================================================================
typedef struct
{
    s16  x, z;
    u8   r, g, b;
}   MyVtx;

#define RSIZE    50
#define RPOSX    70
#define RPOSY   500 // direction for depth constant
#define RPOSZ     0

static void    myDrawArrow( MyVtx *v, s32, s32 );
static void    mySendVtx( MyVtx*, s32, s32 );

/*---------------------------------------------------------------------------*
    Name:           myDrawModel
    Description:    draw model
    Arguments:      Camera *c
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    myDrawModel( GlobalScene* g, Camera *c )
{
    // set modelview matrix
    GXSetCurrentMtx( GX_PNMTX0 );
    GXLoadPosMtxImm( c->viewMtx, GX_PNMTX0 );

    // use constant material color
    GXSetNumChans( 1 );         // # of color channels
    GXSetChanCtrl( GX_COLOR0,
               GX_FALSE,        // enable Channel
               GX_SRC_REG,      // amb source       (Don't care)
               GX_SRC_VTX,      // mat source
               GX_LIGHT0,       // light mask       (Don't care)
               GX_DF_NONE,      // diffuse function (Don't care)
               GX_AF_NONE );    // atten   function (Don't care)

    // Set material color to go through PE stage.
    GXSetNumTexGens( 0 );       // # of Tex gen
    GXSetNumTevStages( 1 );     // # of Tev Stage
    GXSetTevOrder( GX_TEVSTAGE0,
                   GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );

    // Set vtx desc
    GXClearVtxDesc( );
    GXSetVtxDesc( GX_VA_POS,  GX_DIRECT );
    GXSetVtxDesc( GX_VA_CLR0, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ, GX_S16,  0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0 );

    // Draw primitives
    myModel[g->modelId].draw(  1 );
    myModel[g->modelId].draw( -1 );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           myDrawTriangles
    Description:    draw triangles
    Arguments:      s32 xflip
    Returns:        none
 *---------------------------------------------------------------------------*/
static
MyVtx  myTri[] =
{
    {  1,  2,  255,  0,  0 },
    { -1,  2,    0,255,  0 },
    {  1,  0,    0,  0,255 },
    { -1,  0,  255,255,  0 },
    { -1, -2,    0,255,255 },
    {  1, -2,  255,  0,255 },
};

static
void    myDrawTriangles( s32 xflip )
{
    // Draw rectangle
    GXBegin( GX_TRIANGLES, GX_VTXFMT0, 6 );
        mySendVtx( myTri, 6, xflip );
    GXEnd( );

    // Draw arrow
    myDrawArrow( myTri+0, 3, xflip );
    myDrawArrow( myTri+3, 3, xflip );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           myDrawQuads
    Description:    draw quad
    Arguments:      s32 xflip
    Returns:        none
 *---------------------------------------------------------------------------*/
static
MyVtx  myQuad[] =
{
    {  1,  2,  255,  0,  0 },
    { -1,  2,    0,255,  0 },
    { -1,  0,  255,  0,255 },
    {  1,  0,    0,  0,255 },
    {  0, -2,  255,255,  0 },
    {  1, -1,    0,255,255 },
    {  0,  0,  128,  0,255 },
    { -1, -1,    0,128,255 },
};

static
void    myDrawQuads( s32 xflip )
{
    // Draw rectangle
    GXBegin( GX_QUADS, GX_VTXFMT0, 8 );
        mySendVtx( myQuad, 8, xflip );
    GXEnd( );

    // Draw arrow
    myDrawArrow( myQuad+0, 4, xflip );
    myDrawArrow( myQuad+4, 4, xflip );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           myDrawTriangleStrip
    Description:    draw triangle strip
    Arguments:      s32 xflip
    Returns:        none
 *---------------------------------------------------------------------------*/
static
MyVtx  myTriStrip[] =
{
    {  1,  2,  255,  0,  0 },
    { -1,  2,    0,255,  0 },
    {  1,  0,    0,  0,255 },
    { -1,  0,  255,255,  0 },
    { -1, -2,    0,255,255 },
    {  1, -2,  255,  0,255 },
};

static
void    myDrawTriangleStrip( s32 xflip )
{
    // Draw rectangle
    GXBegin( GX_TRIANGLESTRIP, GX_VTXFMT0, 6 );
    mySendVtx( myTriStrip, 6, xflip );
    GXEnd( );

    // Draw arrow
    myDrawArrow( myTriStrip, 6, xflip );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           myDrawTriangleFan
    Description:    draw triangle fan
    Arguments:      s32 xflip
    Returns:        none
 *---------------------------------------------------------------------------*/
static
MyVtx  myTriFan[] =
{
    {  1,  0,  255,  0,  0 },
    {  1,  2,    0,255,  0 },
    { -1,  2,    0,  0,255 },
    { -1,  0,  255,255,  0 },
    {  1, -2,    0,255,255 },
    { -1, -2,  255,  0,255 },
};

static
void    myDrawTriangleFan( s32 xflip )
{
    // Draw rectangle
#ifdef BUG_TRIANGLE_FAN
    GXBegin( GX_TRIANGLES, GX_VTXFMT0, 12 );
    mySendVtx( myTriFan, 3, xflip );

    mySendVtx( myTriFan, 1, xflip );
    mySendVtx( &myTriFan[2], 2, xflip );

    mySendVtx( myTriFan, 1, xflip );
    mySendVtx( &myTriFan[3], 2, xflip );

    mySendVtx( myTriFan, 1, xflip );
    mySendVtx( &myTriFan[4], 2, xflip );
    GXEnd( );
#else
    GXBegin( GX_TRIANGLEFAN, GX_VTXFMT0, 6 );
    mySendVtx( myTriFan, 6, xflip );
    GXEnd( );
#endif // BUG_TRIANGLE_FAN

    // Draw arrow
    myDrawArrow( myTriFan, 6, xflip );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           myDrawArrow
    Description:    draw arrow
    Arguments:      MyVtx *v, s32 nvtx, s32 xflip
    Returns:        none
 *---------------------------------------------------------------------------*/
#define ARROWSIZE   10
#define COS30       (1.7320508f/2.0f)
#define SIN30       (1.0000000f/2.0f)

static
void    myDrawArrow( MyVtx *v, s32 nvtx, s32 xflip )
{
    u32     i;
    s32     xs, zs, x0, z0, x1, z1;
    f32     d, c, s;

    // Draw arrow
    GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR );
    GXBegin( GX_LINESTRIP, GX_VTXFMT0, (u8)(nvtx + 3) );
    for ( i = 0; i < nvtx; i ++ )
    {
        xs = ( RPOSX + v[i].x * RSIZE ) * xflip;
        zs =   RPOSZ + v[i].z * RSIZE;
        GXPosition3s16( (s16)xs, RPOSY, (s16)zs );
        GXColor3u8( 255, 255, 255 );
    }
    c = (float)( ( v[i-2].x - v[i-1].x ) * xflip );
    s = (float)  ( v[i-2].z - v[i-1].z );
    d = (float)ARROWSIZE / sqrtf( c * c + s * s );
    c *= d;
    s *= d;
    x0 = (s32)(  c * COS30 - s * SIN30 + 0.5f );
    z0 = (s32)(  c * SIN30 + s * COS30 + 0.5f );
    x1 = (s32)(  c * COS30 + s * SIN30 + 0.5f );
    z1 = (s32)(- c * SIN30 + s * COS30 + 0.5f );
    GXPosition3s16( (s16)( xs + x0 ), RPOSY, (s16)( zs + z0 ) );
    GXColor3u8( 255, 255, 255 );
    GXPosition3s16( (s16)xs, RPOSY, (s16)zs );
    GXColor3u8( 255, 255, 255 );
    GXPosition3s16( (s16)( xs + x1 ), RPOSY, (s16)( zs + z1 ) );
    GXColor3u8( 255, 255, 255 );
    GXEnd( );
    GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           mySendVtx
    Description:    send vertex into pipeline
    Arguments:      MyVtx* v, s32 nvtx, s32 xflip
    Returns:        none
 *---------------------------------------------------------------------------*/
static
void    mySendVtx( MyVtx* v, s32 nvtx, s32 xflip )
{
    u32     i;
    s32     xs, zs;

    for ( i = 0; i < nvtx; i ++ )
    {
    xs = ( RPOSX + v[i].x * RSIZE ) * xflip;
    zs =   RPOSZ + v[i].z * RSIZE;
    GXPosition3s16( (s16)xs, RPOSY, (s16)zs );
    GXColor3u8( v[i].r, v[i].g, v[i].b );
    }

    return;
}


/*---------------------------------------------------------------------------*
    Name:           PrintIntro
    Description:    Print directions on how to use this demo
    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("cul-faces: show different culling modes\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Button A     : change model\n");
} 

/*======== End of cul-faces.c ========*/
