/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tf-reflect.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Transform/tf-reflect.c $
    
    12    7/23/01 9:04p Hirose
    Fixed MTXRotAxis -> MTXRotAxisDeg.
    
    11    5/10/01 5:07a Hirose
    removed obsolete flags
    
    10    10/25/00 9:59p Hirose
    flag change. MAC -> EMU for PC emulator compatibility
    
    9     7/18/00 2:19p Tian
    Added call to GXSetResetWritePipe
    
    8     7/07/00 5:57p Dante
    PC Compatibility
    
    7     6/14/00 4:04p Carl
    Put in fix for ind mtx load bug.
    
    6     6/13/00 11:31a Hirose
    Fixed compiler warnings
    
    5     6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    4     3/24/00 5:46p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:14p Alligator
    move from tests/gx and rename
    
    8     3/02/00 5:36p Alligator
    ifdef for BUG_TRIANGLE_FAN
    
    7     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    6     1/25/00 3:00p Carl
    Changed to standardized end of test message
    
    5     1/25/00 12:18p Carl
    Fixed spelling error
    
    4     1/24/00 6:58p Carl
    Removed #ifdef EPPC stuff
    
    3     1/13/00 8:56p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/13/99 6:40p Yasu
    Delete GXSetZUpdate
    
    1     12/12/99 5:45a Yasu
    Test for Texture Matrix Index
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define VP_ASPECT   ((float)4/(float)3)

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void  DrawScene( void );
void  myDrawModel( void );
void  myAnimeModel( void );

/*---------------------------------------------------------------------------*
   Rendering parameters
 *---------------------------------------------------------------------------*/
typedef struct
{
    Point3d   position;
    Point3d   target;
    Vec       up;
    f32       fovy;
    f32       znear;
    f32       zfar;
    Mtx       viewMtx;
}   Camera;

Camera  myCamera =
{
    { 0.0f, -400.0f, 0.0f },   // position
    { 0.0f,    0.0f, 0.0f },   // target
    { 0.0f,    0.0f, 1.0f },   // upVec
       33.3f,                   // fovy
      128.0f,                   // near plane Z in camera coordinates
     2048.0f,                   // far  plane Z in camera coordinates
};

TEXPalettePtr   texPalette = 0;
GXTexObj        texObj;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void  main ( void )
{
	GXColor grey = {128,128,128,0};

    // initialize render settings and set clear color for first frame
    DEMOInit( NULL );  // Defined in $DOLPHIN/build/libraries/demo/src/DEMOInit.c
    GXInvalidateTexAll( );
    GXSetCopyClear( grey, GX_MAX_Z24 );

#ifdef BUG_INDX_ALIGN    
    // This program depends on specific command alignment in the fifo, so
    // we force the write pipe to reset on every GXFlush.
    GXSetResetWritePipe( GX_TRUE );
#endif    

    // Image on fish eye camera view
    TEXGetPalette( &texPalette, "gxTests/tf-02.tpl" );
    TEXGetGXTexObjFromPalette( texPalette, &texObj, 1 );
    
    while ( ! ( DEMOPadGetButton(0) & PAD_BUTTON_MENU ) )
    {
        // get pad status
        DEMOPadRead( );
        // General control & model animation
        myAnimeModel( );

        // Draw scene
        DEMOBeforeRender( );
        DrawScene( );
        DEMODoneRender( );
    }

    OSHalt("End of test");
}

//---------------------------------------------------------------------------
//  Model settings
//---------------------------------------------------------------------------
typedef  struct {
   s8  x,  y;
   s8  nx, ny, nz;
   s8  pad;
}  MyVertex;

static MyVertex ringVtx[] ATTRIBUTE_ALIGN(32) =
{
    { 100,   0,    64,   0,  0 }, /*   0 */
    {  87,  50,    55,  32,  0 }, /*  30 */
    {  50,  87,    32,  55,  0 }, /*  60 */
    {   0, 100,     0,  64,  0 }, /*  90 */
    { -50,  87,   -32,  55,  0 }, /* 120 */
    { -87,  50,   -55,  32,  0 }, /* 150 */
    {-100,   0,   -64,   0,  0 }, /* 180 */
    { -87, -50,   -55, -32,  0 }, /* 210 */
    { -50, -87,   -32, -55,  0 }, /* 240 */
    {   0,-100,     0, -64,  0 }, /* 270 */
    {  50, -87,    32, -55,  0 }, /* 300 */
    {  87, -50,    55, -32,  0 }, /* 330 */
};

#define VTXINRING       ((s32)(sizeof(ringVtx)/sizeof(ringVtx[0])))
#define NUMPIPES        64
#define NUMRINGS        (NUMPIPES+1)

f32  pipeAngle  = 10.0f;
f32  pipeLength = 50.0f;

Mtx pipeLocalMtx = 
{
  { 0.0646906f, 0.0760343f, 0.0058220f, 0.0f },
  {-0.0740718f, 0.0608390f, 0.0284950f, 0.0f },
  { 0.0181239f,-0.0227461f, 0.0956772f, 0.0f }
};

// For Pos/Tex Matrix Array
Mtx pipePosMtx[NUMRINGS] ATTRIBUTE_ALIGN(32);
Mtx pipeTexMtx[NUMRINGS] ATTRIBUTE_ALIGN(32);

//---------------------------------------------------------------------------
//  Draw entire scene
//---------------------------------------------------------------------------
void    DrawScene( void )
{
    Mtx44    projMtx;
    Camera*  c = &myCamera;
    
    // Set projection matrix
    MTXPerspective( projMtx, c->fovy, VP_ASPECT, c->znear, c->zfar );
    GXSetProjection( projMtx, GX_PERSPECTIVE );

    // Set rendering mode
    GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );
    GXSetZMode( GX_TRUE, GX_LESS, GX_TRUE );
    GXSetZCompLoc( GX_TRUE );

    // Set rendering mode
    GXSetCullMode( GX_CULL_BACK );

    // Draw objects
    myDrawModel( );

    return;
}

//---------------------------------------------------------------------------
//  Animate model
//---------------------------------------------------------------------------
void  myAnimeModel( void )
{
    Camera*  c = &myCamera;
    Mtx      localMtx, tmpMtx;
    Vec      axis;
    s32      i, j;
    f32      nfct;
    f32      zoom;
    Vec      rotAxis = { 0.0f, 1.0f, 0.2f };
    u16      buttons, stickDirs;

#ifdef  EMU
#  define MOVE_SCALE    5.0f
#else
#  define MOVE_SCALE    1.0f
#endif

    axis.x = axis.y = axis.z = 0.0f;
    zoom   = 0.0f;

    stickDirs = DEMOPadGetDirs(0);
    buttons   = DEMOPadGetButton(0);

    if ( stickDirs & DEMO_STICK_UP    ) axis.x -= 1.0f;
    if ( stickDirs & DEMO_STICK_DOWN  ) axis.x += 1.0f;
    if ( stickDirs & DEMO_STICK_LEFT  ) axis.y -= 1.0f;
    if ( stickDirs & DEMO_STICK_RIGHT ) axis.y += 1.0f;
    if ( buttons & PAD_BUTTON_A ) zoom =  2.0f * MOVE_SCALE;
    if ( buttons & PAD_BUTTON_B ) zoom = -2.0f * MOVE_SCALE;
    if ( buttons & PAD_TRIGGER_L ) pipeAngle += 0.1f;
    if ( buttons & PAD_TRIGGER_R ) pipeAngle -= 0.1f;
    
    //--------- Get View matrix
    c->position.y += zoom;
    MTXLookAt( c->viewMtx, &c->position, &c->up, &c->target );
    
    //--------- Make local matrix
    MTXInverse( c->viewMtx, tmpMtx );
    MTXMultVecSR( tmpMtx, &axis, &axis );
    if ( axis.x != 0.0f || axis.y != 0.0f || axis.z != 0.0f )
    {
        MTXRotAxisDeg( tmpMtx, &axis, 0.6f * MOVE_SCALE );
        MTXConcat( tmpMtx, pipeLocalMtx, pipeLocalMtx );
    }
    
    //--------- Get Position Matrix
    MTXConcat    ( c->viewMtx, pipeLocalMtx, pipePosMtx[0] );
    MTXTrans     ( localMtx, 0, 0, pipeLength );
    MTXRotAxisDeg( tmpMtx, &rotAxis, pipeAngle  );
    MTXConcat    ( tmpMtx, localMtx, localMtx );
    for ( i = 1; i < NUMRINGS; i ++ )
    {
        MTXConcat( pipePosMtx[i-1], localMtx, pipePosMtx[i] );
    }
    
    //--------- Get Texture Matrix for environment mapping
    for ( i = 0; i < NUMRINGS; i ++ )
    {
        for ( j = 0; j < 2; j ++ )
        {
            nfct = 0.5f / VECMag( (VecPtr)pipePosMtx[i][0] );
            if ( j == 1 ) nfct = -nfct;
            pipeTexMtx[i][j][0] = pipePosMtx[i][j][0] * nfct;
            pipeTexMtx[i][j][1] = pipePosMtx[i][j][1] * nfct;
            pipeTexMtx[i][j][2] = pipePosMtx[i][j][2] * nfct;
            pipeTexMtx[i][j][3] = 0.5f;
        }
    }
    DCFlushRange( pipePosMtx, sizeof(pipePosMtx) );
    DCFlushRange( pipeTexMtx, sizeof(pipeTexMtx) );
}

//---------------------------------------------------------------------------
//  Draw model
//---------------------------------------------------------------------------
void  myDrawModel( void )
{
    s32          i, j, k;
    GXPosNrmMtx  pm0, pm1;
    GXTexMtx     tm0, tm1;
	GXColor      black = {   0,   0,   0,   0 };
	GXColor      color = { 255, 128,  64, 255 };

    //--------- Set rendering parameters
    GXSetChanCtrl( GX_COLOR0,   GX_DISABLE,
                   GX_SRC_REG,  GX_SRC_REG, GX_LIGHT_NULL,
                   GX_DF_CLAMP, GX_AF_NONE  );
                   
    GXSetChanCtrl( GX_ALPHA0,   GX_DISABLE,
                   GX_SRC_REG,  GX_SRC_REG, GX_LIGHT_NULL,
                   GX_DF_CLAMP, GX_AF_NONE  );
                   
    GXSetChanAmbColor( GX_COLOR0A0, black );
    GXSetChanMatColor( GX_COLOR0A0, color );
    
    GXSetNumTexGens  ( 1 );             // # of Texgen proc
    GXSetNumChans    ( 1 );             // # of Color Channel
    GXSetNumTevStages( 1 );             // # of Tev Stage
    
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_MODULATE );
    
    // Set texture
    GXLoadTexObj( &texObj, GX_TEXMAP0 );
    
    //-------- Array base & stride setting
    GXSetArray( GX_POS_MTX_ARRAY, &pipePosMtx[0], sizeof( pipePosMtx[0] ) );
    GXSetArray( GX_TEX_MTX_ARRAY, &pipeTexMtx[0], sizeof( pipeTexMtx[0] ) );
    GXSetArray( GX_VA_POS,        &ringVtx[0].x,  sizeof( ringVtx[0]    ) );
    GXSetArray( GX_VA_NRM,        &ringVtx[0].nx, sizeof( ringVtx[0]    ) );
    
    //-------- Vertex descriptor settings
    GXClearVtxDesc( );
    GXSetVtxDesc( GX_VA_POS, GX_INDEX8 );
    GXSetVtxDesc( GX_VA_NRM, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XY,  GX_S8, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0 );
    
    //-------- Pipe lid 0  (Triangle fan)
    pm0 = GX_PNMTX0;
    tm0 = GX_TEXMTX0;
#ifdef BUG_INDX_MTX_LOAD
    GXCmd1u8(0);
    GXCmd1u8(0);
#endif
    GXLoadPosMtxIndx( 0, pm0 );
    GXLoadTexMtxIndx( 0, tm0, GX_MTX2x4 );
    GXSetCurrentMtx ( pm0 );
    GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, tm0 );

#ifdef BUG_TRIANGLE_FAN
    for ( i = 1; i < VTXINRING; i ++ )
    {
    GXBegin( GX_TRIANGLES, GX_VTXFMT0, 3 );
        GXPosition1x8( (u8)0 );
        GXNormal3s8  ( (s8)(ringVtx[0].nx/4), (s8)(ringVtx[0].ny/4), -60 );

        GXPosition1x8( (u8)i );
        GXNormal3s8  ( (s8)(ringVtx[i].nx/4), (s8)(ringVtx[i].ny/4), -60 );

        GXPosition1x8( (u8)((i+1) % VTXINRING));
        GXNormal3s8  ( (s8)(ringVtx[(i+1) % VTXINRING].nx/4), (s8)(ringVtx[(i+1) % VTXINRING].ny/4), -60 );
    GXEnd( );
    }
#else
    GXBegin( GX_TRIANGLEFAN, GX_VTXFMT0, VTXINRING );
    for ( i = 0; i < VTXINRING; i ++ )
    {
        GXPosition1x8( (u8)i );
        GXNormal3s8  ( (s8)(ringVtx[i].nx/4), (s8)(ringVtx[i].ny/4), -60 );
    }
    GXEnd( );
#endif // BUG_TRIANGLE_FAN

    //-------- Pipe body (Triangle strip)
    GXSetVtxDesc( GX_VA_PNMTXIDX,   GX_DIRECT );
    GXSetVtxDesc( GX_VA_TEX0MTXIDX, GX_DIRECT );
    GXSetVtxDesc( GX_VA_NRM,        GX_INDEX8 );
    for ( j = 0; j < NUMRINGS-1; j ++ )
    {
        pm1 = ( pm0 == GX_PNMTX0  ) ? GX_PNMTX1  : GX_PNMTX0;
        tm1 = ( tm0 == GX_TEXMTX0 ) ? GX_TEXMTX1 : GX_TEXMTX0;
        GXLoadPosMtxIndx( (u16)(j+1), pm1 );
        GXLoadTexMtxIndx( (u16)(j+1), tm1, GX_MTX2x4 );
        GXBegin( GX_TRIANGLESTRIP, GX_VTXFMT0, (VTXINRING+1)*2 );
        k = VTXINRING-1;
        for ( i = -1; i < VTXINRING; i ++ )
        {
            GXMatrixIndex1x8( (u8)pm0 );
            GXMatrixIndex1x8( (u8)tm0 );
            GXPosition1x8   ( (u8)k   );
            GXNormal1x8     ( (u8)k   );
            GXMatrixIndex1x8( (u8)pm1 );
            GXMatrixIndex1x8( (u8)tm1 );
            GXPosition1x8   ( (u8)k   );
            GXNormal1x8     ( (u8)k   );
            k = i + 1;
        }
        GXEnd( );
        pm0 = pm1;
        tm0 = tm1;
    }
    
    //-------- Pipe lid 1 (Triangle fan)
    GXSetVtxDesc( GX_VA_PNMTXIDX,   GX_NONE   );
    GXSetVtxDesc( GX_VA_TEX0MTXIDX, GX_NONE   );
    GXSetVtxDesc( GX_VA_NRM,        GX_DIRECT );
    GXSetCurrentMtx( pm0 );
    GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, tm0 );

#ifdef BUG_TRIANGLE_FAN
    for ( i = 1; i < VTXINRING; i ++ )
    {
    GXBegin( GX_TRIANGLES, GX_VTXFMT0, 3 );
        GXPosition1x8( (u8)(VTXINRING-1) );
        GXNormal3s8  ( (s8)(ringVtx[0].nx/4), (s8)(ringVtx[0].ny/4), +60 );

        GXPosition1x8( (u8)(VTXINRING-1-i) );
        GXNormal3s8  ( (s8)(ringVtx[i].nx/4), (s8)(ringVtx[i].ny/4), +60 );

        if (i != VTXINRING-1)
            GXPosition1x8( (u8)(VTXINRING-2-i) );
        else
            GXPosition1x8( (u8)1);
        GXNormal3s8  ( (s8)(ringVtx[(i+1)%VTXINRING].nx/4), (s8)(ringVtx[(i+1)%VTXINRING].ny/4), +60 );
    GXEnd( );
    }
#else
    GXBegin( GX_TRIANGLEFAN, GX_VTXFMT0, VTXINRING );
    for ( i = 0; i < VTXINRING; i ++ )
    {
        GXPosition1x8( (u8)(VTXINRING-1-i) );
        GXNormal3s8  ( (s8)(ringVtx[i].nx/4), (s8)(ringVtx[i].ny/4), +60 );
    }
    GXEnd( );
#endif // BUG_TRIANGLE_FAN 

    return;
}

/*======== End of tf-reflect.c ========*/
