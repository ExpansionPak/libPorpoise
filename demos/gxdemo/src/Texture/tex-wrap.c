/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-wrap.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-wrap.c $    
    
    8     5/11/01 1:20a Hirose
    removed obsolete flags. fixed some verify messages.
    
    7     3/26/00 7:32p Hirose
    centered display object more considering H/V overscan again
    
    6     3/24/00 6:55p Carl
    Fixed borders that went away due to the last change.
    
    5     3/24/00 6:24p Carl
    Adjusted for overscan.
    
    4     3/24/00 4:57p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    5     2/25/00 3:45p Hirose
    updated messages and copyright
    
    4     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    3     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    10    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    9     11/15/99 4:49p Yasu
    Change datafile name
    
    8     10/12/99 10:19a Hirose
    a small bug fix
    
    7     10/07/99 10:07a Hirose
    
    6     10/06/99 5:52p Hirose
    added single frame test / changed some structures
    
    5     9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    4     9/20/99 11:51a Ryan
    update to use new DEMO calls
    
    3     9/15/99 5:51p Hirose
    changed to use new tpl file
    
    2     9/14/99 7:00p Hirose

    1     9/13/99 5:03p Hirose
    created (copied tex-test01)
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-wrap
        texture wrap mode test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define Clamp(val,min,max) \
    ((val) = (((val)<(min)) ? (min) : ((val)>(max)) ? (max) : (val)))

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
    MyCameraObj       cam;
    u32               texNumber;
    f32               tcScaleX;
    f32               tcScaleY; 
    GXTexFilter       filterMode;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main           ( void );
static void DrawInit       ( MySceneCtrlObj* sc );
static void DrawTick       ( MySceneCtrlObj* sc );
static void AnimTick       ( MySceneCtrlObj* sc );
static void DrawQuad       ( void );
static void SetCamera      ( MyCameraObj* cam );
static void PrintIntro     ( void );

/*---------------------------------------------------------------------------*
   Parameters
 *---------------------------------------------------------------------------*/
static GXTexWrapMode WrapModeTable[] =
{
    GX_REPEAT,
    GX_CLAMP,
    GX_MIRROR
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 500.0F }, // location
    { 0.0F, 1.0F,   0.0F }, // up
    { 0.0F, 0.0F,   0.0F }, // tatget
    -320.0F, // left
    224.0F,  // top  (note: was 240, now adjusted for overscan)
    50.0F,   // near
    2000.0F  // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;          // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;    // texture palette
static u32              TexNumMax;          // number of textures

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);  // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl); // Initialize vertex formats, array pointers
                          // and default scene settings.

    PrintIntro();    // Print demo directions

    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);    // Draw the model.
        DEMODoneRender();
        DEMOPadRead();           // Read controller
        AnimTick(&SceneCtrl);    // Do animation
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format and sets up
                    the array pointer for the indexed data.
                    This function also initializes scene control parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{
    //  Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    //  Load TPL file
    TEXGetPalette(&MyTplObj, "gxTests/tex-03.tpl");
    TexNumMax = MyTplObj->numDescriptors;

    // Texture coord generation setting
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetNumTexGens(1);


    // Default scene control parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // texture number
    sc->texNumber = 0;
    
    // texture coord scale and filter
    sc->tcScaleX = 1.0F;
    sc->tcScaleY = 1.0F;
    sc->filterMode = GX_NEAR;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    TEXDescriptorPtr tdp;
    GXTexObj         texObj;
    GXTexFilter      minFilter, magFilter;
    GXBool           mipMapFlag;
    Mtx              mt, mv;
    s32              x, y;
    f32              tcx, tcy;

    // use texture in Tev
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

    // get texture descriptor from texture palette
    tdp = TEXGet(MyTplObj, sc->texNumber);
    
    // set up texture filters
    mipMapFlag =
        ( tdp->textureHeader->minLOD == tdp->textureHeader->maxLOD ) ?
        GX_FALSE : GX_TRUE;

    magFilter = sc->filterMode;
    if ( mipMapFlag )
    {
        minFilter = ( sc->filterMode == GX_NEAR ) ?
                    GX_NEAR_MIP_LIN : GX_LIN_MIP_LIN;
    }
    else
    {
        minFilter = sc->filterMode;
    }


    // display 3x3 panels
    for ( y = 0 ; y < 3 ; ++y )
    {
        for ( x = 0 ; x < 3 ; ++x )
        {
            GXInitTexObj(
                &texObj,
                tdp->textureHeader->data,
                tdp->textureHeader->width,
                tdp->textureHeader->height,
                (GXTexFmt)tdp->textureHeader->format,
                WrapModeTable[x], // s
                WrapModeTable[y], // t
                mipMapFlag );     // Mipmap

            GXInitTexObjLOD(
                &texObj,
                minFilter,
                magFilter,
                tdp->textureHeader->minLOD,
                tdp->textureHeader->maxLOD,
                tdp->textureHeader->LODBias,
                GX_FALSE,
                tdp->textureHeader->edgeLODEnable,
                GX_ANISO_1 );

            // Load texture obj
            GXLoadTexObj(&texObj, GX_TEXMAP0);
    
            // Set modelview matrix
            MTXTrans(mt, (f32)(x-1)*180, (f32)(y-1)*136, 0.0F);
            MTXConcat(sc->cam.view, mt, mv);
            GXLoadPosMtxImm(mv, GX_PNMTX0);
            
            // Set texcoord matrix
            tcx = (f32)(sc->tcScaleX * 2.0F + 1.0F);
            tcy = (f32)(sc->tcScaleY * 2.0F + 1.0F);
            MTXScale(mv, tcx, tcy, 1.0F);
            MTXTrans(mt, -(sc->tcScaleX), -(sc->tcScaleY), 0.0F);
            MTXConcat(mt, mv, mv);
            GXLoadTexMtxImm(mv, GX_TEXMTX0, GX_MTX2x4);

            // Draw a textured matrix
            DrawQuad();
        }
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
    u16 down = DEMOPadGetButtonDown(0);

    // Texture coordinate scale control
    sc->tcScaleX += 0.01F * (f32)(DEMOPadGetStickX(0)) / 32.0F;
    sc->tcScaleY -= 0.01F * (f32)(DEMOPadGetStickY(0)) / 32.0F;
    Clamp(sc->tcScaleX, 0.0F, 10.0F);
    Clamp(sc->tcScaleY, 0.0F, 10.0F);
    
    // Reset coordinates
    if ( down & PAD_BUTTON_X )
    {
        sc->tcScaleX = sc->tcScaleY = 1.0F;
    }

    // Change the texture
    if ( down & PAD_BUTTON_A )
    {
        sc->texNumber = ( sc->texNumber + 1 ) % TexNumMax;
    }

    // Change filter mode
    if ( down & PAD_BUTTON_B )
    {
        sc->filterMode = ( sc->filterMode == GX_NEAR ) ?
        GX_LINEAR : GX_NEAR;
    }

}

/*---------------------------------------------------------------------------*
    Name:           DrawQuad
    
    Description:    Draw a textured quad.
 *---------------------------------------------------------------------------*/
static void DrawQuad( void )
{
    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // draw the box
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16(-64,  64, 0);
        GXTexCoord2f32(0.0F, 0.0F);
        GXPosition3s16( 64,  64, 0);
        GXTexCoord2f32(1.0F, 0.0F);
        GXPosition3s16( 64, -64, 0);
        GXTexCoord2f32(1.0F, 1.0F);
        GXPosition3s16(-64, -64, 0);
        GXTexCoord2f32(0.0F, 1.0F);
    GXEnd();
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

    MTXOrtho(
        cam->proj,
        cam->cfg.top,
        - (cam->cfg.top),
        cam->cfg.left,
        - (cam->cfg.left),
        cam->cfg.znear,
        cam->cfg.zfar );
    GXSetProjection(cam->proj, GX_ORTHOGRAPHIC);
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
    OSReport("tex-wrap: texture wrap mode test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick : change scale of texture coordinates\n");
    OSReport("A button   : change the texture\n");
    OSReport("B button   : switch NEAR/LINEAR filters\n");
    OSReport("X button   : reset texture coordinates\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
