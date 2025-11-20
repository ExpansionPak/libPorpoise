/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tev-outline.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Tev/tev-outline.c $
    
    21    7/23/01 9:04p Hirose
    Fixed MTXRotAxis -> MTXRotAxisDeg.
    
    20    5/13/01 2:48p Hirose
    removed obsolete flags
    
    19    4/09/01 7:40p Hirose
    fixed matrix type for projection API (should be Mtx44)
    
    18    11/01/00 10:31p Hirose
    deleted GXSetTevClampMode calls
    
    17    10/31/00 8:37p Hirose
    flag fixes
    
    16    7/20/00 3:40p Hirose
    added overview
    
    15    7/18/00 2:19p Tian
    Added call to GXSetResetWritePipe
    
    14    7/07/00 5:57p Dante
    PC Compatibility
    
    13    6/15/00 6:01p Carl
    Fixed issue concerning index normal matrix load
    
    12    6/13/00 6:52p Hirose
    fixed wrong build flag
    
    11    6/13/00 11:31a Hirose
    Fixed compiler warnings
    
    10    6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    9     6/06/00 8:06p Hirose
    removed the workaround which actually doesn't effect
    
    8     6/02/00 10:30p Hirose
    updated dummy command
    
    7     5/23/00 2:33p Alligator
    fixed warning
    
    6     5/22/00 3:30a Hirose
    completed workarounds to make this code work on HW1 
    
    5     5/21/00 3:57a Hirose
    temporary update to make the code work on HW1_DRIP
    
    4     5/02/00 4:04p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    3     3/24/00 3:44p Hirose
    changed to use DEMOPad library instead of cmn-pad
    added a patch for TRAINGLE_FAN bug on HW1
    
    2     3/15/00 6:21p Danm
    Fixed EPPC version.
    
    1     3/06/00 12:12p Alligator
    move from tests/gx and rename
    
    8     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    7     2/12/00 5:16p Alligator
    Integrate ArtX source tree changes
    
    6     2/08/00 7:11p Yasu
    
    5     1/25/00 2:56p Carl
    Changed to standardized end of test message
    
    4     1/24/00 6:40p Carl
    Removed some #ifdef EPPC stuff.
    
    Also wrote more implementation for EPPC side.
    Unsure if it works right, however.
    
    3     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/10/99 4:45p Carl
    Fixed Z compare stuff.
    
    5     11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    4     11/11/99 7:03p Yasu
    Fixed order of parameter on GXLoadTexObjPreLoaded
    
    3     11/11/99 5:45p Yasu
    Fixed a bug in EPPC
    
    2     11/11/99 2:44p Yasu
    Add SINGLEFRAME test
    
    1     11/08/99 2:17p Yasu
    Initial version of outline demo
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   tev-outline
       Cartoon outline effects by using alpha plane copy and TEV features
 *---------------------------------------------------------------------------*

   Overview of this method:

       (1st. pass) Render the entire scene as usual. At this time,
       paint each object part by its individual ID number which is
       drawn into alpha plane. It doesn't affect the result image
       in color buffer.
       
       Then copy the entire alpha plane into a texture (ID map).
       The framebuffer is not cleared by this operation.
       
       (2nd. pass) Render a screen-sized quad with ID map. Then,
       generate two texcoords to lookup this map. One exactly
       matchs screen-space coordinate, and another is perturbed
       by just one pixel width horizontally (or vertically).
       Then compare texel values from these two texcoords each
       other. The comparison can be done by some TEV stages and
       alpha compare feature. If two ID values are not similar,
       it means the part is a border of two different objects,
       hence should be outlined.
       
       Since this demo sets the tolerance value of comparison
       as more than 1, no outline will be generated between two
       adjacent ID numbers. It will help outlining self-intersection
       of soft objects.
       
       This demo draws horizontal/vertical outlines separately.
       
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>
#ifdef   MACOS
#  include <agl.h>
#endif

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define SCREEN_WD_MAX   640
#define SCREEN_HT_MAX   480

#define VP_ASPECT   ((float)SCREEN_WD_MAX/(float)SCREEN_HT_MAX)
#define VP_FOVY       33.3f
#define VP_NEAR       16.0f     // near plane Z in view coordinates
#define VP_FAR      1024.0f     // far  plane Z in view coordinates

#define SCREEN_ZNEAR    0.0f        // near plane Z in screen coordinates
#define SCREEN_ZFAR     1.0f        // far  plane Z in screen coordinates
#define ZBUFFER_MAX     0x00ffffff

#define ASFT(x)         ((x)<<2)    // ID step size ( 6bit alpha )


#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void  DrawInit ( void );
void  DrawScene( void );
void  myCreateModel( void );
void  myDrawModel( void );
void  myAnimeModel( void );
void  PrintIntro( void );

void  DrawCartoonOutline( void );


/*---------------------------------------------------------------------------*
   Global Variables
 *---------------------------------------------------------------------------*/
u16 screen_wd;
u16 screen_ht;

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
       33.3f,                  // fovy
      128.0f,                  // near plane Z in camera coordinates
     2048.0f,                  // far  plane Z in camera coordinates
};

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void  main ( void )
{
    GXRenderModeObj *rmp;

    // initialize render settings and set clear color for first frame
    DEMOInit( NULL );  // Defined in $DOLPHIN/build/libraries/demo/src/DEMOInit.c

#ifdef BUG_INDX_ALIGN    
    // This program depends on specific command alignment in the fifo, so
    // we force the write pipe to reset on every GXFlush.
    GXSetResetWritePipe( GX_TRUE );
#endif    

    rmp = DEMOGetRenderModeObj();
    screen_wd = rmp->fbWidth;
    screen_ht = rmp->efbHeight;

    PrintIntro( );  // Print demo directions
    
    DrawInit( );

    while ( ! ( DEMOPadGetButton(0) & PAD_BUTTON_MENU ) )
    {
        // get pad status
        DEMOPadRead( );

        // General control & model animation
        myAnimeModel( );

        // Draw scene
        DEMOBeforeRender( );

        DrawScene( );
        DrawCartoonOutline( );
        DEMODoneRender( );
    }

    OSHalt("End of demo");
}

//---------------------------------------------------------------------------
// Cartoon outline settings
//---------------------------------------------------------------------------

/*---------------------------------------------------------------------------*/
#ifndef  EMU

/*---------------------------------------------------------------------------*
   Draw Cartoon outline for real HW
 *---------------------------------------------------------------------------*/
void  DrawCartoonOutline( void )
{
    static u16      *alphaBuffer;
    static GXTexObj eFBTexObj;
    static Mtx      texMtx1, texMtx2;
    static Mtx      mtxUnit;
    static Mtx44    mtxProj;
    static s32      initialized = 0;
    f32 zMid;
    
    if ( !initialized )
    {
        alphaBuffer = OSAlloc(screen_wd*screen_ht*sizeof(u16));
        ASSERT( alphaBuffer != NULL );

        MTXIdentity(mtxUnit);
        MTXOrtho(mtxProj, 0.0f, (f32) screen_ht, 0.0f, (f32) screen_wd, SCREEN_ZNEAR, SCREEN_ZFAR);

        // set up texture offset matrices
        MTXIdentity(texMtx1);
        MTXRowCol(texMtx1, 0, 3) = 1.0f / (f32)screen_wd;
        MTXIdentity(texMtx2);
        MTXRowCol(texMtx2, 1, 3) = 1.0f / (f32)screen_ht;
        
        GXInitTexObj( &eFBTexObj, alphaBuffer, screen_wd, screen_ht,
                      GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_DISABLE );
        GXInitTexObjLOD( &eFBTexObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f,
                         GX_DISABLE, GX_DISABLE, GX_ANISO_1 );
        initialized = 1;
    }

    // Copy out eFB alpha plane as I8 texture format
    GXSetTexCopySrc( 0, 0, screen_wd, screen_ht );
    GXSetTexCopyDst(screen_wd, screen_ht, GX_TF_A8, GX_DISABLE );
    GXCopyTex( alphaBuffer, GX_DISABLE );

    // Be sure texture is completely copied out
    GXPixModeSync();

    // flush texture cache
    GXInvalidateTexAll();

    // Load the bitmap into TMEM
    GXLoadTexObj( &eFBTexObj, GX_TEXMAP0 );

    // Setup for outline drawing
    GXSetNumTexGens  ( 2 );
    GXSetNumChans    ( 0 );
    GXSetNumTevStages( 2 );
    
    // first TEV stage just gets texture lookup value
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
    GXSetTevOp( GX_TEVSTAGE0, GX_REPLACE );

    // second stage does the subtract (in alpha; colors gets zero)
    // clamping turned off (in alpha) to allow wrapped "negative" values
    GXSetTevOrder( GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL );
    GXSetTevColorIn( GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO,
                     GX_CC_ZERO, GX_CC_ZERO );
    GXSetTevColorOp( GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                     GX_ENABLE, GX_TEVPREV );
    GXSetTevAlphaIn( GX_TEVSTAGE1, GX_CA_TEXA, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_APREV );
    GXSetTevAlphaOp( GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1,
                     GX_DISABLE, GX_TEVPREV );

    // only write pixels that pass alpha test
    GXSetAlphaCompare( GX_GREATER, ASFT(1), GX_AOP_AND, GX_LESS, 256-ASFT(1) );
    
    // always draw regardless of Z
    GXSetZMode( GX_TRUE, GX_ALWAYS, GX_TRUE );

    // load matrices
    GXSetCurrentMtx(GX_PNMTX0);
    GXLoadPosMtxImm(mtxUnit, GX_PNMTX0);
    GXSetProjection(mtxProj, GX_ORTHOGRAPHIC);

    GXLoadTexMtxImm(mtxUnit, GX_TEXMTX0, GX_MTX2x4);
    GXLoadTexMtxImm(texMtx1, GX_TEXMTX1, GX_MTX2x4);    
    GXLoadTexMtxImm(texMtx2, GX_TEXMTX2, GX_MTX2x4);

    // vertex descriptor settings
    GXClearVtxDesc( );
    GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
    GXSetVtxDesc( GX_VA_TEX0, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 );

    zMid = -0.5f * (SCREEN_ZNEAR + SCREEN_ZFAR);

    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1);

    // draw with texture offset in S
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(0.0F, 0.0F, zMid);
        GXTexCoord2f32(0.0F, 0.0F);
        GXPosition3f32((f32) screen_wd, 0.0F, zMid);
        GXTexCoord2f32(1.0F, 0.0F);
        GXPosition3f32((f32) screen_wd, (f32) screen_ht, zMid);
        GXTexCoord2f32(1.0F, 1.0F);
        GXPosition3f32(0.0F, (f32) screen_ht, zMid);
        GXTexCoord2f32(0.0F, 1.0F);
    GXEnd();

    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2);

    // draw with texture offset in T
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(0.0F, 0.0F, zMid);
        GXTexCoord2f32(0.0F, 0.0F);
        GXPosition3f32((f32) screen_wd, 0.0F, zMid);
        GXTexCoord2f32(1.0F, 0.0F);
        GXPosition3f32((f32) screen_wd, (f32) screen_ht, zMid);
        GXTexCoord2f32(1.0F, 1.0F);
        GXPosition3f32(0.0F, (f32) screen_ht, zMid);
        GXTexCoord2f32(0.0F, 1.0F);
    GXEnd();

    return;
}

/*---------------------------------------------------------------------------*/
#else // EMU
u8  alphaBuffer[SCREEN_WD_MAX*SCREEN_HT_MAX];

/*---------------------------------------------------------------------------*
   Draw Cartoon outline for the emulator (OpenGL)
 *---------------------------------------------------------------------------*/
void  DrawCartoonOutline( void )
{
    u32    i;
    u8     a00 = 0, a01, a10, d01, d10;
    
    // Read out pixel alpha
    glReadPixels( 0, 0, screen_wd, screen_ht,
                  GL_ALPHA, GL_UNSIGNED_BYTE, alphaBuffer );

    // Modify alpha
    for ( i = 0; i < screen_wd*(screen_ht-1); i ++ )
    {
        a01 = a00;
        a00 = alphaBuffer[i];
        a10 = alphaBuffer[i+screen_wd];
        d01 = (u8)(a01 - a00 + ASFT(1));
        d10 = (u8)(a10 - a00 + ASFT(1));
        if ( d01 > ASFT(2) || d10 > ASFT(2) ) alphaBuffer[i] = 0x00;
        else                                  alphaBuffer[i] = 0xff;
    }
    
    // Write back alpha as blend parameter
    glBlendFunc( GL_ZERO, GL_SRC_ALPHA );
    glAlphaFunc( GL_ALWAYS, 0.0f );
    glViewport ( 0, 0, screen_wd, screen_ht );
    glScissor  ( 0, 0, screen_wd, screen_ht );
    glDrawPixels( screen_wd, screen_ht,
                  GL_ALPHA, GL_UNSIGNED_BYTE, alphaBuffer );
    return;
}
#endif // EMU

//---------------------------------------------------------------------------
//  Model settings
//---------------------------------------------------------------------------
#define MYDLBUFSIZE      1024

#define VTXINRING        12
#define NUMRINGS         6

#define PIPELID0_NRM     12
#define PIPELID1_NRM     13
#define PIPELID0_CLR     (NUMRINGS-1) 
#define PIPELID1_CLR     (NUMRINGS)

typedef  struct {
   s8  x,  y;
   s8  nx, ny, nz, pad;
}  MyVertex;

static MyVertex ringVtx[] ATTRIBUTE_ALIGN(32) =
{
    //  x,   y,    nx,  ny,  nz,  pad
    { 100,   0,    64,   0,   0,    0 }, /*   0 */
    {  87,  50,    55,  32,   0,    0 }, /*  30 */
    {  50,  87,    32,  55,   0,    0 }, /*  60 */
    {   0, 100,     0,  60,   0,    0 }, /*  90 */
    { -50,  87,   -32,  55,   0,    0 }, /* 120 */
    { -87,  50,   -55,  32,   0,    0 }, /* 150 */
    {-100,   0,   -64,   0,   0,    0 }, /* 180 */
    { -87, -50,   -55, -32,   0,    0 }, /* 210 */
    { -50, -87,   -32, -55,   0,    0 }, /* 240 */
    {   0,-100,     0, -64,   0,    0 }, /* 270 */
    {  50, -87,    32, -55,   0,    0 }, /* 300 */
    {  87, -50,    55, -32,   0,    0 }, /* 330 */
    
    {   0,   0,     0,   0, -60,    0 }, /* center of lid */
    {   0,   0,     0,   0,  60,    0 }, /* center of lid */
};

static GXColor pipeClr[] ATTRIBUTE_ALIGN(32) =
{
    //  R,   G,   B,   A(ID)
    {  64, 128, 255, ASFT(4) }, // for pipe
    {  64, 128, 255, ASFT(5) },
    {  64, 128, 255, ASFT(5) },
    {  64, 128, 255, ASFT(5) },
    {  64, 128, 255, ASFT(6) },
    {  64, 128, 255, ASFT(2) }, // for lid0
    {  64, 128, 255, ASFT(8) }, // for lid1
};

struct {
   f32  rot;
   f32  length;
}
pipe[NUMRINGS-1] =
{
    {  0.0f, 400.0f },
    { 20.0f,  50.0f },
    { 20.0f,  50.0f },
    { 20.0f,  50.0f },
    {  0.0f, 400.0f },
};

Mtx pipeLocalMtx = 
{ { 0.0646906f, 0.0760343f, 0.0058220f, 0.0f },
  {-0.0740718f, 0.0608390f, 0.0284950f, 0.0f },
  { 0.0181239f,-0.0227461f, 0.0956772f, 0.0f } };

Mtx pipeMtx[NUMRINGS] ATTRIBUTE_ALIGN(32);

// display list
u8*             myDisplayList = NULL;
u32             myDLSize;
GXVtxDescList   myDLVcd[GX_MAX_VTXDESCLIST_SZ];

//---------------------------------------------------------------------------
//  Init scene
//---------------------------------------------------------------------------
void    DrawInit( void )
{
	GXColor color = {160,160,160,0};
    // need an alpha channel
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    GXSetCopyClear( color , GX_MAX_Z24 );
    // clear framebuffer becausae eFB format is changed
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);

    // We're using alpha compare, so we must compare z AFTER texturing
    GXSetZCompLoc( GX_DISABLE );

    // update both color and alpha
    GXSetAlphaUpdate(GX_TRUE);
    GXSetColorUpdate(GX_TRUE);
    
    // create the model data
    myCreateModel();
}

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

    // Set Viewport
    GXSetViewport( 0, 0, screen_wd, screen_ht, SCREEN_ZNEAR, SCREEN_ZFAR );
    GXSetScissor ( 0, 0, screen_wd, screen_ht );

    // Set rendering mode
    GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );
    GXSetZMode( GX_TRUE, GX_LESS, GX_TRUE );

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
    static f32  angle = 60.0f;
    Camera*     c = &myCamera;
    Mtx         localMtx, tmpMtx;
    Vec         axis;
    s32         i;
    u16         stickDirs;
    
#ifdef EMU
#  define MOVE_SCALE    5.0f
#else
#  define MOVE_SCALE    1.0f
#endif

    axis.x = axis.y = axis.z = 0.0f;

    stickDirs = DEMOPadGetDirs(0);

    // Model rotation
    stickDirs = DEMOPadGetDirs(0);
    if ( stickDirs & DEMO_STICK_UP    )
        axis.x -= 1.0f;
    if ( stickDirs & DEMO_STICK_DOWN  )
        axis.x += 1.0f;
    if ( stickDirs & DEMO_STICK_LEFT  )
        axis.y -= 1.0f;
    if ( stickDirs & DEMO_STICK_RIGHT )
        axis.y += 1.0f;
    if ( stickDirs & DEMO_SUBSTICK_LEFT  )
        axis.z -= 1.0f;
    if ( stickDirs & DEMO_SUBSTICK_RIGHT )
        axis.z += 1.0f;

    // Camera zoom
    c->position.y += (f32)DEMOPadGetSubStickY(0) / 30.0f * MOVE_SCALE;
    Clamp(c->position.y, -600.0f, -200.0f);

    // Joint angle
    if ( DEMOPadGetButton(0) & PAD_TRIGGER_L )
        angle += MOVE_SCALE * 3.0f;
    if ( DEMOPadGetButton(0) & PAD_TRIGGER_R )
        angle -= MOVE_SCALE * 3.0f;
    Clamp(angle, 0.0f, 150.0f);
    
    
    //--------- Get View matrix
    MTXLookAt( c->viewMtx, &c->position, &c->up, &c->target );
    
    //--------- Make local matrix
    MTXInverse( c->viewMtx, tmpMtx );
    MTXMultVecSR( tmpMtx, &axis, &axis );
    if ( axis.x != 0.0f || axis.y != 0.0f || axis.z != 0.0f )
    {
        MTXRotAxisDeg( tmpMtx, &axis, MOVE_SCALE );
        MTXConcat( tmpMtx, pipeLocalMtx, pipeLocalMtx );
    }

    //--------- Set pipe bend parameter
    for ( i = 1; i < NUMRINGS-2; i ++ )
    {
        pipe[i].rot = angle / ((NUMRINGS-3) * 2);
    }
    
    //--------- Make ModelView matrix
    MTXConcat( c->viewMtx, pipeLocalMtx, pipeMtx[0] );
    for ( i = 0; i < NUMRINGS-1; i ++ )
    {
        MTXTrans( localMtx, 0, 0, pipe[i].length );
        MTXRotDeg( tmpMtx, 'Y', pipe[i].rot );
        MTXConcat( tmpMtx,   localMtx, localMtx );
        MTXConcat( localMtx, tmpMtx,   localMtx );
        MTXConcat( pipeMtx[i], localMtx, pipeMtx[i+1] );
    }
    DCFlushRange( pipeMtx, sizeof(pipeMtx) );
}

//---------------------------------------------------------------------------
//  Create a model as display list
//---------------------------------------------------------------------------
void  myCreateModel( void )
{
    s32     i, j, k;
    u8      m0 = 0; // GX_PNMTX0;
    u8      m1;

    //--------- allocate buffer for the display list
    if ( myDisplayList == NULL )
    {
        myDisplayList = OSAlloc(MYDLBUFSIZE);
        DCInvalidateRange( myDisplayList, MYDLBUFSIZE);
    }

    //-------- Vertex attribute settings
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS,  GX_POS_XY,   GX_S8, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM,  GX_NRM_XYZ,  GX_S8, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );

    //-------- Vertex descriptor settings
    GXClearVtxDesc( );
    GXSetVtxDesc( GX_VA_PNMTXIDX, GX_DIRECT );
    GXSetVtxDesc( GX_VA_POS, GX_INDEX8 );
    GXSetVtxDesc( GX_VA_NRM, GX_INDEX8 );
    GXSetVtxDesc( GX_VA_CLR0, GX_INDEX8 );

    GXGetVtxDescv(myDLVcd); // save vertex descriptor state

    //-------- start generating display list
    GXBeginDisplayList( myDisplayList, MYDLBUFSIZE );


    //-------- Pipe lid 0  (Triangle fan)
#ifdef BUG_TRIANGLE_FAN // There is a FAN bug on HW version 1.
    GXBegin( GX_TRIANGLES, GX_VTXFMT0, (VTXINRING-2)*3 );
    for ( i = 1; i < VTXINRING-1; i ++ )
    {
        GXMatrixIndex1x8( m0 );
        GXPosition1x8   ( (u8)0 );
        GXNormal1x8     ( (u8)PIPELID0_NRM );
        GXColor1x8      ( (u8)PIPELID0_CLR );
        GXMatrixIndex1x8( m0 );
        GXPosition1x8   ( (u8)i );
        GXNormal1x8     ( (u8)PIPELID0_NRM );
        GXColor1x8      ( (u8)PIPELID0_CLR );
        GXMatrixIndex1x8( m0 );
        GXPosition1x8   ( (u8)(i+1) );
        GXNormal1x8     ( (u8)PIPELID0_NRM );
        GXColor1x8      ( (u8)PIPELID0_CLR );
    }
    GXEnd( );
#else // !BUG_TRIANGLE_FAN
    GXBegin( GX_TRIANGLEFAN, GX_VTXFMT0, VTXINRING );
    for ( i = 0; i < VTXINRING; i ++ )
    {
        GXMatrixIndex1x8( m0 );
        GXPosition1x8   ( (u8)i );
        GXNormal1x8     ( (u8)PIPELID0_NRM );
        GXColor1x8      ( (u8)PIPELID0_CLR );
    }
    GXEnd( );
#endif // BUG_TRIANGLE_FAN


    //-------- Pipe body (Triangle strip)
    for ( j = 0; j < NUMRINGS-1; j ++ )
    {
        m0 = (u8)(j * 3);   // GX_PNMTX0(=0), GX_PNMTX1(=3), ...
        m1 = (u8)((j+1)*3); // GX_PNMTX1(=3), GX_PNMTX2(=6), ...
        
        GXBegin( GX_TRIANGLESTRIP, GX_VTXFMT0, (VTXINRING+1)*2 );
        k = VTXINRING-1;
        for ( i = -1; i < VTXINRING; i ++, k = i )
        {
            GXMatrixIndex1x8( m0 );
            GXPosition1x8   ( (u8)k );
            GXNormal1x8     ( (u8)k );
            GXColor1x8      ( (u8)j );
            GXMatrixIndex1x8( m1 );
            GXPosition1x8   ( (u8)k );
            GXNormal1x8     ( (u8)k );
            GXColor1x8      ( (u8)j );
        }
        GXEnd( );

    }

    //-------- Pipe lid 1 (Triangle fan)
#ifdef BUG_TRIANGLE_FAN // There is a FAN bug on HW version 1.
    GXBegin( GX_TRIANGLES, GX_VTXFMT0, (VTXINRING-2)*3 );
    for ( i = 1; i < VTXINRING-1; i ++ )
    {
        GXMatrixIndex1x8( m1 );
        GXPosition1x8   ( (u8)(VTXINRING-1) );
        GXNormal1x8     ( (u8)PIPELID1_NRM );
        GXColor1x8      ( (u8)PIPELID1_CLR );
        GXMatrixIndex1x8( m1 );
        GXPosition1x8   ( (u8)i );
        GXNormal1x8     ( (u8)PIPELID1_NRM );
        GXColor1x8      ( (u8)PIPELID1_CLR );
        GXMatrixIndex1x8( m1 );
        GXPosition1x8   ( (u8)(i-1) );
        GXNormal1x8     ( (u8)PIPELID1_NRM );
        GXColor1x8      ( (u8)PIPELID1_CLR );
    }
    GXEnd( );
#else // !BUG_TRIANGLE_FAN
    GXBegin( GX_TRIANGLEFAN, GX_VTXFMT0, VTXINRING );
    for ( i = 0; i < VTXINRING; i ++ )
    {
        GXMatrixIndex1x8( m1 );
        GXPosition1x8   ( (u8)(VTXINRING-1-i) );
        GXNormal1x8     ( (u8)PIPELID1_NRM );
        GXColor1x8      ( (u8)PIPELID1_CLR );
    }
    GXEnd( );
#endif // BUG_TRIANGLE_FAN


    //-------- end of the display list
    myDLSize = GXEndDisplayList();

}

//---------------------------------------------------------------------------
//  Draw model
//---------------------------------------------------------------------------
void  myDrawModel( void )
{
    s32         i;
    u32         m0;
    GXColor     matColor = { 0, 0, 0, 0 };
    GXLightObj  light;
    GXColor		color0 = { 160, 160, 160, 0 };
    GXColor		color1 = { 96, 96, 96, 0 };
    
    //--------- Set rendering parameters
    GXSetChanCtrl( GX_COLOR0,   GX_ENABLE,
                   GX_SRC_REG,  GX_SRC_VTX, GX_LIGHT0,
                   GX_DF_CLAMP, GX_AF_NONE  );
                   
    GXSetChanCtrl( GX_ALPHA0,   GX_DISABLE,
                   GX_SRC_REG,  GX_SRC_VTX, GX_LIGHT_NULL,
                   GX_DF_CLAMP, GX_AF_NONE  );
                   
    GXSetChanAmbColor( GX_COLOR0A0, color0 );

    GXInitLightPos(&light, 1000.0f, 0.0f, 0.0f);
    GXInitLightColor(&light, color1 );
    GXLoadLightObjImm(&light, GX_LIGHT0);

    GXSetNumTexGens  ( 0 );             // # of Texgen proc
    GXSetNumChans    ( 1 );             // # of Color Channel
    GXSetNumTevStages( 1 );             // # of Tev Stage
    
    GXSetTevOrder( GX_TEVSTAGE0,
                   GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
    
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

    //-------- Array base & stride setting
    GXSetArray( GX_VA_POS,        &ringVtx[0].x,  sizeof( ringVtx[0] ) );
    GXSetArray( GX_VA_NRM,        &ringVtx[0].nx, sizeof( ringVtx[0] ) );
    GXSetArray( GX_VA_CLR0,       &pipeClr[0],    sizeof( pipeClr[0] ) );
    GXSetArray( GX_POS_MTX_ARRAY, &pipeMtx[0],    sizeof( pipeMtx[0] ) );

    //-------- Vertex attribute settings
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XY,  GX_S8, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );

    //-------- Vertex descriptor settings
    GXSetVtxDescv(myDLVcd);

    //-------- Set transform matrices
    for ( i = 0 ; i < NUMRINGS ; ++i )
    {
        m0 = (u32)(i * 3); // GX_PNMTX0(=0), GX_PNMTX1(=3), ...
        GXLoadPosMtxIndx( (u16)i, m0 );
        GXLoadNrmMtxImm( pipeMtx[i], m0 );
    }

    //-------- Draw the model
    GXCallDisplayList( myDisplayList, myDLSize );

    return;
}

//---------------------------------------------------------------------------
//    Prints the directions on how to use this demo
//---------------------------------------------------------------------------
void PrintIntro( void )
{
    OSReport("\n\n");
    OSReport("**********************************************\n");
    OSReport("tev-outline: cartoon-outline demo\n");
    OSReport("**********************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main stick   : Rotate the model\n");
    OSReport("Sub  stick   : Control zoom\n");
    OSReport("L/R triggers : Bend/Streatch the model\n");
    OSReport("**********************************************\n");
    OSReport("\n\n");
}

/*======== End of tev-outline.c ========*/
