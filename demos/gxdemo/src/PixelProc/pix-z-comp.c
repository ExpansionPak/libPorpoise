/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     pix-z-comp.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/PixelProc/pix-z-comp.c $
    
    9     5/06/01 8:55p Hirose
    removed obsolete flags
    
    8     7/01/00 3:22p Alligator
    intro stuff
    
    7     6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    6     5/17/00 9:05p Hirose
    fixed the last fix. After all Z mode is set in DEMOInitCaption
    
    5     5/17/00 8:06p Hirose
    deleted tab codes /
    set appropriate Z mode before calling DEMOPuts library because
    DEMOInitCaption doesn't set Z mode any more
    
    4     5/02/00 4:04p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    3     3/23/00 6:31p Hirose
    updated to use DEMOPad library instead of cmn-pad
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:11p Alligator
    move from tests/gx and rename
    
    9     2/24/00 11:53p Yasu
    
    8     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    7     1/25/00 2:56p Carl
    Changed to standardized end of test message
    
    6     1/25/00 12:15p Carl
    Fixed spelling error
    
    5     1/24/00 2:50p Carl
    Removed #ifdef EPPC stuff
    
    4     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    3     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/10/99 4:44p Carl
    Fixed Z compare stuff.
    
    11    11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    10    10/29/99 3:46p Hirose
    replaced GXSetTevStages(GXTevStageID) by GXSetNumTevStages(u8)
    
    9     10/22/99 7:42p Yasu
    Add GXSetTevStages and GXSetTevOrder
    
    8     10/14/99 7:57p Yasu
    Change ViewPort structure to fit ViewPort/Scissor API
    
    7     10/13/99 4:32p Alligator
    change GXSetViewport, GXSetScissor to use xorig, yorig, wd, ht
    
    6     9/30/99 10:08p Yasu
    Renamed functions and enums
    
    5     9/28/99 6:55p Yasu
    Change parameter of DEMOInitCaption

    4     9/28/99 3:35a Yasu
    Change indent form.
    Delete con-out

    3     9/28/99 12:06a Yasu
    Add SINGLEFRAME settings

    2     9/27/99 11:01p Yasu
    Add local model for Z func only

    1     9/23/99 8:51p Yasu
    Test for Z func
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include "cmn-model.h"

#define SCREEN_WD   1           // Dummy - actual value filled at runtime
#define SCREEN_HT   1           // Dummy - actual value filled at runtime
#define SCREEN_DEPTH    128.0f
#define SCREEN_ZNEAR    0.0f    // near plane Z in screen coordinates
#define SCREEN_ZFAR 1.0f        // far  plane Z in screen coordinates
#define ZBUFFER_MAX 0x00ffffff

/*---------------------------------------------------------------------------*
   Typedefs
 *---------------------------------------------------------------------------*/
typedef struct
{
    char*       title;
    // GXSetZMode
    GXBool      compare_enable;
    GXCompare   func;
    GXBool      update_enable;
    // GXSetZCompLoc
    GXBool      before_tex;
}   ZMode;

typedef struct
{
    Model*      model;
    Camera*     camera;
    ViewPort*   viewport;
    ZMode*      zmode;
    u32         flag;
}   Scene;

#define SCENE_DRAWN     (1<<0)
#define SCENE_CURSOR    (1<<1)

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void    SceneDraw( Scene* );
void    SceneDrawInfo( Scene* );
void    SceneControl( void );
void    myDrawModel( s32, s32, ZMode* );
static  void PrintIntro( void );

/*---------------------------------------------------------------------------*
   Rendering parameters
 *---------------------------------------------------------------------------*/
Camera  myCamera =
{
    { 0.0f,   0.0f, 30.0f },    // position (ignored in this demo)
    { 0.0f,1000.0f,  0.0f },    // target   (ignored in this demo)
    { 0.0f,   0.0f,  1.0f },    // upVec    (ignored in this demo)
       33.3f,           // fovy     (ignored in this demo)
       16.0f,           // near plane Z in camera coordinates
     1024.0f,           // far  plane Z in camera coordinates
};

ViewPort myViewPort[] =
{
    // full size (these are adjusted in main)
    { 0,         0,         SCREEN_WD*2, SCREEN_HT*2 },
    // half size
    { 0,         0,         SCREEN_WD,   SCREEN_HT },
    { 0,         SCREEN_HT, SCREEN_WD,   SCREEN_HT },
    { SCREEN_WD, 0,         SCREEN_WD,   SCREEN_HT },
    { SCREEN_WD, SCREEN_HT, SCREEN_WD,   SCREEN_HT },
};

ZMode   myZMode[] =
{
    { NULL, GX_TRUE, GX_EQUAL,   GX_TRUE, GX_TRUE },
    { NULL, GX_TRUE, GX_LESS,    GX_TRUE, GX_TRUE },
    { NULL, GX_TRUE, GX_GREATER, GX_TRUE, GX_TRUE },
    { NULL, GX_TRUE, GX_ALWAYS,  GX_TRUE, GX_TRUE },
};

// ZScale for test
float   sceneZScale = 1.0f;

GXColor sceneBgColor = { 32, 32, 128,255};

Scene   myScene[] =
{
    { &cmModel[1], &myCamera, &myViewPort[0], &myZMode[0], 0 },
    { &cmModel[1], &myCamera, &myViewPort[1], &myZMode[0], 0 },
    { &cmModel[1], &myCamera, &myViewPort[2], &myZMode[1], 0 },
    { &cmModel[1], &myCamera, &myViewPort[3], &myZMode[2], 0 },
    { &cmModel[1], &myCamera, &myViewPort[4], &myZMode[3], 0 },
};

#define NUMSCENE    (sizeof(myScene)/sizeof(Scene))

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void    main ( void )
{
    GXRenderModeObj *rmp;
    u32     i;

    // initialize render settings and set clear color for first frame
    DEMOInit( NULL );  // Defined in $DOLPHIN/build/libraries/demo/src/DEMOInit.c
    GXInvalidateTexAll( );
    GXSetCopyClear( sceneBgColor, GX_MAX_Z24 );

    // Perform dummy copy operation to clear eFB by specified color
    GXCopyDisp( DEMOGetCurrentBuffer(), GX_TRUE );

    rmp = DEMOGetRenderModeObj();

    for(i = 0; i < 5; i++) 
    {
        myViewPort[i].xorg   *= rmp->fbWidth/2;
        myViewPort[i].yorg   *= rmp->efbHeight/2;
        myViewPort[i].width  *= rmp->fbWidth/2;
        myViewPort[i].height *= rmp->efbHeight/2;
    }
    
    PrintIntro();

    while ( ! ( DEMOPadGetButton(0) & PAD_BUTTON_MENU ) )
    {
        // get pad status
        DEMOPadRead( );
        // General control & model animation
        SceneControl( );
        // No animation for this model.
    
        // Draw scene
        DEMOBeforeRender( );
        for ( i = 0; i < NUMSCENE; i ++ )
        {
            SceneDraw( &myScene[i] );
        }
        DEMODoneRender( );
    }

    OSHalt("End of test");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

//============================================================================
//  Scene
//============================================================================
typedef struct
{
    GXCompare   func;
    char*   name;
}   ZComp;

ZComp   myZComp[] =
{
 { GX_NEVER,      "NEVER"            },
 { GX_LESS,       "LESS"             },
 { GX_LEQUAL,     "LESS OR EQUAL"    },
 { GX_EQUAL,      "EQUAL"            },
 { GX_NEQUAL,     "NOT EQUAL"        },
 { GX_GEQUAL,     "GREATER OR EQUAL" },
 { GX_GREATER,    "GREATER"          },
 { GX_ALWAYS,     "ALWAYS"           },
};

#define NUMZCOMPS   (sizeof(myZComp)/sizeof(ZComp))

/*---------------------------------------------------------------------------*
    Name:           SceneDraw
    Description:    Draw model
    Arguments:      Scene* s
    Returns:        none
 *---------------------------------------------------------------------------*/
void    SceneDraw( Scene* s )
{
    ViewPort*   v = s->viewport;
    ZMode*  z = s->zmode;

    // Check if drawn flag
    if ( !(s->flag & SCENE_DRAWN) ) return;

    // Set Viewport
    GXSetViewport( v->xorg, v->yorg, v->width, v->height,
                   SCREEN_ZNEAR, SCREEN_ZFAR );
    GXSetScissor( (u32)v->xorg, (u32)v->yorg, (u32)v->width, (u32)v->height );

    // Draw objects
    myDrawModel( (s32)v->width, (s32)v->height, z );

    // draw information
    SceneDrawInfo( s );
    
    // Z mode should be enabled if you want to clear Z buffer
    // at the timing of copy to XFB
    GXSetZMode( GX_TRUE, GX_ALWAYS, GX_TRUE );
    
    return;
}

/*---------------------------------------------------------------------------*
    Name:           SceneDrawInfo
    Description:    Draw scene information
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
void    SceneDrawInfo( Scene* s )
{
    Camera*     c = s->camera;
    ViewPort*   v = s->viewport;
    ZMode*      z = s->zmode;
    s32         lid;
    s8          log2;
    union
    {
        f32 fval;
        struct
        {
            s32     sgn:1;
            u32     exp:8;
            u32     mts:23;
        }   field;
    }   zfactor;

    // Draw paramters to the window
    DEMOInitCaption( DM_FT_XLU, v->width, v->height );
    if ( z->title )
    {
        DEMOPuts( 15, 12, 0, z->title );
    }
    else
    {
        // get zcomp id
        for ( lid = NUMZCOMPS-1; lid > 0; lid -- )
        {
            if ( z->func == myZComp[lid].func ) break;
        }
        DEMOPuts( 15, 12, 0, myZComp[lid].name );
    }

    // Draw precision of zscale
    zfactor.fval = sceneZScale / SCREEN_DEPTH;
    log2 = (s8)(zfactor.field.exp - 127);
    DEMOPrintf( 160, 12, 0, "ZSCALE 2^%d", log2 );

    // Draw cursor
    if ( s->flag & SCENE_CURSOR ) DEMOPuts( 7, 12, 0, "\x7f" );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           SceneControl
    Description:    user interface for paramter control
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
void    SceneControl( void )
{
    static  s32 zoomMode = 0;
    static  s32 cursor   = 0;
    s32     i, lid;
    u16     button, stickDirs;
    Scene*  s;
    ZMode*  z;

    button    = DEMOPadGetButtonDown(0);
    stickDirs = DEMOPadGetDirsNew(0);

    // zoom mode
    if ( button & PAD_BUTTON_A ) zoomMode ^= 1;

    // change zscale
    if ( button & PAD_BUTTON_Y )
    {
        if ( sceneZScale < 1.0f ) sceneZScale *= 2.0f;
    }
    if ( button & PAD_BUTTON_X )
    {
        sceneZScale *= 0.5f;
    }

    if ( zoomMode )
    {
        //
        // *** zoom mode
        //
    
        // show specified scene in full screen and put cursor
        s = &myScene[cursor+1];
        z = s->zmode;
        myScene[0].zmode = z;
        myScene[0].flag  = SCENE_DRAWN | SCENE_CURSOR;
    
        // turn off another window
        for ( i = 1; i < NUMSCENE; i ++ )
        {
            myScene[i].flag = 0;
        }

        // get zmode id
        for ( lid = NUMZCOMPS-1; lid > 0; lid -- )
        {
            if ( z->func == myZComp[lid].func ) break;
        }
    
        // change zfunc parameter
        if ( stickDirs & DEMO_STICK_LEFT )
        {
            if ( lid > 0 ) lid --;
        }
        if ( stickDirs & DEMO_STICK_RIGHT )
        {
            if ( lid < NUMZCOMPS-1 ) lid ++;
        }

        // set parameter
        z->func = myZComp[lid].func;
    }
    else
    {
        //
        // *** catalog mode
        //
    
        // choose a scene.
        if ( stickDirs & DEMO_STICK_LEFT  ) cursor &= ~2; // left
        if ( stickDirs & DEMO_STICK_RIGHT ) cursor |=  2; // right
        if ( stickDirs & DEMO_STICK_UP    ) cursor &= ~1; // up
        if ( stickDirs & DEMO_STICK_DOWN  ) cursor |=  1; // down

        // show 4 small windows
        for ( i = 1; i < 5; i ++ )
        {
            myScene[i].flag = SCENE_DRAWN;
        }
    
        // turn off large window
        myScene[0].flag = 0;
    
        // set cursor
        s = &myScene[cursor+1];
        s->flag |= SCENE_CURSOR;
    }
    return;
}

//============================================================================
//  Model
//  Rectangles
//============================================================================
typedef struct
{
    f32      x0, y0, x1, y1, z;
    GXColor  color;
}   myModel;

myModel rect[] =
{
  //  x0  y0  x1  y1  z          color
  {   30, 20,170,180, 50, {  64, 64,255,255 }   },
  {   10, 40,120, 90, 51, { 255, 64, 64,255 }   },
  {   80,110,190,160, 25, {  64,255, 64,255 }   },
  {  110, 10,160,120, 75, { 255,255, 64,255 }   },
  {   40, 80, 90,190, 49, {  64,255,255,255 }   },
  {   70, 70,130,130, 50, { 255,255,255,255 }   },
};

#define NUMRECTS    (sizeof(rect)/sizeof(myModel))

/*---------------------------------------------------------------------------*
    Name:           myDrawModel
    Description:    draw model
    Arguments:      f32 width : width  of viewport
                    f32 height: height of viewport
    Returns:        none
 *---------------------------------------------------------------------------*/
void    myDrawModel( s32 width, s32 height, ZMode *z )
{
    Mtx     mtx;
    s32     i;

    // use constant material color
    GXSetChanCtrl( GX_COLOR0,
               GX_FALSE,        // enable Channel
               GX_SRC_REG,      // amb source       (Don't care)
               GX_SRC_REG,      // mat source
               GX_LIGHT0,       // light mask       (Don't care)
               GX_DF_NONE,      // diffuse function (Don't care)
               GX_AF_NONE );    // atten   function (Don't care)
    GXSetChanCtrl( GX_ALPHA0,
               GX_FALSE,        // enable Channel
               GX_SRC_REG,      // amb source       (Don't care)
               GX_SRC_REG,      // mat source
               GX_LIGHT0,       // light mask       (Don't care)
               GX_DF_NONE,      // diffuse function (Don't care)
               GX_AF_NONE );    // atten   function (Don't care)

    // set cull mode
    GXSetCullMode( GX_CULL_BACK );

    // Load tags in point sample mode
    DEMOLoadFont( GX_TEXMAP0, GX_TEXMTX0, DMTF_POINTSAMPLE );

    // Here, use orthographic screen coordinates.
    // Set normalized 0-200 screen coordinates and reduce z range in MTX0
    // Set vertex descripter on VTXFMT1
    DEMOSetupScrnSpc( width, height, SCREEN_DEPTH );
    MTXScale( mtx, ((float)width)/200.0f, ((float)height)/200.f, sceneZScale );
    GXLoadPosMtxImm( mtx, GX_PNMTX0 );
    GXSetVtxAttrFmt( GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );

    // Force to draw a first rectange
    GXSetZMode( GX_TRUE, GX_ALWAYS, GX_TRUE );

    for ( i = 0; i < NUMRECTS; i ++ )
    {
        // Set material color to go through PE stage.
        GXSetNumChans(1);         // # of light channels
        GXSetNumTexGens( 0 );     // # of Tex Gens
        GXSetNumTevStages( 1 );   // # of Tev Stage
        GXSetChanMatColor( GX_COLOR0A0, rect[i].color );
        GXSetTevOrder( GX_TEVSTAGE0,
                       GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
        GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
        GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );
    
        // Fill rectangle
        GXClearVtxDesc( );
        GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
        GXBegin( GX_QUADS, GX_VTXFMT1, 4 );
          GXPosition3f32( rect[i].x0, rect[i].y0, rect[i].z );
          GXPosition3f32( rect[i].x1, rect[i].y0, rect[i].z );
          GXPosition3f32( rect[i].x1, rect[i].y1, rect[i].z );
          GXPosition3f32( rect[i].x0, rect[i].y1, rect[i].z );
        GXEnd( );

        // Set texture color to go through PE stage and XOR'ed by blender.
        GXSetNumTexGens( 1 );     // # of Tex Gens
        GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
        GXSetTevOp( GX_TEVSTAGE0, GX_REPLACE );
        GXSetBlendMode( GX_BM_LOGIC, GX_BL_ONE, GX_BL_ZERO, GX_LO_XOR );
    
        // Draw tag to show z value as coplanar surface
#ifndef EMU
        GXSetCoPlanar( GX_TRUE );
#endif
        GXSetZMode( GX_TRUE, GX_EQUAL, GX_TRUE );
        DEMOPrintf( (s16)rect[i].x0, (s16)rect[i].y0,
                    (s16)rect[i].z,  "%d", (s32)rect[i].z );
#ifndef EMU
        GXSetCoPlanar( GX_FALSE );
#endif
    // Reset ZMode
        GXSetZMode( z->compare_enable, z->func, z->update_enable );
    }

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
    OSReport("************************************************\n");
    OSReport("pix-z-comp: test z compare modes\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
    OSReport("  Stick X/Y    : move the cursor, select options\n");
    OSReport("  A button     : zoom mode control\n");
    OSReport("X/Y button     : change z scale\n");
    OSReport("************************************************\n\n");
}


/*======== End of pix-z-comp.c ========*/
