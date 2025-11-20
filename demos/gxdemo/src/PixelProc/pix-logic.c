/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     pix-logic.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/PixelProc/pix-logic.c $
    
    12    5/06/01 8:55p Hirose
    removed obsolete flags
    
    11    10/30/00 1:11p Carl
    Fixed error in setting blend mode.
    
    10    10/17/00 5:34p Hirose
    added subtract mode test for GX revision 2
    
    9     7/07/00 5:57p Dante
    PC Compatibility
    
    8     7/01/00 3:29p Alligator
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
    
    8     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    7     1/25/00 2:56p Carl
    Changed to standardized end of test message
    
    6     1/25/00 12:15p Carl
    Fixed spelling error
    
    5     1/20/00 4:37p Carl
    Removed #ifdef EPPC stuff
    
    4     1/18/00 6:53p Carl
    Fixed aspect ratio problem with EPPC
    
    3     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/10/99 4:44p Carl
    Fixed Z compare stuff.
    
    12    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    11    10/14/99 7:57p Yasu
    Change ViewPort structure to fit ViewPort/Scissor API
    
    10    10/13/99 4:32p Alligator
    change GXSetViewport, GXSetScissor to use xorig, yorig, wd, ht
    
    9     9/30/99 10:08p Yasu
    Renamed functions and enums
    
    8     9/28/99 6:55p Yasu
    Change parameter of DEMOInitCaption

    7     9/28/99 3:36a Yasu
    Change indent form.
    Delete console-out

    6     9/23/99 8:54p Yasu
    Change currentBuffer -> CurrentBuffer
    Add EPPC screen size definition

    5     9/23/99 7:14p Yasu
    Added single frame mode

    4     9/23/99 5:39p Yasu
    fixed small bug

    3     9/23/99 4:55p Yasu
    Made it available in single frame mode.
    Constant frame rate mode has deleted.
    Changed to use DEMOlib.

    2     9/14/99 6:01p Yasu
    Change cmPuts function set to DEMOPuts funtcion set

    1     9/10/99 6:30a Yasu
    Initial version
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include "cmn-model.h"

#define SCREEN_WD       1       // Dummy - actual value filled at runtime
#define SCREEN_HT       1       // Dummy - actual value filled at runtime
#define SCREEN_ZNEAR    0.0f    // near plane Z in screen coordinates
#define SCREEN_ZFAR     1.0f    // far  plane Z in screen coordinates
#define ZBUFFER_MAX     0x00ffffff

#ifdef  EMU
// Not implemented on emulator
#  define GXPosition2s16(x,y)   GXPosition3s16((x),(y),(s16)0)
#  define GX_POS_XY             GX_POS_XYZ
#endif

/*---------------------------------------------------------------------------*
   Typedefs
 *---------------------------------------------------------------------------*/
typedef struct
{
    char*               title;
    GXBlendMode type;
    GXBlendFactor               sfactor;
    GXBlendFactor               dfactor;
    GXLogicOp           logic;
}   Blend;

typedef struct
{
    Model*      model;
    Camera*     camera;
    ViewPort*   viewport;
    Blend*      blend;
    u32         flag;
}   Scene;

#define SCENE_DRAWN     (1<<0)
#define SCENE_CURSOR    (1<<1)

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

Blend   myBlend[] =
{
 { NULL, GX_BM_LOGIC, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY    },
 { NULL, GX_BM_LOGIC, GX_BL_ONE, GX_BL_ZERO, GX_LO_INVCOPY },
 { NULL, GX_BM_LOGIC, GX_BL_ONE, GX_BL_ZERO, GX_LO_XOR     },
 { NULL, GX_BM_LOGIC, GX_BL_ONE, GX_BL_ZERO, GX_LO_AND     },
};

GXColor sceneBgColor = { 32, 32, 128,255};

Scene   myScene[] =
{
    { &cmModel[3], &myCamera, &myViewPort[0], &myBlend[0], 0 },
    { &cmModel[3], &myCamera, &myViewPort[1], &myBlend[0], 0 },
    { &cmModel[3], &myCamera, &myViewPort[2], &myBlend[1], 0 },
    { &cmModel[3], &myCamera, &myViewPort[3], &myBlend[2], 0 },
    { &cmModel[3], &myCamera, &myViewPort[4], &myBlend[3], 0 },
};

#define NUMSCENE        (sizeof(myScene)/sizeof(Scene))

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
typedef struct
{
    GXLogicOp   ops;
    char*       name;
}   Logic;

#if ( GX_REV != 1 ) // Revision 2 or later
// Bogus id number to test non-logic (subtract) mode
#define GX_LO_XXX   (GXLogicOp)0xFFFFFFFF
#endif

Logic   myLogic[] = {
 { GX_LO_CLEAR,   "CLEAR    0"            },
 { GX_LO_SET,     "SET      1"            },
 { GX_LO_COPY,    "COPY     Src"          },
 { GX_LO_INVCOPY, "INVCOPY  ~Src"         },
 { GX_LO_NOOP,    "NOOP     Dst"          },
 { GX_LO_INV,     "INV      ~Dst"         },
 { GX_LO_AND,     "AND      Src & Dst"    },
 { GX_LO_NAND,    "NAND     ~(Src & Dst)" },
 { GX_LO_OR,      "OR       Src | Dst"    },
 { GX_LO_NOR,     "NOR      ~(Src & Dst)" },
 { GX_LO_XOR,     "XOR      Src ^ Dst"    },
 { GX_LO_EQUIV,   "EQUIV    ~(Src ^ Dst)" },
 { GX_LO_REVAND,  "REVAND   Src & ~Dst"   },
 { GX_LO_INVAND,  "INVAND   ~Src ^ Dst"   },
 { GX_LO_REVOR,   "REVOR    Src | ~Dst"   },
 { GX_LO_INVOR,   "INVOR    ~Src | Dst"   },

#if ( GX_REV != 1 ) // Revision 2 or later
 { GX_LO_XXX,     "SUB      Dst - Src"    },    // subtract mode test
#endif
};

#define NUMLOGICS       (sizeof(myLogic)/sizeof(Logic))

/*---------------------------------------------------------------------------*
    Name:           SceneDrawBegin
    Description:    Update the perspective and view matrices.
    Arguments:      Scene* s
    Returns:        none
 *---------------------------------------------------------------------------*/
void    SceneDraw( Scene* s )
{
    Camera*     c = s->camera;
    ViewPort*   v = s->viewport;
    Blend*      b = s->blend;
    float       aspect = (float) (4.0 / 3.0);
	GXColor     black = {0, 0, 0, 0};

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

    // Set Pixel Pocessing Mode
    GXSetFog( GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, black );
    GXSetZMode( GX_TRUE, GX_LESS, GX_TRUE );

    // Set blend
    if ( b->logic >= GX_LO_CLEAR && b->logic <= GX_LO_SET )
        GXSetBlendMode( b->type, b->sfactor, b->dfactor, b->logic );
    else
        GXSetBlendMode( b->type, b->sfactor, b->dfactor, GX_LO_CLEAR );

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
    Blend*      b = s->blend;
    s32         lid;

    // Draw paramters to the window
    DEMOInitCaption( DM_FT_OPQ, v->width, v->height );
    if ( b->title )
    {
        DEMOPuts( 20, 12, 0, b->title );
    }
    else
    {
        // get logic id
        for ( lid = NUMLOGICS-1; lid > 0; lid -- )
        {
            if ( b->logic == myLogic[lid].ops ) break;
        }
        DEMOPuts( 20, 12, 0, myLogic[lid].name );
    }

    // Draw cursor
    if ( s->flag & SCENE_CURSOR ) DEMOPuts( 12, 12,0, "\x7f" );

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
    static  u32 modelId  = 3;

    s32     i, lid;
    Scene*  s;
    Blend*  b;

#if 0
    //  This test uses only 1 kind of model.
    //  Following code then has no meaning.

    // clear flag
    cmModel[modelId].flag = 0;

    // change model
    if ( pad->buttonDown & PAD_BUTTON_B )
    {
    }

    // set flag
    for ( i = 0; i < NUMSCENE; i ++ )
    {
        myScene[i].model = &cmModel[modelId];
    }
#endif

    //  Validate animation
    cmModel[modelId].flag = MODEL_REFERRED;

    // zoom mode
    if ( pad->buttonDown & PAD_BUTTON_A ) zoomMode ^= 1;

    if ( zoomMode )
    {
        //
        // *** zoom mode
        //

        // show specified scene in full screen and put cursor
        s = &myScene[cursor+1];
        b = s->blend;
        myScene[0].blend = b;
        myScene[0].flag  = SCENE_DRAWN | SCENE_CURSOR;

        // turn off another window
        for ( i = 1; i < NUMSCENE; i ++ )
        {
            myScene[i].flag = 0;
        }

        // get logic id
        for ( lid = NUMLOGICS-1; lid > 0; lid -- )
        {
            if ( b->logic == myLogic[lid].ops ) break;
        }

        // change blend parameter
        if ( pad->dirsNew & ( DEMO_STICK_LEFT  | DEMO_STICK_UP   ) )
        {
            if ( lid > 0 ) lid --;
        }
        if ( pad->dirsNew & ( DEMO_STICK_RIGHT | DEMO_STICK_DOWN ) )
        {
            if ( lid < NUMLOGICS-1 ) lid ++;
        }

        // set parameter
        b->logic = myLogic[lid].ops;

#if ( GX_REV != 1 ) // Revision 2 or later

        // the last bogus id is used for testing subtract mode
        if ( b->logic == GX_LO_XXX )
        {
            b->type = GX_BM_SUBTRACT;
        }
        else
        {
            b->type = GX_BM_LOGIC;
        }

#endif // ( GX_REV != 1 )

    }
    else
    {
        //
        // *** catalog mode
        //

        // choose a scene.
        if ( pad->dirsNew & DEMO_STICK_LEFT  ) cursor &= ~2; // left
        if ( pad->dirsNew & DEMO_STICK_RIGHT ) cursor |=  2; // right
        if ( pad->dirsNew & DEMO_STICK_UP    ) cursor &= ~1; // up
        if ( pad->dirsNew & DEMO_STICK_DOWN  ) cursor |=  1; // down

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
    OSReport("pix-logic: logic-ops test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
    OSReport("  Stick X/Y    : move cursor, change option\n");
    OSReport("  A button     : toggle zoom mode\n");
    OSReport("************************************************\n\n");
}


/*======== End of pix-logic.c ========*/
