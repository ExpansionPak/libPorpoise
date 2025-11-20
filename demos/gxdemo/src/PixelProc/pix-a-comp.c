/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     pix-a-comp.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/PixelProc/pix-a-comp.c $
    
    10    5/06/01 8:55p Hirose
    removed obsolete flags
    
    9     7/07/00 5:57p Dante
    PC Compatibility
    
    8     7/01/00 3:04p Alligator
    add intro
    
    7     6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    6     5/17/00 9:05p Hirose
    fixed the last fix. After all Z mode is set in DEMOInitCaption
    
    5     5/17/00 8:06p Hirose
    deleted tab codes /
    set appropriate Z mode before calling DEMOPuts library because
    DEMOInitCaption doesn't set Z mode any more
    
    4     5/02/00 4:03p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    3     3/23/00 6:31p Hirose
    updated to use DEMOPad library instead of cmn-pad
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:11p Alligator
    move from tests/gx and rename
    
    11    2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    10    2/12/00 5:16p Alligator
    Integrate ArtX source tree changes
    
    9     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    8     1/31/00 1:27a Yasu
    Add GXSetZCompLoc
    
    7     1/25/00 2:56p Carl
    Changed to standardized end of test message
    
    6     1/25/00 12:15p Carl
    Fixed spelling error
    
    5     1/24/00 2:55p Carl
    Removed #ifdef EPPC stuff
    
    4     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    3     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/10/99 4:45p Carl
    Fixed Z compare stuff.
    
    8     11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    7     10/29/99 3:46p Hirose
    replaced GXSetTevStages(GXTevStageID) by GXSetNumTevStages(u8)
    
    6     10/22/99 7:42p Yasu
    Add GXSetTevStages and GXSetTevOrder
    
    5     10/14/99 7:57p Yasu
    Change ViewPort structure to fit ViewPort/Scissor API
    
    4     10/13/99 4:32p Alligator
    change GXSetViewport, GXSetScissor to use xorig, yorig, wd, ht
    
    3     9/30/99 10:08p Yasu
    Renamed functions and enums
    
    2     9/28/99 6:55p Yasu
    Change parameter of DEMOInitCaption

    1     9/28/99 3:34a Yasu
    Initial version
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include "cmn-model.h"

#define SCREEN_WD   1           // Dummy - actual value filled at runtime
#define SCREEN_HT   1           // Dummy - actual value filled at runtime
#define SCREEN_DEPTH    128.0f
#define SCREEN_ZNEAR    0.0f    // near plane Z in screen coordinates
#define SCREEN_ZFAR     1.0f    // far  plane Z in screen coordinates
#define ZBUFFER_MAX 0x00ffffff

/*---------------------------------------------------------------------------*
   Typedefs
 *---------------------------------------------------------------------------*/
typedef struct
{
    char*       title;
    // GXSetAlphaCompare
    GXCompare   comp0;
    u8          ref0;
    GXAlphaOp   op;
    GXCompare   comp1;
    u8          ref1;
    // GXSetAlphaUpdate
    GXBool      update_enable;
}   AlphaMode;

typedef struct
{
//  Model*  model;
    Camera*     camera;
    ViewPort*   viewport;
    AlphaMode*  alphamode;
    u32         flag;
}   Scene;

#define SCENE_DRAWN     (1<<0)
#define SCENE_CURSOR    (1<<1)
#define SCENE_LCURSOR0  (1<<2)
#define SCENE_LCURSOR1  (1<<3)
#define SCENE_LCURSOR2  (1<<4)
#define SCENE_LCURSOR3  (1<<5)
#define SCENE_LCURSOR4  (1<<6)

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void    SceneDraw( Scene* );
void    SceneDrawInfo( Scene* );
void    SceneControl( void );
void    myDrawModel( AlphaMode*, s32, s32 );
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

AlphaMode myAlphaMode[] =
{
//   Title  comp0       ref0  op          comp1      ref1  update_enable
    { NULL, GX_ALWAYS,  60,   GX_AOP_AND, GX_ALWAYS, 40,   GX_TRUE },
    { NULL, GX_LESS,    60,   GX_AOP_AND, GX_ALWAYS, 40,   GX_TRUE },
    { NULL, GX_GREATER, 60,   GX_AOP_AND, GX_ALWAYS, 40,   GX_TRUE },
    { NULL, GX_EQUAL,   60,   GX_AOP_AND, GX_ALWAYS, 40,   GX_TRUE },
};

GXColor sceneBgColor = { 32, 32, 128,255};

Scene   myScene[] =
{
    { /*&cmModel[1],*/ &myCamera, &myViewPort[0], &myAlphaMode[0], 0 },
    { /*&cmModel[1],*/ &myCamera, &myViewPort[1], &myAlphaMode[0], 0 },
    { /*&cmModel[1],*/ &myCamera, &myViewPort[2], &myAlphaMode[1], 0 },
    { /*&cmModel[1],*/ &myCamera, &myViewPort[3], &myAlphaMode[2], 0 },
    { /*&cmModel[1],*/ &myCamera, &myViewPort[4], &myAlphaMode[3], 0 },
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
    GXCompare   comp;
    char*       fmt;
}   AlphaComp;

AlphaComp   myAlphaComp[] =
{
 { GX_NEVER,      "A:NEVER" },
 { GX_LESS,       "A < %3d" },
 { GX_LEQUAL,     "A <=%3d" },
 { GX_EQUAL,      "A ==%3d" },
 { GX_NEQUAL,     "A !=%3d" },
 { GX_GEQUAL,     "A >=%3d" },
 { GX_GREATER,    "A > %3d" },
 { GX_ALWAYS,     "A:ALWYS" },
};

typedef struct
{
    GXAlphaOp   op;
    char*   name;
}   AlphaOps;

AlphaOps    myAlphaOps[] =
{
 { GX_AOP_AND,    "AND "   },
 { GX_AOP_OR,     "OR  "   },
 { GX_AOP_XOR,    "XOR "   },
 { GX_AOP_XNOR,   "XNOR"   },
};

#define NUMALPHACOMPS   (sizeof(myAlphaComp)/sizeof(AlphaComp))
#define NUMALPHAOPS (sizeof(myAlphaOps )/sizeof(AlphaOps ))

/*---------------------------------------------------------------------------*
    Name:           SceneDraw
    Description:    Draw model
    Arguments:      Scene* s
    Returns:        none
 *---------------------------------------------------------------------------*/
void    SceneDraw( Scene* s )
{
    ViewPort*   v = s->viewport;
    AlphaMode*  a = s->alphamode;
	GXColor 	black = {0, 0, 0, 0};

    // Check if drawn flag
    if ( !(s->flag & SCENE_DRAWN) ) return;

    // Set Viewport
    GXSetViewport( v->xorg, v->yorg, v->width, v->height,
                   SCREEN_ZNEAR, SCREEN_ZFAR );
    GXSetScissor( (u32)v->xorg, (u32)v->yorg, (u32)v->width, (u32)v->height );

    // Pixel processing mode
    GXSetFog( GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, black );
    GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );

    // Set Z Comp Location (after Alpha compare)
    GXSetZCompLoc( GX_FALSE );

    // Set Alpha Mode
    GXSetAlphaCompare( a->comp0, a->ref0, a->op, a->comp1, a->ref1 );
    GXSetAlphaUpdate( a->update_enable );

    // Draw objects
    myDrawModel(a, (s32)v->width, (s32)v->height );

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
void    SceneDrawInfo( Scene* s )
{
    Camera*     c = s->camera;
    ViewPort*   v = s->viewport;
    AlphaMode*  a = s->alphamode;
    s32         lid0, lid1, oid;

    // Temporary settings for drawing captions
    GXSetAlphaCompare( GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0 );
    
    // Draw paramters to the window
    DEMOInitCaption( DM_FT_XLU, v->width, v->height );
    if ( a->title )
    {
        DEMOPuts( 10, 2, 0, a->title );
    }
    else
    {
        // get alpha comp id and alpha op id
        for ( lid0 = NUMALPHACOMPS-1; lid0 > 0; lid0 -- )
        {
            if ( a->comp0 == myAlphaComp[lid0].comp ) break;
        }
        for ( lid1 = NUMALPHACOMPS-1; lid1 > 0; lid1 -- )
        {
            if ( a->comp1 == myAlphaComp[lid1].comp ) break;
        }
        for ( oid = NUMALPHAOPS-1; oid > 0; oid -- )
        {
            if ( a->op == myAlphaOps[oid].op ) break;
        }
        DEMOPrintf( 10, 12, 0, myAlphaComp[lid0].fmt, a->ref0 );
        DEMOPuts  ( 74, 12, 0, myAlphaOps[oid].name );
        DEMOPrintf(114, 12, 0, myAlphaComp[lid1].fmt, a->ref1 );
    }

    // Draw cursor
    if ( s->flag & SCENE_CURSOR   ) DEMOPuts(   2, 12, 0, "\x7f" );
    if ( s->flag & SCENE_LCURSOR0 ) DEMOPuts(  26, 20, 0, "^" );
    if ( s->flag & SCENE_LCURSOR1 ) DEMOPuts(  58, 20, 0, "^" );
    if ( s->flag & SCENE_LCURSOR2 ) DEMOPuts(  74, 20, 0, "^" );
    if ( s->flag & SCENE_LCURSOR3 ) DEMOPuts( 130, 20, 0, "^" );
    if ( s->flag & SCENE_LCURSOR4 ) DEMOPuts( 170, 20, 0, "^" );

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
    static  s32 cursorL  = 0;
    s32         i, lid;
    Scene*      s;
    AlphaMode*  a;
    GXCompare*  comp;
    u8*         ref;
    u16         buttons, stickDirs;

    buttons   = DEMOPadGetButtonDown(0);
    stickDirs = DEMOPadGetDirsNew(0);

    // zoom mode
    if ( buttons & PAD_BUTTON_A ) zoomMode ^= 1;

    if ( zoomMode )
    {
        //
        // *** zoom mode
        //

        // show specified scene in full screen
        s = &myScene[cursor+1];
        a = s->alphamode;
        myScene[0].alphamode = a;
        myScene[0].flag      = SCENE_DRAWN;

        // turn off another window
        for ( i = 1; i < NUMSCENE; i ++ )
        {
            myScene[i].flag = 0;
        }

        // move cursor
        if ( stickDirs & DEMO_STICK_LEFT )
        {
            if ( cursorL > 0 )
            {
                cursorL --;
                // If ALWAYS or NEVER, skip reference setting 0
                if ( cursorL == 1 )
                {
                    if ( a->comp0 == GX_NEVER || a->comp0 == GX_ALWAYS )
                    {
                        cursorL = 0;
                    }
                }
            }
        }
        if ( stickDirs & DEMO_STICK_RIGHT )
        {
            if ( cursorL < 4 )
            {
                cursorL ++;
                // If ALWAYS or NEVER, cannot go reference setting 1
                if ( cursorL == 4 )
                {
                    if ( a->comp1 == GX_NEVER || a->comp1 == GX_ALWAYS )
                    {
                        cursorL = 3;
                    }
                }
                // If ALWAYS or NEVER, skip reference setting 0
                else if ( cursorL == 1 )
                {
                    if ( a->comp0 == GX_NEVER || a->comp0 == GX_ALWAYS )
                    {
                        cursorL = 2;
                    }
                }
            }
        }
        myScene[0].flag |= (SCENE_LCURSOR0 << cursorL);

        // get alpha mode id and ops id
        switch ( cursorL )
        {
          case    0:
          case    3:
            // comp0 or comp1
            comp = ( cursorL == 0 ) ? &(a->comp0) : &(a->comp1);
            for ( lid = NUMALPHACOMPS-1; lid > 0; lid -- )
            {
                if ( *comp == myAlphaComp[lid].comp ) break;
            }
            if ( stickDirs & DEMO_STICK_UP )
            {
                if ( lid > 0 ) lid --;
            }
            if ( stickDirs & DEMO_STICK_DOWN )
            {
                if ( lid < NUMALPHACOMPS-1 ) lid ++;
            }
            (*comp) = myAlphaComp[lid].comp;
            break;
    
          case    1:
          case    4:
            // ref0 or ref1
            ref = ( cursorL == 1 ) ? &(a->ref0) : &(a->ref1);
            if ( stickDirs & DEMO_STICK_UP )
            {
                if ( (*ref) <= 250 ) (*ref) += 5;
            }
            if ( stickDirs & DEMO_STICK_DOWN )
            {
                if ( (*ref) >=   5 ) (*ref) -= 5;
            }
            break;
    
          case    2:
            // op
            for ( lid = NUMALPHAOPS-1; lid > 0; lid -- )
            {
                if ( a->op == myAlphaOps[lid].op ) break;
            }
            if ( stickDirs & DEMO_STICK_UP )
            {
                if ( lid > 0 ) lid --;
            }
            if ( stickDirs & DEMO_STICK_DOWN )
            {
                if ( lid < NUMALPHAOPS-1 ) lid ++;
            }
            a->op = myAlphaOps[lid].op;
            break;
    
        default:
            // Never reseached here
            break;
        }
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
  {   30, 20,170,180, 50, {  64, 64,255, 50 }   },
  {   10, 40,120, 90, 60, { 255, 64, 64, 60 }   },
  {   80,110,190,160, 25, {  64,255, 64, 25 }   },
  {  110, 10,160,120, 75, { 255,255, 64, 75 }   },
  {   40, 80, 90,190, 40, {  64,255,255, 40 }   },
  {   70, 70,130,130, 50, { 255,255,255, 50 }   },
};

#define NUMRECTS    (sizeof(rect)/sizeof(myModel))

/*---------------------------------------------------------------------------*
    Name:           myDrawModel
    Description:    draw model
    Arguments:      f32 width : width  of viewport
                    f32 height: height of viewport
    Returns:        none
 *---------------------------------------------------------------------------*/
void    myDrawModel(AlphaMode *a, s32 width, s32 height )
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

    // set several mode
    GXSetCullMode( GX_CULL_BACK );      // Cull mode

    // Load tags in point sample mode
    DEMOLoadFont( GX_TEXMAP0, GX_TEXMTX0, DMTF_POINTSAMPLE );

    // Here, use orthographic screen coordinates.
    // Set normalized 0-200 screen coordinates and reduce z range in MTX0
    // Set vertex descripter on VTXFMT1
    DEMOSetupScrnSpc( width, height, SCREEN_DEPTH );
    MTXScale( mtx, ((float)width)/200.0f, ((float)height)/200.f, 1.0f );
    GXLoadPosMtxImm( mtx, GX_PNMTX0 );
    GXSetVtxAttrFmt( GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );

    for ( i = 0; i < NUMRECTS; i ++ )
    {
        // Set material color to go through PE stage.
        GXSetNumChans( 1 );
        GXSetNumTexGens( 0 );       // # of Tex Gens
        GXSetNumTevStages( 1 );     // # of Tev Stages
        GXSetChanMatColor( GX_COLOR0A0, rect[i].color );
        GXSetTevOrder( GX_TEVSTAGE0,
                       GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
        GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
        GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );
    
        // Fill rectangle
        GXSetZMode( GX_TRUE, GX_ALWAYS, GX_TRUE );
        GXClearVtxDesc( );
        GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
        GXBegin( GX_QUADS, GX_VTXFMT1, 4 );
          GXPosition3f32( rect[i].x0, rect[i].y0, rect[i].z );
          GXPosition3f32( rect[i].x1, rect[i].y0, rect[i].z );
          GXPosition3f32( rect[i].x1, rect[i].y1, rect[i].z );
          GXPosition3f32( rect[i].x0, rect[i].y1, rect[i].z );
        GXEnd( );
    
        // Set texture color to go through PE stage and XOR'ed by blender.
        GXSetNumTexGens( 1 );
        GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
        GXSetTevOp( GX_TEVSTAGE0, GX_REPLACE );
        GXSetBlendMode( GX_BM_LOGIC, GX_BL_ONE, GX_BL_ZERO, GX_LO_XOR );
    
        // Draw tag to show z value as coplanar surface
#ifndef EMU
        GXSetCoPlanar( GX_TRUE );
#endif
        GXSetAlphaCompare( GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0 );
        GXSetZMode( GX_TRUE, GX_EQUAL, GX_TRUE );
        DEMOPrintf( (s16)rect[i].x0, (s16)rect[i].y0,
                    (s16)rect[i].z,  "%d", (s32)rect[i].z );
#ifndef EMU
        GXSetCoPlanar( GX_FALSE );
#endif
        GXSetAlphaCompare( a->comp0, a->ref0, a->op, a->comp1, a->ref1 );
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
    OSReport("pix-a-comp: alpha compare test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
    OSReport("  Stick X/Y    : move cursor\n");
    OSReport("  A button     : toggle zoom mode\n");
    OSReport("************************************************\n\n");
}


/*======== End of pix-a-comp.c ========*/
