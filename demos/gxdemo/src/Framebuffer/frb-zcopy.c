/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     frb-zcopy.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Framebuffer/frb-zcopy.c $    
    
    8     5/05/01 10:05p Hirose
    removed obsolete flags
    
    7     10/25/00 9:59p Hirose
    flag change. MAC -> EMU for PC emulator compatibility
    
    6     9/27/00 4:22p Hirose
    Now it doesn't have to set GX_PF_Z24 pixel mode before Z texture copy
    
    5     5/02/00 4:03p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    4     4/20/00 9:58p Hirose
    full update (changed to use Z texture)
    
    3     3/23/00 3:24p Hirose
    updated to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:09p Alligator
    move from tests/gx and rename
    
    7     2/24/00 7:29p Hirose
    updated pad control functions to match HW1
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     2/12/00 5:16p Alligator
    Integrate ArtX source tree changes
    
    4     1/24/00 2:39p Carl
    Removed #ifdef EPPC stuff
    
    3     1/19/00 7:05p Alligator
    fix EPPC errors
    
    2     1/13/00 8:53p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    5     11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    4     11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    3     11/08/99 7:34p Hirose
    added single frame mode / depth offset control
    
    2     11/04/99 9:26p Hirose
    simple prototype
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    frb-zcopy
        Z texture copy test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/

#define PI                   3.14159265358979323846F
#define MAX_Z                0x00ffffff // max value of Z buffer
#define TEX_SIZE             128

#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

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
    s32           theta;
    s32           phi;
} MyCameraObj;

// for entire scene control
typedef struct
{
    MyCameraObj   cam;
    GXTexObj      cTexObj;
    u8*           cTexData;
    GXTexObj      zTexObj;
    u8*           zTexData;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main               ( void );
static void DrawInit           ( MySceneCtrlObj* sc );
static void DrawTick           ( MySceneCtrlObj* sc );
static void AnimTick           ( MySceneCtrlObj* sc );
static void DrawQuad           ( void );
static void DrawUnitCell       ( Mtx view );
static void DrawCells          ( Mtx view );
static void SetCamera          ( MyCameraObj* cam );
static void SetLight           ( void );
static void DisableLight       ( void );
static void PrintIntro         ( void );

/*---------------------------------------------------------------------------*
   Model and texture data
 *---------------------------------------------------------------------------*/

#define REG_AMBIENT  ColorArray[1]
#define LIGHT_COLOR  ColorArray[2]
#define BG_COLOR     ColorArray[3]
#define REG_MAT0     ColorArray[4]
#define REG_MAT1     ColorArray[5]
#define REG_MAT2     ColorArray[6]
#define REG_MAT3     ColorArray[7]
#define REG_MAT4     ColorArray[8]

static GXColor ColorArray[] ATTRIBUTE_ALIGN(32) = 
{
    { 0x00, 0x00, 0x00, 0x00 },    // Black
    { 0x40, 0x40, 0x40, 0xFF },    // Gray  (Ambient etc.)
    { 0xFF, 0xFF, 0xFF, 0xFF },    // White (Light etc.)
    { 0x20, 0x20, 0x20, 0x00 },    // Background
    { 0xFF, 0xFF, 0xFF, 0xFF },    // Material 0
    { 0xFF, 0x80, 0xFF, 0xFF },    // Material 1
    { 0x80, 0xFF, 0xFF, 0xFF },    // Material 2
    { 0xFF, 0xFF, 0x80, 0xFF },    // Material 3
    { 0x40, 0x40, 0xFF, 0xFF },    // Material 4
};

#define NUM_CELLS  20
static Vec CellLocTbl[NUM_CELLS] =
{
    {  2.5F,  1.5F, -1.5F },
    { -0.5F,  1.5F, -1.5F },
    { -3.5F,  1.5F, -1.5F },
    { -6.5F,  1.5F, -1.5F },
    {  2.5F,  1.5F,  1.5F },
    { -0.5F,  1.5F,  1.5F },
    { -3.5F,  1.5F,  1.5F },
    { -6.5F,  1.5F,  1.5F },
    { -2.0F,  0.0F,  0.0F },
    { -5.0F,  0.0F,  0.0F },
    { -8.0F,  0.0F,  0.0F },
    { -2.0F,  3.0F,  0.0F },
    {  1.0F,  3.0F,  0.0F },
    {  4.0F,  0.0F,  0.0F },
    {  5.5F, -1.5F, -1.5F },
    {  5.5F, -1.5F,  1.5F },
    {  7.0F, -3.0F, -3.0F },
    {  7.0F, -3.0F,  3.0F },
    { -2.0F,  0.0F, -3.0F },
    { -2.0F,  0.0F,  3.0F }
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig UnitObjCamera =
{
    {   0.0F, 0.0F,  2.0F }, // location (actually not used)
    {   0.0F, 1.0F,  0.0F }, // up
    {   0.0F, 0.0F,  0.0F }, // target
     -2.0F,  // left
      2.0F,  // top
      0.0F,  // near
    128.0F   // far
};

static CameraConfig SceneCamera =
{
    {   0.0F, 0.0F, 16.0F }, // location (actually not used)
    {   0.0F, 1.0F,  0.0F }, // up
    {   0.0F, 0.0F,  0.0F }, // target
      -2.0F,  // left (actually not used)
       2.0F,  // top  (actually not used)
       0.0F,  // near
     128.0F   // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl; // scene control parameters

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

    OSHalt("End of test");
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
static void  DrawInit( MySceneCtrlObj* sc )
{
    u32  size;

    //  Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // Pixel mode, Z compare, alpha compare and background color
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_EQUAL, 255, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCopyClear(BG_COLOR, MAX_Z);
    
    // Clear background by specified color at first
    // The operation can be done by dummy display copy.
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);

    
    // Default scene control parameter settings

    // camera
    sc->cam.theta = 0;
    sc->cam.phi   = 0;

    // Texture object for RGBA copy
    size = GXGetTexBufferSize(TEX_SIZE, TEX_SIZE, GX_TF_RGBA8, GX_FALSE, 0);
    sc->cTexData = OSAlloc(size);
    GXInitTexObj(&sc->cTexObj, sc->cTexData, TEX_SIZE, TEX_SIZE,
                 GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, GX_FALSE);
    GXInitTexObjLOD(&sc->cTexObj, GX_NEAR, GX_NEAR, 0, 0, 0,
                    GX_FALSE, GX_FALSE, GX_ANISO_1);
    
    // Texture object for Z copy
    size = GXGetTexBufferSize(TEX_SIZE, TEX_SIZE, GX_TF_Z24X8, GX_FALSE, 0);
    sc->zTexData = OSAlloc(size);
    GXInitTexObj(&sc->zTexObj, sc->zTexData, TEX_SIZE, TEX_SIZE,
                 GX_TF_Z24X8, GX_REPEAT, GX_REPEAT, GX_FALSE);
    GXInitTexObjLOD(&sc->zTexObj, GX_NEAR, GX_NEAR, 0, 0, 0,
                    GX_FALSE, GX_FALSE, GX_ANISO_1);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the scene by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    GXRenderModeObj *rmp;
    Vec  camPos;
    u16  scrWd, scrHt;
    f32  r_theta, r_phi;

    // get current rendering mode structure
    rmp   = DEMOGetRenderModeObj();
    scrWd = rmp->fbWidth;
    scrHt = rmp->efbHeight;

    // Texture data is changed every frame
    GXInvalidateTexAll();
    // turn off Z texture
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);

    // pre-calculate camera orientation
    r_theta = sc->cam.theta * PI / 180.0F;
    r_phi   = sc->cam.phi   * PI / 180.0F;
    camPos.x = cosf(r_phi) * sinf(r_theta);
    camPos.y = sinf(r_phi);
    camPos.z = cosf(r_phi) * cosf(r_theta);

    
    //-------------------------------------------
    //    Make a image for texture
    //-------------------------------------------

    // Set up viewport and camera
    GXSetViewport(0, 0, TEX_SIZE, TEX_SIZE, 0.0F, 1.0F);
    
    sc->cam.cfg = UnitObjCamera;
    VECScale(&camPos, &sc->cam.cfg.location, 2.0F);
    SetCamera(&sc->cam);
    
    // enable lighting
    SetLight();

    // Draw an unit model
    DrawUnitCell(sc->cam.view);
    
    
    //-------------------------------------------
    //    Copy RGBA / Z image into Texture
    //-------------------------------------------

    // Turn off vertical de-flicker filter temporary
    // (to avoid filtering during framebuffer-to-texture copy)
    GXSetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);
    
    // Copy Z image
    GXSetTexCopySrc(0, 0, TEX_SIZE, TEX_SIZE);
    GXSetTexCopyDst(TEX_SIZE, TEX_SIZE, GX_TF_Z24X8, GX_FALSE);
    GXCopyTex(sc->zTexData, GX_FALSE);

    // Copy RGBA image
    GXSetTexCopySrc(0, 0, TEX_SIZE, TEX_SIZE);
    GXSetTexCopyDst(TEX_SIZE, TEX_SIZE, GX_TF_RGBA8, GX_FALSE);
    GXCopyTex(sc->cTexData, GX_TRUE);

    // Wait for finishing all rendering task in the graphics pipeline
	// while allowing CPU to continue
	GXPixModeSync();

    // Restore vertical de-flicker filter mode
    GXSetCopyFilter(rmp->aa, rmp->sample_pattern, GX_TRUE, rmp->vfilter);

    //-------------------------------------------
    //    Make Main Image
    //-------------------------------------------
    // Pixel mode for actual image
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);

    // Set up viewport and camera
    GXSetViewport(0, 0, scrWd, scrHt, 0.0F, 1.0F);
    sc->cam.cfg = SceneCamera;
    sc->cam.cfg.left = - (f32)scrWd * 2.0F / TEX_SIZE;
    sc->cam.cfg.top  =   (f32)scrHt * 2.0F / TEX_SIZE;
    VECScale(&camPos, &sc->cam.cfg.location, 16.0F);
    SetCamera(&sc->cam);

    // Disable lighting
    DisableLight();

    // Loads generated Z texture
    GXLoadTexObj(&sc->cTexObj, GX_TEXMAP0);
#ifndef EMU
    GXLoadTexObj(&sc->zTexObj, GX_TEXMAP1);
#endif

    // Draw billboards
    DrawCells(sc->cam.view);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    // camera position
    sc->cam.theta += ( DEMOPadGetStickX(0) / 24 );
    sc->cam.theta = ( sc->cam.theta + 360 ) % 360;
    
    sc->cam.phi   += ( DEMOPadGetStickY(0) / 24 );
    Clamp(sc->cam.phi, -80, 80);

}

/*---------------------------------------------------------------------------*
    Name:           DrawQuad
    
    Description:    Draw a textured quad
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawQuad( void )
{
    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16( -2,  2, 0 );
        GXTexCoord2s8( 0, 0 );
        GXPosition3s16(  2,  2, 0 );
        GXTexCoord2s8( 1, 0 );
        GXPosition3s16(  2, -2, 0 );
        GXTexCoord2s8( 1, 1 );
        GXPosition3s16( -2, -2, 0 );
        GXTexCoord2s8( 0, 1 );
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawUnitCell
    
    Description:    Draw an unit model
                    
    Arguments:      view : view matrix
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawUnitCell( Mtx view )
{
    Mtx  ms, mr, mt, mv, mvi;
    s32  i, j, x, y, z;

    // set shading mode to use lit vertex color only
    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
	GXSetNumChans(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);


    // Eight corners
    GXSetChanMatColor(GX_COLOR0A0, REG_MAT0);
    MTXScale(ms, 0.25F, 0.25F, 0.25F);
    for ( i = 0 ; i < 8 ; ++i )
    {
        x = ( i & 1 ) ? -1 : 1;
        y = ( i & 2 ) ? -1 : 1;
        z = ( i & 4 ) ? -1 : 1;
        
        MTXTrans(mt, x, y, z);
        MTXConcat(mt, ms, mv);
        MTXConcat(view, mv, mv);
        
        GXLoadPosMtxImm(mv, GX_PNMTX0);
        MTXInverse(mv, mvi);
        MTXTranspose(mvi, mv);
        GXLoadNrmMtxImm(mv, GX_PNMTX0);
        
        GXDrawSphere1(2);
    }

    // Twelve edges
    MTXScale(ms, 0.15F, 0.15F, 1.0F);
    for ( i = 0 ; i < 3 ; ++i )
    {
        switch(i)
        {
          case 0 :
            GXSetChanMatColor(GX_COLOR0A0, REG_MAT1);
            MTXIdentity(mr);
            break;
          case 1 :
            GXSetChanMatColor(GX_COLOR0A0, REG_MAT2);
            MTXRotDeg(mr, 'x', 90);
            break;
          case 2 :
            GXSetChanMatColor(GX_COLOR0A0, REG_MAT3);
            MTXRotDeg(mr, 'y', 90);
            break;
        }
    
        for ( j = 0 ; j < 4 ; ++j )
        {
            x = ( j & 1 ) ? -1 : 1;
            y = ( j & 2 ) ? -1 : 1;
            z = 0;
            
            MTXTrans(mt, x, y, z);
            MTXConcat(mt, ms, mv);
            MTXConcat(mr, mv, mt);
            MTXConcat(view, mt, mv);
            
            GXLoadPosMtxImm(mv, GX_PNMTX0);
            MTXInverse(mv, mvi);
            MTXTranspose(mvi, mv);
            GXLoadNrmMtxImm(mv, GX_PNMTX0);
            
            GXDrawCylinder(16);
        }
    }

    // Center sphere
    GXSetChanMatColor(GX_COLOR0A0, REG_MAT4);
    MTXScale(ms, 0.40F, 0.40F, 0.40F);
    MTXConcat(view, ms, mv);
    
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi);
    MTXTranspose(mvi, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    
    GXDrawSphere1(2);
}

/*---------------------------------------------------------------------------*
    Name:           DrawCells
        
    Description:    Draw cells which position is defined in the table.
                    Each cell is drawn by using a billboard with run-time
                    rendered RGBA/Z texture.
                    
    Arguments:      view : view matrix
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawCells( Mtx view )
{
    Vec  loc;
    Mtx  mt;
    u32  i;

#ifndef EMU

    // set shading mode to use texture / Z texture
    GXSetNumTevStages(2);
    GXSetNumTexGens(1);
	GXSetNumChans(0);
	
	GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    // The second stage uses texture input only for Z textureing.
    // So this stage simply passes previous stage output.
    GXSetTevOp(GX_TEVSTAGE1, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);

    GXSetZTexture(GX_ZT_ADD, GX_TF_Z24X8, 0);

#else // EMU

    // the emulator isn't able to use Z texture
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
	GXSetNumChans(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

#endif // EMU


    for ( i = 0 ; i < NUM_CELLS ; ++i )
    {
        // modelview matrix for billboards
        // location of each cell is defined in the table.
        MTXMultVec(view, &CellLocTbl[i], &loc);
        MTXTrans(mt, loc.x, loc.y, loc.z);
        GXLoadPosMtxImm(mt, GX_PNMTX0);
        
        DrawQuad();
    }

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
    Name:           SetLight
    
    Description:    Sets light objects and color channels
                    
    Arguments:      le   : pointer to a MyLightEnvObj structure
                    view : view matrix.
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetLight( void )
{
    GXLightObj  lobj;

    // set up light position and color
    GXInitLightPos(&lobj, 0.0F, 10000.0F, 10000.0F); // almost parallel
    GXInitLightColor(&lobj, LIGHT_COLOR);
    GXLoadLightObjImm(&lobj, GX_LIGHT0);

    // channel setting
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,        // enable channel
        GX_SRC_REG,       // amb source
        GX_SRC_REG,       // mat source
        GX_LIGHT0,        // light mask
        GX_DF_CLAMP,      // diffuse function
        GX_AF_NONE);      // attenuation function
    // channel ambient
    GXSetChanAmbColor(GX_COLOR0A0, REG_AMBIENT);
}

/*---------------------------------------------------------------------------*
    Name:           DisableLight
    
    Description:    Disables lighting
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DisableLight( void )
{
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,    // disable channel
        GX_SRC_REG,    // amb source
        GX_SRC_VTX,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_NONE,    // diffuse function
        GX_AF_NONE);
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
    OSReport("frb-zcopy: Z texture copy from EFB\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick: move camera\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
