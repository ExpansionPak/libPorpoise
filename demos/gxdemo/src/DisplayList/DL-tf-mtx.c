/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     DL-tf-mtx.c

  Copyright 1998 - 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/DisplayList/DL-tf-mtx.c $
    
    2     7/16/01 8:55a Hirose
    
    1     7/03/01 9:44p Hirose
    moved to demo project.
    
    2     7/02/01 1:55p Hirose
    
    1     7/02/01 1:52p Hirose
    initial check in as a test.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    DL-tf-mtx
        Display list which includes matrix load commands
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define ASPECT              ((float)640/(float)448)

#define NUM_MTX_SLOTS       10
#define MTX_IDX_SPAN        3

#define DL_BUFSIZE          16384

#define NUM_CONTOUR         100     // (9*11+1) : should be (9n+1)
#define CONTOUR_SPAN        50.0F
#define VTX_PER_CONTOUR     12


/*---------------------------------------------------------------------------*
   Rendering parameters
 *---------------------------------------------------------------------------*/
typedef f32 Mtx33[3][3];
typedef f32 Mtx24[2][4];

typedef struct
{
    Point3d   position;
    Point3d   target;
    Vec       up;
    f32       fovy;
    f32       znear;
    f32       zfar;
    Mtx       viewMtx;
} MyCameraObj;

typedef struct
{
   s8  x,  y;
   s8  nx, ny, nz;
   s8  pad;
} MyVertex;


/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
static void     DrawInit        ( void );
static void     DrawTick        ( void );
static void     AnimTick        ( void );
static void     BuildModelDL    ( void );
static void     BuildMtxArray   ( void );
static void     CheckMissingObj ( void );
inline void     CopyMtxTo3x3    ( Mtx src, Mtx33 dst );

//---------------------------------------------------------------------------
//  Global variables
//---------------------------------------------------------------------------
// Camera
MyCameraObj DefaultCamera =
{
    { 0.0f, -5000.0f, 0.0f },   // position
    { 0.0f,     0.0f, 0.0f },   // target
    { 0.0f,     0.0f, 1.0f },   // upVec
       33.3f,                   // fovy
     2000.0f,                   // near plane Z in camera coordinates
    10000.0f,                   // far  plane Z in camera coordinates
};

// For texture palette
static TEXPalettePtr   MyTplObj = 0;

// For model display list
static void*    ModelDLBuffer = NULL;
static u32      ModelDLSize = 0;
// For model transform matrix array (double buffered)
static Mtx*     PosMtxArrayBuf[2];
static Mtx33*   NrmMtxArrayBuf[2];
static Mtx24*   TexMtxArrayBuf[2];
static u32      CurrentMtxBuffer;

// For animation control
static Vec      AnimCtrlArray[NUM_CONTOUR];
static u32      AnimCtrlPtr = 0;
static s32      AnimCounter = 0;
static u32      AnimReset   = 0;
static u32      AnimPause   = 0;
static Mtx      AnimMtxOrigin;
static Mtx      AnimGlobalRot;

//---------------------------------------------------------------------------
//  Model data
//---------------------------------------------------------------------------
static MyVertex VtxArray[VTX_PER_CONTOUR] ATTRIBUTE_ALIGN(32) =
{
    {  50,   0,    64,   0,  0 }, /*   0 */
    {  44,  25,    55,  32,  0 }, /*  30 */
    {  25,  44,    32,  55,  0 }, /*  60 */
    {   0,  50,     0,  64,  0 }, /*  90 */
    { -25,  44,   -32,  55,  0 }, /* 120 */
    { -44,  25,   -55,  32,  0 }, /* 150 */
    { -50,   0,   -64,   0,  0 }, /* 180 */
    { -44, -25,   -55, -32,  0 }, /* 210 */
    { -25, -44,   -32, -55,  0 }, /* 240 */
    {   0, -50,     0, -64,  0 }, /* 270 */
    {  25, -44,    32, -55,  0 }, /* 300 */
    {  44, -25,    55, -32,  0 }, /* 330 */
};

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void  main ( void )
{
    // initialize render settings and set clear color for first frame
    DEMOInit(NULL);
    DrawInit();

    while ( ! ( DEMOPadGetButton(0) & PAD_BUTTON_MENU ) )
    {
        DEMOBeforeRender();

        // Draw scene
        DrawTick();

        // get pad status
        DEMOPadRead();
        // Control and Animation
        AnimTick();

        CheckMissingObj();
        DEMODoneRender();
    }

    OSHalt("End of test");
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes parameters and default graphics status.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void  DrawInit( void )
{
    u32  i;

    // Reflection map texture
    TEXGetPalette(&MyTplObj, "gxTests/tf-02.tpl");

#ifdef BUG_INDX_ALIGN    
    // This program depends on specific command alignment in the fifo, so
    // we force the write pipe to reset on every GXFlush.
    GXSetResetWritePipe(GX_TRUE);
#endif

    // Set background color
    GXSetCopyClear((GXColor){ 64, 64, 64, 64}, GX_MAX_Z24);

    // Set pixel processing mode
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    GXSetZMode(GX_TRUE, GX_LESS, GX_TRUE);
    GXSetZCompLoc(GX_TRUE);

    // Set culling mode
    GXSetCullMode(GX_CULL_BACK);

    // Allocate matrix array buffers (double-buffer)
    PosMtxArrayBuf[0] = (Mtx*)  OSAlloc(sizeof(Mtx)   * NUM_CONTOUR);
    PosMtxArrayBuf[1] = (Mtx*)  OSAlloc(sizeof(Mtx)   * NUM_CONTOUR);
    NrmMtxArrayBuf[0] = (Mtx33*)OSAlloc(sizeof(Mtx33) * NUM_CONTOUR);
    NrmMtxArrayBuf[1] = (Mtx33*)OSAlloc(sizeof(Mtx33) * NUM_CONTOUR);
    TexMtxArrayBuf[0] = (Mtx24*)OSAlloc(sizeof(Mtx24) * NUM_CONTOUR);
    TexMtxArrayBuf[1] = (Mtx24*)OSAlloc(sizeof(Mtx24) * NUM_CONTOUR);
    CurrentMtxBuffer  = 0;
    
    // Animation setting
    AnimReset   = 0;
    AnimPause   = 0;
    AnimCtrlPtr = 0;
    AnimCounter = 0;
    for ( i = 0; i < NUM_CONTOUR; i++ )
    {
        AnimCtrlArray[i] = (Vec){ 5.0F, 0.5F, 0.0F };
    }
    // Initial location of the object
    MTXTrans(AnimMtxOrigin, -800.0F, 0.0F, -400.0F);
    MTXIdentity(AnimGlobalRot);

    // Build matrix array for the first frame
    // (for safety, initialize both of double-buffer)
    BuildMtxArray();
    BuildMtxArray();

    // Build a model display list
    BuildModelDL();

    return;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void  DrawTick( void )
{
    Mtx*            posMtxArray = PosMtxArrayBuf[CurrentMtxBuffer];
    Mtx33*          nrmMtxArray = NrmMtxArrayBuf[CurrentMtxBuffer];
    Mtx24*          texMtxArray = TexMtxArrayBuf[CurrentMtxBuffer];
    Mtx44           projMtx;
    MyCameraObj*    c = &DefaultCamera;
    GXTexObj        texObj;
    GXLightObj      litObj;
    Vec             lpos = { 10000.0F, -10000.0F, 10000.0F };

    // Set projection matrix
    MTXPerspective(projMtx, c->fovy, ASPECT, c->znear, c->zfar);
    GXSetProjection(projMtx, GX_PERSPECTIVE);

    // Lighting
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    
    GXSetChanAmbColor(GX_COLOR0A0, (GXColor){ 48,  48,  48,  48 });
    GXSetChanMatColor(GX_COLOR0A0, (GXColor){ 255, 255, 192, 255 });

    MTXMultVec(c->viewMtx, &lpos, &lpos);
    GXInitLightPosv(&litObj, &lpos);
    GXInitLightColor(&litObj, (GXColor){ 208, 208, 208, 208 });
    GXLoadLightObjImm(&litObj, GX_LIGHT0);

    // TEV
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    // Texture
    TEXGetGXTexObjFromPalette(MyTplObj, &texObj, 1);
    GXLoadTexObj(&texObj, GX_TEXMAP0);
    // Texture coord gen    
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0);
    
    // Array base & stride setting
    GXSetArray(GX_POS_MTX_ARRAY, posMtxArray,     sizeof(posMtxArray[0]));
    GXSetArray(GX_NRM_MTX_ARRAY, nrmMtxArray,     sizeof(nrmMtxArray[0]));
    GXSetArray(GX_TEX_MTX_ARRAY, texMtxArray,     sizeof(texMtxArray[0]));
    GXSetArray(GX_VA_POS,        &VtxArray[0].x,  sizeof(VtxArray[0]));
    GXSetArray(GX_VA_NRM,        &VtxArray[0].nx, sizeof(VtxArray[0]));
    
    // Vertex attribute/descriptor settings
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY,  GX_S8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX,   GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0MTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS,        GX_INDEX8);
    GXSetVtxDesc(GX_VA_NRM,        GX_INDEX8);

    // Draw the model
    GXCallDisplayList(ModelDLBuffer, ModelDLSize);

    return;
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Updates scene animation parameter by using PAD status.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void AnimTick( void )
{
    Mtx     tmpMtx0, tmpMtx1, tmpMtx2;
    f32     dir0, dir1;

    // Reset location
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_A )
    {
        AnimReset = 1;
    }

    // Pause
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_B )
    {
        AnimPause = 1 - AnimPause;
    }

    // Global model rotation calculation
    MTXRotDeg(tmpMtx1, 'Z', ( (f32)DEMOPadGetSubStickX(0) / 32 + 0.20F));
    MTXRotDeg(tmpMtx0, 'X', (-(f32)DEMOPadGetSubStickY(0) / 32));
    MTXConcat(tmpMtx1, AnimGlobalRot, AnimGlobalRot);
    MTXConcat(tmpMtx0, AnimGlobalRot, AnimGlobalRot);

    // Increment animation counter
    AnimCounter = ( AnimCounter + 1 ) % 180;

    // Reset location when the object goes somewhere outside the screen.
    if ( AnimReset )
    {
        MTXIdentity(AnimMtxOrigin);
        AnimReset = 0;
    }
    
    // Update matrix for origin (end point) of the object.
    if ( !AnimPause )
    {
        MTXTrans(tmpMtx0, 0.0F, 0.0F, CONTOUR_SPAN);
        MTXRotDeg(tmpMtx1, 'Y', AnimCtrlArray[AnimCtrlPtr].x);
        MTXConcat(tmpMtx1, tmpMtx0, tmpMtx2);
        MTXRotDeg(tmpMtx1, 'Z', AnimCtrlArray[AnimCtrlPtr].y);
        MTXConcat(tmpMtx1, tmpMtx2, tmpMtx0);
        MTXConcat(AnimMtxOrigin, tmpMtx0, AnimMtxOrigin);
        
        // Get new parameter for the top point.
        dir0 = 7.5F * sinf((f32)AnimCounter * 3.1415F / 90) + 2.5F;
        dir1 = 1.0F * sinf((f32)AnimCounter * 3.1415F / 45);
        AnimCtrlArray[AnimCtrlPtr].x = (f32)(DEMOPadGetStickX(0)/5) + dir0;
        AnimCtrlArray[AnimCtrlPtr].y = (f32)(DEMOPadGetStickY(0)/5) + dir1;
    
        // Advance pointer of ring buffer.
        AnimCtrlPtr = (AnimCtrlPtr + 1) % NUM_CONTOUR;
    }

    // Build matrix array
    BuildMtxArray();
}

/*---------------------------------------------------------------------------*
    Name:           BuildModelDL
    
    Description:    Build a display list for stitched pipe model.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*
    This function puts these three command APIs into the display list:

        GXLoadPosMtxIndx / GXLoadTexMtxIndx / GXLoadNrmMtxIndx3x3

    that are basically safe with using inside a display list. Since actual
    matrix loads will occur when you call this display list, you can
    animate this object by just updating matrix array on main memory.
 *---------------------------------------------------------------------------*/
void BuildModelDL( void )
{
    s32          i, j, k;
    u8           vi;
    GXPosNrmMtx  pm0, pm1;
    GXTexMtx     tm0, tm1;

    //---- allocate buffer for the display list
    if ( ModelDLBuffer == NULL )
    {
        ModelDLBuffer = OSAlloc(DL_BUFSIZE);
        DCInvalidateRange(ModelDLBuffer, DL_BUFSIZE);
    }

    //---- start generating display list
    GXBeginDisplayList(ModelDLBuffer, DL_BUFSIZE);
    

    //-------- Pipe lid
    pm0 = GX_PNMTX0;
    tm0 = GX_TEXMTX0;
    GXLoadPosMtxIndx   (0, pm0);
    GXLoadNrmMtxIndx3x3(0, pm0);
    GXLoadTexMtxIndx   (0, tm0, GX_MTX2x4);

    GXBegin( GX_TRIANGLEFAN, GX_VTXFMT0, VTX_PER_CONTOUR );
    for ( i = 0; i < VTX_PER_CONTOUR; i ++ )
    {
        GXMatrixIndex1x8((u8)pm0);
        GXMatrixIndex1x8((u8)tm0);
        GXPosition1x8   ((u8)i);
        GXNormal1x8     ((u8)i);
    }
    GXEnd( );
    
    //-------- Pipe body
    for ( i = 0; i < NUM_CONTOUR-1; i += (NUM_MTX_SLOTS-1) )
    {
        // Load 10 transform matrices into HW slots.
        //
        // It is better to load matrices as much as possible at one
        // time to minimize the stall latency of graphics pipeline.
        pm0 = GX_PNMTX0;
        tm0 = GX_TEXMTX0;
        for ( j = 0 ; j < NUM_MTX_SLOTS ; j++ )
        {
            GXLoadPosMtxIndx   ((u16)(i+j), pm0);
            GXLoadNrmMtxIndx3x3((u16)(i+j), pm0);
            GXLoadTexMtxIndx   ((u16)(i+j), tm0, GX_MTX2x4);
            pm0 = (GXPosNrmMtx)(pm0 + MTX_IDX_SPAN);
            tm0 = (GXTexMtx)(tm0 + MTX_IDX_SPAN);
        }
        
        // Stich all transformed contour.
        pm0 = GX_PNMTX0;
        tm0 = GX_TEXMTX0;
        for ( j = 0 ; j < (NUM_MTX_SLOTS-1) ; j++ )
        {
            pm1 = (GXPosNrmMtx)(pm0 + MTX_IDX_SPAN);
            tm1 = (GXTexMtx)(tm0 + MTX_IDX_SPAN);
        
            GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (VTX_PER_CONTOUR+1)*2);
            vi = VTX_PER_CONTOUR-1;
            for ( k = -1; k < VTX_PER_CONTOUR; k++ )
            {
                GXMatrixIndex1x8((u8)pm0);
                GXMatrixIndex1x8((u8)tm0);
                GXPosition1x8   (vi);
                GXNormal1x8     (vi);
                GXMatrixIndex1x8((u8)pm1);
                GXMatrixIndex1x8((u8)tm1);
                GXPosition1x8   (vi);
                GXNormal1x8     (vi);
                vi = (u8)(k + 1);
            }
            GXEnd();
            
            pm0 = pm1;
            tm0 = tm1;
        }
    }
    
    //-------- Pipe lid 1
    pm1 = GX_PNMTX9;
    tm1 = GX_TEXMTX9;
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, VTX_PER_CONTOUR);
    for ( i = 0; i < VTX_PER_CONTOUR; i ++ )
    {
        GXMatrixIndex1x8((u8)pm1);
        GXMatrixIndex1x8((u8)tm1);
        GXPosition1x8   ((u8)(VTX_PER_CONTOUR-1-i));
        GXNormal1x8     ((u8)(VTX_PER_CONTOUR-1-i));
    }
    GXEnd();
    

    //---- end of the display list
    ModelDLSize = GXEndDisplayList();
    if ( !ModelDLSize )
    {
        OSHalt("Display list exceeds size of the buffer\n");
    }
    OSReport("Size = %d\n", ModelDLSize);
}

/*---------------------------------------------------------------------------*
    Name:           BuildMtxArray
    
    Description:    Build matrix array for animation.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void BuildMtxArray( void )
{
    MyCameraObj* c = &DefaultCamera;
    Mtx*    posMtxArray;
    Mtx33*  nrmMtxArray;
    Mtx24*  texMtxArray;
    Mtx     tmpMtx0, tmpMtx1, tmpMtx2;
    u32     i, j;
    Vec     angles;
    f32     nfct;

    // Switch matrix array buffer
    //
    //    Using double-buffering is good to gain performance.
    //    This function builds up matrix data for the next frame into
    //    one buffer while the GP may be drawing object by using matrix
    //    data from another buffer.
    CurrentMtxBuffer = 1 - CurrentMtxBuffer;
    posMtxArray = PosMtxArrayBuf[CurrentMtxBuffer];
    nrmMtxArray = NrmMtxArrayBuf[CurrentMtxBuffer];
    texMtxArray = TexMtxArrayBuf[CurrentMtxBuffer];


    // Get View matrix
    MTXLookAt(c->viewMtx, &c->position, &c->up, &c->target);
    
    // Build position matrix array
    MTXConcat(c->viewMtx, AnimGlobalRot, tmpMtx0);
    MTXConcat(tmpMtx0, AnimMtxOrigin, posMtxArray[0]);
    for ( i = 1; i < NUM_CONTOUR; i++ )
    {
        angles = AnimCtrlArray[(i+AnimCtrlPtr-1)%NUM_CONTOUR];
    
        MTXTrans(tmpMtx0, 0.0F, 0.0F, 50.0F);
        MTXRotDeg(tmpMtx1, 'Y', angles.x);
        MTXConcat(tmpMtx1, tmpMtx0, tmpMtx2);
        MTXRotDeg(tmpMtx1, 'Z', angles.y);
        MTXConcat(tmpMtx1, tmpMtx2, tmpMtx0);
        MTXConcat(posMtxArray[i-1], tmpMtx0, posMtxArray[i]);
    }

    // Build normalmal matrix array
    for ( i = 0; i < NUM_CONTOUR; i++ )
    {
        // No need to calculate inverse-transpose because these
        // matrices don't contain anisotropic scale factor.
        
        // Copy 3x3 part to normal matrix arry.
        CopyMtxTo3x3( posMtxArray[i], nrmMtxArray[i] );
    }
    
    // Build texture matrix array for environment mapping
    for ( i = 0; i < NUM_CONTOUR; i++ )
    {
        for ( j = 0; j < 2; j++ )
        {
            nfct = 0.5F / VECMag( (VecPtr)posMtxArray[i][0] );
            if ( j == 1 ) nfct = -nfct;
            texMtxArray[i][j][0] = posMtxArray[i][j][0] * nfct;
            texMtxArray[i][j][1] = posMtxArray[i][j][1] * nfct;
            texMtxArray[i][j][2] = posMtxArray[i][j][2] * nfct;
            texMtxArray[i][j][3] = 0.5f;
        }
    }
    DCFlushRange(posMtxArray, sizeof(Mtx)   * NUM_CONTOUR);
    DCFlushRange(nrmMtxArray, sizeof(Mtx33) * NUM_CONTOUR);
    DCFlushRange(texMtxArray, sizeof(Mtx24) * NUM_CONTOUR);
}

/*---------------------------------------------------------------------------*
    Name:           CopyMtxTo3x3
    
    Description:    Copy 3x3 portion from 3x4 matrix to another place
                    in order to use GXLoadNrmMtxIndx3x3.
                    
    Arguments:      src : source 3x4 matrix.
                    dst : destination 3x3 matrix.
    
    Returns:        none
 *---------------------------------------------------------------------------*/
inline void CopyMtxTo3x3( Mtx src, Mtx33 dst )
{
    dst[0][0] = src[0][0];  dst[0][1] = src[0][1];  dst[0][2] = src[0][2];
    dst[1][0] = src[1][0];  dst[1][1] = src[1][1];  dst[1][2] = src[1][2];
    dst[2][0] = src[2][0];  dst[2][1] = src[2][1];  dst[2][2] = src[2][2];
}

/*---------------------------------------------------------------------------*
    Name:           CheckMissingObj
    
    Description:    Check if the object is drawn on the screen.
                    (The object has gone somewhere outside.)
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void CheckMissingObj( void )
{
    u16  bt, bb, br, bl;

    // Synchronization
    GXDrawDone();
    // Get bounding box
    GXReadBoundingBox(&bl, &bt, &br, &bb);

    if ( br <= 0 && bl >= 640 )
    {
        // Object lost. Need reset.
        AnimReset = 1;
    }

    // Clear bounding box (for next frame)
    GXClearBoundingBox();
}

/*===========================================================================*/