/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     pix-fog.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/PixelProc/pix-fog.c $
    
    12    5/06/01 8:55p Hirose
    removed obsolete flags
    
    11    11/03/00 6:46p Hirose
    added parameter range clampping
    
    10    10/25/00 9:59p Hirose
    flag change. MAC -> EMU for PC emulator compatibility
    
    9     7/07/00 3:33p Carl
    Added fog-range tests.
    
    8     7/01/00 3:39p Alligator
    intro stuff
    
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
    
    9     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    8     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    7     1/25/00 2:56p Carl
    Changed to standardized end of test message
    
    6     1/25/00 12:15p Carl
    Fixed spelling error
    
    5     1/20/00 4:31p Carl
    Removed #ifdef EPPC stuff
    
    4     1/18/00 6:51p Carl
    Fixed aspect ratio problem with EPPC
    
    3     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/10/99 4:43p Carl
    Fixed Z compare stuff.
    
    21    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    20    11/03/99 7:21p Yasu
    Delete define of GXPosition2f32
    
    19    10/14/99 7:57p Yasu
    Change ViewPort structure to fit ViewPort/Scissor API
    
    18    10/13/99 4:32p Alligator
    change GXSetViewport, GXSetScissor to use xorig, yorig, wd, ht
    
    17    9/30/99 10:08p Yasu
    Renamed functions and enums
    
    16    9/28/99 6:54p Yasu
    Change parameter of DEMOInitCaption

    15    9/28/99 3:36a Yasu
    Change indent form
    Delete console out

    14    9/23/99 8:53p Yasu
    Change currentBuffer -> CurrentBuffer.

    13    9/23/99 7:15p Yasu
    Added single frame mode 2

    12    9/23/99 4:55p Yasu
    Made it available in single frame mode.
    Constant frame rate mode has deleted.
    Changed to use DEMOlib.

    11    9/14/99 6:00p Yasu
    Change to use DEMOPuts function set instead of cmPuts function set.

    10    9/10/99 6:36a Yasu
    Let animation going in 60Hz
    Change box cursor to more simple one

    9     9/07/99 9:52p Yasu
    Delete common section

    8     9/02/99 3:18p Ryan
    Made Frame Buffer Api changes

    7     8/27/99 9:19a Yasu
    Fixed data format

    6     8/27/99 1:26a Yasu
    Make it easier to handle a model data.

    5     8/26/99 7:56p Yasu
    Add GXInvalidateTexAll() and change UI.

    4     8/26/99 6:24p Yasu
    Change for one-shot mode

    3     8/26/99 2:41p Yasu
    Delete fog density controls.

    2     8/25/99 6:22p Yasu
    Add some test demos.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>       // for cosf(), sinf()
#include "cmn-model.h"

#define SCREEN_WD       1       // Dummy - actual value filled at runtime
#define SCREEN_HT       1       // Dummy - actual value filled at runtime
#define SCREEN_ZNEAR    0.0f    // near plane Z in screen coordinates
#define SCREEN_ZFAR     1.0f    // far  plane Z in screen coordinates
#define ZBUFFER_MAX     0x00ffffff

#ifdef  EMU
// Not implemented on emulator
#  define GX_FOG_REVEXP        GX_FOG_NONE
#  define GX_FOG_REVEXP2       GX_FOG_NONE
#endif

#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
   Typedefs
 *---------------------------------------------------------------------------*/
typedef struct
{
    char*       title;
    GXFogType   type;
    f32         startz;
    f32         endz;
    GXColor     color;
}   Fog;

typedef struct
{
    Model*      model;
    Camera*     camera;
    ViewPort*   viewport;
    Fog*        fog;
    u32         flag;
}   Scene;

#define SCENE_DRAWN     (1<<0)
#define SCENE_CURSOR    (1<<1)
#define SCENE_LCURSOR0  (1<<2)
#define SCENE_LCURSOR1  (1<<3)

typedef struct {                // for z coord tag data
  f32     x, y, z, dir;
  GXColor color;
} Obj1;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void    SceneDraw( Scene* );
void    SceneDrawInfo( Scene* );
void    SceneControl( DEMOPadStatus* );
static  void PrintIntro( void );

/*---------------------------------------------------------------------------*
   Rendering parameters
 *---------------------------------------------------------------------------*/
Camera  myCamera =
{
    { 0.0f,   0.0f, 30.0f },    // position
    { 0.0f,1000.0f,  0.0f },    // target
    { 0.0f,   0.0f,  1.0f },    // upVec
       33.3f,                   // fovy
       16.0f,                   // near plane Z in camera coordinates
     1024.0f,                   // far  plane Z in camera coordinates
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

Fog     myFog[] =
{
    // title      type            startz   endz    color
    { "NONE",     GX_FOG_NONE,      0.0f,    0.0f, {128,128,128,255} },
    { "LINEAR",   GX_FOG_LIN,      16.0f, 1024.0f, {128,128,128,255} },
#ifdef EMU
    { "EXP",      GX_FOG_EXP,       0.0f, 1024.0f, {128,128,128,255} },
    { "EXP2",     GX_FOG_EXP2,      0.0f, 1024.0f, {128,128,128,255} },
    { "REV-EXP",  GX_FOG_REVEXP,    0.0f,    0.0f, {128,128,128,255} },
    { "REV-EXP2", GX_FOG_REVEXP2,   0.0f,    0.0f, {128,128,128,255} },
#else
    { "EXP",      GX_FOG_EXP,      16.0f, 1024.0f, {128,128,128,255} },
    { "EXP2",     GX_FOG_EXP2,     16.0f, 1024.0f, {128,128,128,255} },
    { "REV-EXP",  GX_FOG_REVEXP,   16.0f, 1024.0f, {128,128,128,255} },
    { "REV-EXP2", GX_FOG_REVEXP2,  16.0f, 1024.0f, {128,128,128,255} },
#endif
};

GXColor sceneBgColor = {128,128,128,255};

Scene   myScene[] =
{
    { &cmModel[0], &myCamera, &myViewPort[0], &myFog[0], 0 },
    { &cmModel[0], &myCamera, &myViewPort[1], &myFog[1], 0 },
    { &cmModel[0], &myCamera, &myViewPort[2], &myFog[2], 0 },
    { &cmModel[0], &myCamera, &myViewPort[3], &myFog[3], 0 },
    { &cmModel[0], &myCamera, &myViewPort[4], &myFog[0], 0 },
};

#define NUMSCENE        (sizeof(myScene)/sizeof(Scene))

GXBool fogAdjust = GX_FALSE;

extern Obj1 *obj1;
extern Obj1 obj1_data[10];

Obj1 obj1_alt_data[10] = {
  {-291.0f, -40.0f,  500.0f,  1.0f, {255,255,255,255} },
  {-227.0f, -20.0f,  500.0f,  1.0f, {255,255,  0,255} },
  {-163.0f, -40.0f,  500.0f,  1.0f, {255,  0,255,255} },
  { -99.0f, -20.0f,  500.0f,  1.0f, {  0,255,255,255} },
  { -35.0f, -40.0f,  500.0f,  1.0f, {255,  0,  0,255} },
  {  33.0f, -20.0f,  500.0f,  1.0f, {  0,  0,255,255} },
  {  97.0f, -40.0f,  500.0f,  1.0f, {  0,255,  0,255} },
  { 161.0f, -20.0f,  500.0f,  1.0f, {255,255,128,255} },
  { 225.0f, -40.0f,  500.0f,  1.0f, {255,128,255,255} },
  { 289.0f, -20.0f,  500.0f,  1.0f, {128,255,255,255} },
};

Obj1 obj1_cir_data[10] = {
  { 0.0f, -41.0f, 0.0f,  1.0f, {255,255,255,255} },
  { 0.0f, -21.0f, 0.0f,  1.0f, {255,255,  0,255} },
  { 0.0f, -41.0f, 0.0f,  1.0f, {255,  0,255,255} },
  { 0.0f, -21.0f, 0.0f,  1.0f, {  0,255,255,255} },
  { 0.0f, -41.0f, 0.0f,  1.0f, {255,  0,  0,255} },
  { 0.0f, -21.0f, 0.0f,  1.0f, {  0,  0,255,255} },
  { 0.0f, -41.0f, 0.0f,  1.0f, {  0,255,  0,255} },
  { 0.0f, -21.0f, 0.0f,  1.0f, {255,255,128,255} },
  { 0.0f, -41.0f, 0.0f,  1.0f, {255,128,255,255} },
  { 0.0f, -21.0f, 0.0f,  1.0f, {128,255,255,255} },
};

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void    main ( void )
{
    GXRenderModeObj *rmp;
    u32   i;
    f32 ang;
    
    // initialize obj1 circle data
    for(i=0; i<10; i++)
    {
        ang = (i * 44.4f/10.0f - 44.4f*9.0f/20.0f) * 3.1415926535f/180.0f;
        obj1_cir_data[i].x = sinf(ang)*500.0f;
        obj1_cir_data[i].z = cosf(ang)*500.0f;
    }
    
    // initialize render settings and set clear color for first frame
    DEMOInit(NULL);
    GXInvalidateTexAll( );
    GXSetCopyClear( sceneBgColor, ZBUFFER_MAX );

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

    while( ! ( DEMOPadGetButton(0) & PAD_BUTTON_MENU ) )
    {
        // get pad status
        DEMOPadRead( );
        // General control & model animation
        SceneControl( &DemoPad[0] );
        cmModelAnime( &DemoPad[0], &myCamera );

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
/*---------------------------------------------------------------------------*
    Name:           SceneDraw
    Description:    Update the perspective and view matrices.
    Arguments:      Scene* s
    Returns:        none
 *---------------------------------------------------------------------------*/
void    SceneDraw( Scene* s )
{
    Camera*     c = s->camera;
    ViewPort*   v = s->viewport;
    Fog*        f = s->fog;
    float       aspect = (float) (4.0 / 3.0);
    GXFogAdjTable fogTable;

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
    GXSetScissor( (u32)v->xorg,  (u32)v->yorg, (u32)v->width, (u32)v->height );

    // Set Pixel Processing Mode
    GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );
    GXSetZMode( GX_TRUE, GX_LESS, GX_TRUE );

    // Set fog and clear color
    GXSetFog( f->type, f->startz, f->endz, c->znear, c->zfar, f->color );
    sceneBgColor = f->color;

    if (fogAdjust)
    {
        GXInitFogAdjTable(&fogTable, (u16) v->width, c->projMtx);
        GXSetFogRangeAdj(GX_ENABLE, (u16) (v->xorg + (v->width/2)), &fogTable);
    }
    else
    {
        GXSetFogRangeAdj(GX_DISABLE, 0, NULL);
    }

    // Draw model
    if ( s->model->draw ) (*s->model->draw)( c );

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
    Fog*        f = s->fog;
    s16         ypos;

    // Draw paramters to the window
    DEMOInitCaption( DM_FT_OPQ, v->width, v->height );
    DEMOPuts( 20, 12, 0, f->title );

    if (fogAdjust)
        DEMOPuts( 20, 20, 0, "Adjust" );
    
    ypos = 12;
    if ( f->startz > 0.0f )
    {
        DEMOPrintf( 96, 12, 0, "STARTZ %7.2f", f->startz );
        ypos += 8;
    }
    if ( f->endz   > 0.0f )
    {
         DEMOPrintf( 96, ypos, 0, "ENDZ   %7.2f", f->endz );
    }

    // Draw cursor for full screen
    if ( s->flag & SCENE_CURSOR   ) DEMOPuts(12,   12, 0, "\x7f" );
    if ( s->flag & SCENE_LCURSOR0 ) DEMOPuts(88,   12, 0, "\x7f" );
    if ( s->flag & SCENE_LCURSOR1 ) DEMOPuts(88, ypos, 0, "\x7f" );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           SceneControl
    Description:    user interface for paramter control
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
void    SceneControl( DEMOPadStatus* pad )
{
    static  s32 zoomMode = 0;
    static  s32 cursor   = 0;
    static  s32 range    = 0;
    static  s32 cursorL  = 0;
    static  f32 scale    = 1.0f;
    static  s32 modelId  = 1;
    s32     i;
    Fog*    f;
    Scene*  s;

    // clear flag
    cmModel[modelId].flag = 0;

    // change model
    if ( pad->buttonDown & PAD_BUTTON_B )
    {
        modelId = (modelId == 1) ? 2 : 1;
    }

    // set flag
    cmModel[modelId].flag = MODEL_REFERRED;
    for ( i = 0; i < NUMSCENE; i ++ )
    {
        myScene[i].model = &cmModel[modelId];
    }

    // object mode
    if ( modelId == 1 && pad->buttonDown & PAD_BUTTON_X )
    {
        if (obj1 == obj1_data) 
        {
            obj1 = obj1_alt_data;
        }
        else if (obj1 == obj1_alt_data)
        {
            obj1 = obj1_cir_data;
        }
        else 
        {
            obj1 = obj1_data;
        }
    }

    // fog adjust mode
    if ( pad->buttonDown & PAD_BUTTON_Y ) fogAdjust ^= 1;

    // zoom mode
    if ( pad->buttonDown & PAD_BUTTON_A ) zoomMode ^= 1;

    if ( zoomMode )
    {
        //
        // *** zoom mode
        //

        // show specified scene in full screen
        s = &myScene[cursor+1];
        f = s->fog;
        myScene[0].fog  = f;
        myScene[0].flag = SCENE_DRAWN;

        // turn off another window
        for ( i = 1; i < NUMSCENE; i ++ )
        {
            myScene[i].flag = 0;
        }

        // choose a parameter
        if ( pad->dirsNew & DEMO_STICK_UP )
        {
            if ( cursorL > 0 ) cursorL --;      // line cursor '>' move up
        }
        if ( pad->dirsNew & DEMO_STICK_DOWN )
        {
            if ( cursorL < 1 ) cursorL ++;      // line cursor '>' move down
        }

        // skip if invalid item
        if ( f->startz == 0.0f ) cursorL = 1;  // go to endz   if startz=0
        if ( f->endz   == 0.0f ) cursorL = 0;  // go to startz if endz=0

        // set cursor position
        if ( f->startz > 0.0f || f->endz > 0.0f )
        {
            myScene[0].flag |= (SCENE_LCURSOR0 << cursorL);
        }

        // get scale parameter (velocity)
        if      ( pad->dirs & DEMO_STICK_LEFT  ) scale *= 1.000002f;
        else if ( pad->dirs & DEMO_STICK_RIGHT ) scale /= 1.000002f;
        else                                     scale  = 1.0f;

        // scale clampping
        Clamp(scale, 1.0F/1.5F, 1.5F);

        // multiply scale to the parameter which specified by cursor.
        if ( cursorL == 0 )
        {
            f->startz *= scale;
            Clamp(f->startz, 0.10F, f->endz);
        }
        else
        {
            f->endz *= scale;
            Clamp(f->endz, f->startz, 8192.0F);
        }
    }
    else
    {
        //
        // *** catalog mode
        //

        // choose a scene.
        if ( pad->dirsNew & DEMO_STICK_LEFT )
        {
            if ( cursor <  2 ) range = 0;               // shift right
            cursor &= ~2;                               // go to left
        }
        if ( pad->dirsNew & DEMO_STICK_RIGHT )
        {
            if ( cursor >= 2 ) range = 2;               // shift left
            cursor |=  2;                               // go to right
        }
        if ( pad->dirsNew & DEMO_STICK_UP   ) cursor &= ~1; // up
        if ( pad->dirsNew & DEMO_STICK_DOWN ) cursor |=  1; // down

        // show 4 small windows
        for ( i = 1; i < 5; i ++ )
        {
            myScene[i].fog  = &myFog[i+range-1];
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
    OSReport("pix-fog: fog test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
    OSReport("  Stick X/Y    : move cursor, select option\n");
    OSReport("  A button     : toggle zoom mode\n");
    OSReport("  B button     : change model\n");
    OSReport("  X button     : change tags\n");
    OSReport("  Y button     : toggle range adjust\n");
    OSReport("  R trigger    : toggle animation\n");
    OSReport("************************************************\n\n");
}


/*======== End of pix-fog.c ========*/
