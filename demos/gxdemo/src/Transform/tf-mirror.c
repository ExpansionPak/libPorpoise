/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tf-mirror.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Transform/tf-mirror.c $
    
    7     5/10/01 5:07a Hirose
    removed obsolete flags
    
    6     8/25/00 5:41p Carl
    Fixed reflection image.
    Lots of misc. changes.
    
    5     7/07/00 5:57p Dante
    PC Compatibility
    
    4     3/24/00 5:46p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/21/00 3:24p Hirose
    deleted PADInit() call because this function is called once in
    DEMOInit()
    
    1     3/06/00 12:14p Alligator
    move from tests/gx and rename
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     2/12/00 5:16p Alligator
    Integrate ArtX source tree changes
    
    4     1/25/00 3:00p Carl
    Changed to standardized end of test message
    
    3     1/18/00 6:15p Alligator
    fix to work with new GXInit defaults
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    4     11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    3     11/15/99 4:49p Yasu
    Change datafile name
    
    2     11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    1     11/12/99 1:10p Alligator
    demo of planar mirror
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/

#define SIDE     35
#define MIRRORHT 175.0f
#define MIRRORWD 150.0f
#define NORM     (sqrtf(3.0f)/3.0f)

#define ASPECT  (10.0f/7.0f)
#define FOVY    (45.0f)
#define XCENTER (320)
#define YCENTER (224)

#define PI 3.1415926535f

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main            ( void );

static void CameraInit      ( void );
static void DrawInit        ( void );
static void DrawTick        ( void );
static void AnimTick        ( void );

static void DrawScene       ( GXBool mirror );
static void GetMirrorMv     ( VecPtr trans, VecPtr rot, Mtx mv );
static void DrawMirror      ( void );
static void SendVertex      ( u16 posIndex, u16 texCoordIndex );

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

Mtx v;                  // viewing (camera) matrix
u32 rot = 45;           // cube rotation
Vec mirrRot = { 0.0f, -45.0f, 0.0f }; // mirror rotation on each axis
Vec mirrTrans = { 0, 0, 0 }; // mirror translation

u8 CurrentControl;      // control for reflected texture scale
u8 VpScale = 1;         // actual value for reflected texture scale
u8 DoRotation = 1;      // controls cube rotation
u8 testing = 0;         // if true, show only the computed reflection

GXTexObj face_obj;      // texture object for cube faces
GXTexObj mirr_obj;      // texture object for mirror contents

u8* MirrTexData = NULL; // texture data for mirror contents

f32 FloatVert[] ATTRIBUTE_ALIGN(32) = { // cube verts
    -SIDE,  SIDE, -SIDE,
    -SIDE,  SIDE,  SIDE,
    -SIDE, -SIDE,  SIDE,
    -SIDE, -SIDE, -SIDE,
     SIDE,  SIDE, -SIDE,
     SIDE, -SIDE, -SIDE,
     SIDE, -SIDE,  SIDE,
     SIDE,  SIDE,  SIDE,
};

f32 MirrorVert[] = {                    // mirror verts
    -MIRRORWD,     0.0f, 0.0f,
    -MIRRORWD, MIRRORHT, 0.0f,
     MIRRORWD, MIRRORHT, 0.0f,
     MIRRORWD,     0.0f, 0.0f,
};

f32 FloatNorm[] ATTRIBUTE_ALIGN(32) = { // cube normals; adjusted by app
    -1,  1, -1,
    -1,  1,  1,
    -1, -1,  1,
    -1, -1, -1,
     1,  1, -1,
     1, -1, -1,
     1, -1,  1,
     1,  1,  1,
};

f32 FloatTex[] ATTRIBUTE_ALIGN(32) = {  // cube face texcoords
    0.0F, 1.0F,
    0.0F, 0.0F, 
    1.0F, 0.0F,
    1.0F, 1.0F, 
};

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{
    DEMOInit(NULL);
           
    DrawInit();            // Define my vertex formats and set array pointers.

    DEMOPadRead();         // Read the joystick for this frame

    // While the quit button is not pressed
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))    
    {            
        DEMOPadRead();     // Read the joystick for this frame

        // Do animation based on input
        AnimTick();    
        DEMOBeforeRender();
        
        DrawTick();        // Draw the scene
   
        DEMODoneRender();
    }

    OSHalt("End of test");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.
                    Initialize camera matrix.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit         ( void )
{
    Mtx44 p;
    Vec camPt = {0.0F, 50.0F, 650.0F};
    Vec up = {0.0F, 1.0F, 0.0F};
    Vec origin = {0.0F, 0.0F, 0.0F};
    
    MTXPerspective(p, FOVY, ASPECT, 100, 2000);

    GXSetProjection(p, GX_PERSPECTIVE);

    MTXLookAt(v, &camPt, &up, &origin);    
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Graphics initialization function for the current model.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{
    TEXPalettePtr tpl = 0;
    u32           i;
    GXColor       blue_opa = {0x00, 0x00, 0xa0, 0xff};

    CameraInit();    // Initialize the camera.

    // Set up VAT, array base pointers

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX0, FloatTex, 8);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_NRM, FloatNorm, 12);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_POS, FloatVert, 12);

    // Load cube texture

    TEXGetPalette(&tpl, "gxTests/tf-02.tpl");

    TEXGetGXTexObjFromPalette(tpl, &face_obj, 0);

    // Disable lighting, set up color channel

    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,  // use material color
        GX_SRC_REG,  // --from material register
        GX_SRC_REG,  // not used
        GX_LIGHT0,
        GX_DF_NONE,
        GX_AF_NONE );
    GXSetChanMatColor(
        GX_COLOR0A0,
        blue_opa );

    // Scale normals to unit length (not really necessary, but a good idea)
    for(i = 0; i < 24; i++)
    {
        FloatNorm[i] *= NORM;
    }
    // Important: data must be in main memory for GP to see it
    DCFlushRange( (void *) FloatNorm, sizeof(f32)*24 );

    // Allocate mirror texture buffer
    MirrTexData = (u8*)OSAlloc(
        GXGetTexBufferSize(
            XCENTER*2, // maximum screen width
            YCENTER*2, // maximum screen height
            GX_TF_I8, 
            GX_FALSE,  // no mipmap
            0) );
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the current model once.  
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( void )
{    
    u32 i;          // misc index
    f32 p[7];       // current projection matrix
    f32 vp[6];      // current viewport
    Vec sc[4];      // screen coords of mirror
    s16 minx, maxx; // x bounds of mirror
    s16 miny, maxy; // y bounds of mirror
    u16 width;      // mirror width
    u16 height;     // mirror height
    f32 x1, x2, y1, y2; // used for texture projection matrix calculation
    Mtx mv;         // modelview mtx
    Mtx mt;         // texture proj mtx
    static u16 count = 0;   // drawsync counter

    //
    // Compute bounding box for mirrored render
    //
    GXGetProjectionv(p);
    GXGetViewportv(vp);

    vp[2] /= VpScale;   // scale VP width
    vp[3] /= VpScale;   // scale VP height

    // Construct a matrix for viewing the mirror
    GetMirrorMv( &mirrTrans, &mirrRot, mv );
    MTXConcat(v, mv, mv); // concat with viewing matrix
    
    // Project mirror corners to screen space
    for (i = 0; i < 4; i++)
    {
        GXProject(MirrorVert[i*3], MirrorVert[i*3+1], MirrorVert[i*3+2],
                  mv, p, vp,
                  &sc[i].x, &sc[i].y, &sc[i].z);
    }

    // Get bounding box of texture to copy
    minx = maxx = (s16)sc[0].x;
    miny = maxy = (s16)sc[0].y;
    for (i = 1; i < 4; i++)
    {
        if (minx > (s16)sc[i].x)
            minx = (s16)sc[i].x;
        if (maxx < (s16)sc[i].x)
            maxx = (s16)sc[i].x;
        if (miny > (s16)sc[i].y)
            miny = (s16)sc[i].y;
        if (maxy < (s16)sc[i].y)
            maxy = (s16)sc[i].y;
    }

    // "Clip" against screen boundaries
    if (miny < 0)
        miny = 0;
    if (minx < 0)
        minx = 0;
    if (maxx > XCENTER*2-1)
        maxx = XCENTER*2-1;
    if (maxy > YCENTER*2-1)
        maxy = YCENTER*2-1;

    // Adjust coordinates to be a multiple of 2 (necessary for copy)
    minx = (s16) (minx & (~1)); // round down
    miny = (s16) (miny & (~1));
    maxx = (s16) (maxx | ( 1)); // round up
    maxy = (s16) (maxy | ( 1));

    // Get width and height of texture
    width  = (u16)(maxx - minx + 1);
    height = (u16)(maxy - miny + 1);

    // Compute texture projection matrix
    // First, return min/max's to unit projection space
    x1 = ((f32) minx / (XCENTER/VpScale) ) - 1.0f;
    x2 = ((f32) maxx / (XCENTER/VpScale) ) - 1.0f;
    y1 = ((f32) miny / (YCENTER/VpScale) ) - 1.0f;
    y2 = ((f32) maxy / (YCENTER/VpScale) ) - 1.0f;
    // Adjust such that 0-1 range encompasses viewport used to make texture
    MTXLightPerspective(mt, FOVY, ASPECT,
                        1.0f/(x2-x1), -1.0f/(y2-y1),
                        -x1/(x2-x1), -y1/(y2-y1));
    // Concat modelview matrix
    MTXConcat(mt, mv, mt);
    GXLoadTexMtxImm(mt, GX_TEXMTX0, GX_MTX3x4);

    //
    // Set viewport and scissor for mirror
    //
    GXSetViewport(vp[0], vp[1], vp[2], vp[3], vp[4], vp[5]);
    GXSetScissor((u16)minx, (u16)miny, width, height);

    //
    // Draw mirrored scene
    //
    DrawScene(GX_TRUE);

    if (testing)
    {
        // Show only the computed reflection scene and bounding box
        Mtx mv;

        // Setup to draw bounding box
        GXSetNumTexGens(0);
        GXSetNumChans(1);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL,
                      GX_TEXMAP_NULL, GX_COLOR0A0);
        MTXIdentity(mv);
        mv[0][3] = -(XCENTER/VpScale)+0.5f;
        mv[1][1] = -1;
        mv[1][3] =  (YCENTER/VpScale)-1.5f;
        mv[2][2] = -(YCENTER/VpScale)/tanf(FOVY/2.0f*PI/180.0f);
        GXLoadPosMtxImm(mv, GX_PNMTX0);

        // Drawing bounding box
        GXBegin(GX_LINESTRIP, GX_VTXFMT0, 5);
        GXPosition3f32( minx, miny, 1 );
        GXPosition3f32( maxx, miny, 1 );
        GXPosition3f32( maxx, maxy, 1 );
        GXPosition3f32( minx, maxy, 1 );
        GXPosition3f32( minx, miny, 1 );
        GXEnd();

        return;
    }

    // See comment further below regarding this draw sync
    GXSetDrawSync(++count);

    //
    // Do as much work as possible before checking draw sync
    // to make sure texture rendering is done.
    //

    // Init mirror texture object with newly computed size
    GXInitTexObj( &mirr_obj,
                  MirrTexData,
                  width,
                  height,
                  GX_TF_I8,
                  GX_CLAMP,
                  GX_CLAMP,
                  GX_FALSE );

#if 0 // GXInitTexObjLOD not necessary, since defaults are okay
    GXInitTexObjLOD(
                  &mirr_obj,
                  GX_LINEAR,
                  GX_LINEAR,
                  0.0f,
                  0.0f,
                  0.0f,
                  GX_FALSE,
                  GX_FALSE,
                  GX_ANISO_1 );
#endif

    // Set copy source location
    GXSetTexCopySrc((u16) minx, (u16) miny, width, height);
    GXSetTexCopyDst((u16) width, (u16) height, GX_TF_I8, GX_FALSE);

    // Wait for texture to be rendered before copying to memory
    // One could also have used GXPixModeSync(), which simply
    // flushes the rasterizer.  Using draw sync allows other commands
    // to be executed in place of the flush (although in this case
    // there are very few).
    while( count != GXReadDrawSync() ) {};

    // Copy texture
    GXCopyTex(MirrTexData, GX_TRUE);
    
    //
    // Draw scene normally
    //
    
    // Adjust viewport width and height
    vp[2] *= VpScale;
    vp[3] *= VpScale;
    GXSetViewportv(vp);
    GXSetScissor((u16)vp[0], (u16)vp[1], (u16)vp[2], (u16)vp[3]);

    // Draw normal scene
    DrawScene(GX_FALSE);

    // Invalidate texture cache; could also use GXInvalidateTexRegion
    GXInvalidateTexAll();

    // Draw mirror
    DrawMirror( );
}

/*---------------------------------------------------------------------------*/
static void GetMirrorMv( VecPtr trans, VecPtr rot, Mtx mv )
{
    Mtx tm;
    MTXRotDeg(mv, 'X', rot->x);
    MTXRotDeg(tm, 'Y', rot->y);  // ignore rot->z for now
    MTXConcat(tm, mv, mv);
    MTXTrans(tm, trans->x, trans->y, trans->z);
    MTXConcat(tm, mv, mv);
}

/*---------------------------------------------------------------------------*/
static void DrawMirror ( void )
{
    s32 i;
    Mtx mv;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    GXSetNumTexGens(1);
    GXSetNumChans(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    // Set up projected texture coordinate generation
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0 );

    // Load mirror texture object
    GXLoadTexObj(&mirr_obj, GX_TEXMAP0);

    // Set up matrices
    GetMirrorMv( &mirrTrans, &mirrRot, mv );
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
#if 0 // We don't use any normals here, thus don't need these:
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
#endif

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (i = 0; i < 4; i ++) {
        GXPosition3f32(MirrorVert[i*3], MirrorVert[i*3+1], MirrorVert[i*3+2]);
    }
    GXEnd();

    //
    // Draw back of mirror
    //
    GXSetVtxDesc(GX_VA_TEX0, GX_NONE);
    GXSetNumTexGens(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (i = 3; i >= 0; i--) {
        GXPosition3f32(MirrorVert[i*3], MirrorVert[i*3+1], MirrorVert[i*3+2]);
    }
    GXEnd();
}

/*---------------------------------------------------------------------------*/
static void DrawScene ( GXBool mirror )
{
    Mtx ry, rz, mmv, mv, t, refl;
    Vec p = {0.0f, 0.0f, 0.0f};  // point on planar reflector
    Vec n = {0.0f, 0.0f, -1.0f}; // normal of planar reflector

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);

    GXSetNumTexGens(1);
    GXSetNumChans(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GXLoadTexObj(&face_obj, GX_TEXMAP0);

    // Compute model matrix
    MTXRotDeg(ry, 'Y', (float)rot);
    MTXRotDeg(rz, 'Z', (float)rot);
    MTXTrans(t, -80.0f, 150.0f, 0);
    MTXConcat(rz, ry, mv);
    MTXConcat(t, mv, mv);

    // Calculate reflection modelview matrix if in mirror space
    if (mirror)
    {
        GetMirrorMv( &mirrTrans, &mirrRot, mmv );

        MTXMultVec(mmv, &p, &p);
        MTXInverse(mmv, mmv);
        MTXTranspose(mmv, mmv);
        MTXMultVec(mmv, &n, &n);
        VECNormalize(&n, &n);

        MTXReflect(refl, &p, &n);
        MTXConcat(refl, mv, mv);

        GXSetCullMode( GX_CULL_FRONT ); // geometry is mirrored in mirror space

    } else {

        GXSetCullMode( GX_CULL_BACK ); // for normal space
    }

    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    // Draw the cube

    GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);

        SendVertex(0, 0);
        SendVertex(1, 1);
        SendVertex(2, 2);
        SendVertex(3, 3);

        SendVertex(4, 0);
        SendVertex(5, 1);
        SendVertex(6, 2);
        SendVertex(7, 3);        
                         
        SendVertex(2, 0);
        SendVertex(6, 1);
        SendVertex(5, 2);
        SendVertex(3, 3);
        
        SendVertex(1, 0);
        SendVertex(0, 1);
        SendVertex(4, 2);
        SendVertex(7, 3);   
        
        SendVertex(5, 0);
        SendVertex(4, 1);
        SendVertex(0, 2);
        SendVertex(3, 3);
        
        SendVertex(6, 0);
        SendVertex(2, 1);
        SendVertex(1, 2);
        SendVertex(7, 3);    

    GXEnd();
}

/*---------------------------------------------------------------------------*/
static void SendVertex ( u16 posIndex, u16 texCoordIndex )
{
    GXPosition1x16(posIndex);
    GXNormal1x16(posIndex);
    GXTexCoord1x16(texCoordIndex);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Animates the scene based on the joystick's state.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick ( void )
{
    u16 buttons = DEMOPadGetButtonDown(0);
    
    if (buttons & PAD_BUTTON_X)
    {
        CurrentControl ++;
        if (CurrentControl > 2)
            CurrentControl = 0;

        switch(CurrentControl)
        {
            case 0:
                OSReport("\n\n1:1 reflected texture scale\n\n");
                VpScale = 1;
                break;

            case 1:
                OSReport("\n\n1:2 reflected texture scale\n\n");
                VpScale = 2;
                break;

            case 2:
                OSReport("\n\n1:4 reflected texture scale\n\n");
                VpScale = 4;
                break;
        }
    }

    if (buttons & PAD_BUTTON_A)
    {
        testing = !testing;
    }

    if (buttons & PAD_BUTTON_B)
    {
        DoRotation = !DoRotation;
    }
    
    if (DoRotation)
    {
        rot ++;
        if (rot > 1439)
            rot = 0;
    }

    if (DEMOPadGetStickX(0) > 0)
        mirrRot.y += 2;
    else if (DEMOPadGetStickX(0) < 0)
        mirrRot.y -= 2;

    if (DEMOPadGetStickY(0) > 0)
        mirrRot.x += 2;
    else if (DEMOPadGetStickY(0) < 0)
        mirrRot.x -= 2;

    if (DEMOPadGetSubStickX(0) > 0)
        mirrTrans.x += 2;
    else if (DEMOPadGetSubStickX(0) < 0)
        mirrTrans.x -= 2;

    if (DEMOPadGetSubStickY(0) > 0)
        mirrTrans.y += 2;
    else if (DEMOPadGetSubStickY(0) < 0)
        mirrTrans.y -= 2;
}

/*===========================================================================*/