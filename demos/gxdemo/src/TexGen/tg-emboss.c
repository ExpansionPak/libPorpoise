/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-emboss.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-emboss.c $
    
    14    7/23/01 9:04p Hirose
    Fixed MTXRotAxis -> MTXRotAxisDeg.
    
    13    5/12/01 1:32a Hirose
    removed obsolete flags
    
    12    10/25/00 9:14p Hirose
    flag issue. applied EMU instead of MAC + WIN32
    
    11    7/20/00 3:36p Hirose
    removed "Imperfect" message
    
    10    7/07/00 5:57p Dante
    PC Compatibility
    
    9     6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    8     5/27/00 1:24p Alligator
    added button so all 8 texture coordinates can be used
    
    7     5/17/00 8:15p Hirose
    set appropriate Z mode before calling DEMOPuts library
    
    6     5/02/00 4:04p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    5     3/24/00 6:56p Hirose
    changed to use DEMOPad library
    
    4     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    3     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    2     3/06/00 2:29p Carl
    Fixed clamp bug again.
    
    1     3/06/00 12:12p Alligator
    move from tests/gx and rename
    
    13    3/03/00 4:21p Alligator
    integrate with ArtX source
    
    12    2/24/00 11:58p Yasu
    
    11    2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    10    2/23/00 11:35a Carl
    Fixed clamping problem.
    
    9     1/25/00 2:58p Carl
    Changed to standardized end of test message
    
    8     1/25/00 12:16p Carl
    Fixed spelling error
    
    7     1/20/00 4:44p Carl
    Removed #ifdef EPPC stuff
    
    6     1/18/00 7:36p Carl
    Another fix for new GXInit.
    
    5     1/18/00 6:55p Carl
    Fixed aspect ratio problem with EPPC
    
    4     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    3     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/10/99 4:46p Carl
    Fixed Z compare stuff.
    
    13    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    12    11/15/99 4:49p Yasu
    Change datafile name
    
    11    11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    10    11/09/99 7:48p Hirose
    added GXSetNumTexGens
    
    9     10/29/99 3:46p Hirose
    replaced GXSetTevStages(GXTevStageID) by GXSetNumTevStages(u8)
    
    8     10/29/99 3:04p Yasu
    Fix a bug with EPPC mode
    
    7     10/28/99 10:38p Yasu
    Change to read texture from own tpl file
    
    6     10/28/99 9:05p Yasu
    Add __SINGLEFRAME mode
    
    5     10/28/99 6:30p Yasu
    Fix a bug on real HW code
    
    4     10/26/99 1:34p Alligator
    change GXSetDefaultMatrix to GXSetDefaultMtx
    
    3     10/26/99 3:33a Yasu
    Add control menu 
    
    2     10/25/99 4:55a Yasu
    Fixed typo
    
    1     10/25/99 4:25a Yasu
    Initial version of bump mapping test
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>
#include "cmn-model.h"

#define SCREEN_DEPTH    128.0f
#define SCREEN_ZNEAR    0.0f    // near plane Z in screen coordinates
#define SCREEN_ZFAR     1.0f    // far  plane Z in screen coordinates
#define ZBUFFER_MAX     0x00ffffff

/*---------------------------------------------------------------------------*
   Typedefs
 *---------------------------------------------------------------------------*/
typedef struct
{
    Camera*     camera;
    ViewPort*   viewport;
    GXColor*    bgcolor;
}   Scene;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void    SceneDraw      ( Scene* );
void    SceneControl   ( Scene*, DEMOPadStatus* );
void    myInitModel    ( void );
void    myAnimeModel   ( Scene*, DEMOPadStatus* );
void    myDrawModel    ( Scene* );
void    myInfoModel    ( Scene* );
static void PrintIntro( void );

/*---------------------------------------------------------------------------*
   Rendering parameters
 *---------------------------------------------------------------------------*/
Camera   myCamera =
{
    { 0.0f,-300.0f, 0.0f },     // position
    { 0.0f,   0.0f, 0.0f },     // target
    { 0.0f,   0.0f, 1.0f },     // upVec
       33.3f,                   // fovy
       16.0f,                   // near plane Z in camera coordinates
     1024.0f,                   // far  plane Z in camera coordinates
};

ViewPort        myViewPort = { 0, 0, 0, 0 };
GXColor         myBgColor  = {  32,  32, 255, 255};
Scene           myScene    = { &myCamera, &myViewPort, &myBgColor };

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void    main ( void )
{
    GXRenderModeObj *rmp;

    // initialize render settings and set clear color for first frame
    DEMOInit(NULL);
    GXInvalidateTexAll( );
    GXSetCopyClear( *(myScene.bgcolor), GX_MAX_Z24 );
    
    // Dummy copy operation to clear the eFB by specified color
    GXCopyDisp( DEMOGetCurrentBuffer(), GX_TRUE );

    rmp = DEMOGetRenderModeObj();
    myViewPort.width  = (s16) rmp->fbWidth;
    myViewPort.height = (s16) rmp->efbHeight;

    myInitModel( );
    
    while ( ! ( DEMOPadGetButton(0) & PAD_BUTTON_MENU ) )
    {
        // get pad status
        DEMOPadRead( );

        // General control & model animation
        SceneControl( &myScene, &DemoPad[0] );
        myAnimeModel( &myScene, &DemoPad[0] );

        // Draw scene
        DEMOBeforeRender( );
        SceneDraw( &myScene );
        DEMODoneRender( );
    }
    
    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

//============================================================================
//  Scene
//============================================================================

/*---------------------------------------------------------------------------*
    Name:           SceneControl
    Description:    user interface for paramter control
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
void    SceneControl( Scene* s, DEMOPadStatus* pad )
{
    #pragma  unused( s, pad )
    return;
}

/*---------------------------------------------------------------------------*
    Name:           SceneDraw
    Description:    Draw model
    Arguments:      Scene* s
    Returns:        none
 *---------------------------------------------------------------------------*/
void    SceneDraw( Scene* s )
{
    Camera*     c  = s->camera;
    ViewPort*   v  = s->viewport;
    float       aspect = (float) (4.0 / 3.0);

    // Set projection matrix
    MTXPerspective ( c->projMtx, c->fovy, aspect, c->znear, c->zfar );
    GXSetProjection( c->projMtx, GX_PERSPECTIVE );

    // Set CameraView matrix
    MTXLookAt( c->viewMtx, &c->position, &c->up, &c->target );

    // Set Viewport
    GXSetViewport( v->xorg, v->yorg, v->width, v->height,
                   SCREEN_ZNEAR, SCREEN_ZFAR );
    GXSetScissor ( v->xorg, v->yorg, v->width, v->height );

    // Set rendering mode
    GXSetCullMode ( GX_CULL_BACK );

    // Draw objects
    myDrawModel( s );

    // draw information
    myInfoModel( s );
    return;
}

//============================================================================
//  Model
//  Rectangles
//============================================================================
typedef struct
{
    GXColor        color;
    Point3d        pos;         // in world space
    f32            dist;
    GXLightObj     obj;
}   MyLight;

typedef struct
{
    Mtx            modelMtx;
    u8             bumpScale;   // 0-128 on Emulator, 0-255 on HW
    s32            invNrmScale;
    s32            blendSteps;
    u32            texNo;
    GXTexObj       texObj;
}   MyModel;

typedef struct
{
    s32            cursor;
    s32            animation;
    GXTexCoordID   coord, coord1; // to show all tex coord pairs work
}   MyControl;

MyLight         myLight;
MyModel         myModel;
MyControl       myControl;
TEXPalettePtr   texPalette = 0;

#ifndef M_SQRT3
#define M_SQRT3     1.732050808f
#endif

/*---------------------------------------------------------------------------*
    Name:           myDrawModel
    Description:    draw model
    Arguments:      Camera *c
    Returns:        none
 *---------------------------------------------------------------------------*/
void    myInitModel( void )
{
    Vec   axis = { 1.0f, 0.0f, 1.0f };
    
    //------------------------------------------------------------------------
    //  MODEL
    //------------------------------------------------------------------------

    //  Matrix
    MTXRotAxisDeg( myModel.modelMtx, &axis, -150.0f );
    
    //  Texture
    myModel.texNo = 1;
    TEXGetPalette( &texPalette, "gxTests/tg-02.tpl" );
    TEXGetGXTexObjFromPalette( texPalette, &myModel.texObj, myModel.texNo-1 );

    //  Bump Mapping
    myModel.bumpScale   = 128;
    myModel.blendSteps  = 3;
    myModel.invNrmScale = 48;

    //------------------------------------------------------------------------
    //  LIGHT 
    //------------------------------------------------------------------------

    //  Light color
    myLight.color.r = 32;
    myLight.color.g = 64;
    myLight.color.b = 96;
    
    //  Light position in world space
    myLight.dist  = 100.0f;
    myLight.pos.x = - myLight.dist * 1.0f / M_SQRT3;
    myLight.pos.y = - myLight.dist * 1.0f / M_SQRT3;
    myLight.pos.z = + myLight.dist * 1.0f / M_SQRT3;
    
    //  Set white light with no attenuation
    GXInitLightSpot    ( &myLight.obj, 0.0f, GX_SP_OFF );
    GXInitLightDistAttn( &myLight.obj, 0.0f, 0.0f, GX_DA_OFF );

    //------------------------------------------------------------------------
    //  CONTROL
    //------------------------------------------------------------------------
    myControl.cursor = 0;
    myControl.animation = 1;
    myControl.coord = GX_TEXCOORD0;
    myControl.coord1 = GX_TEXCOORD1;
}

/*---------------------------------------------------------------------------*
    Name:           myAnimeModel
    Description:    animate model
    Arguments:      Scene *s
    Returns:        none
 *---------------------------------------------------------------------------*/
void    myAnimeModel( Scene* s, DEMOPadStatus* pad )
{
#   pragma  unused(s)
    f32  r2, y2, scale;
    Mtx  tmp;
    Vec  axis;
    
    //---------------------------------------------------------------------
    //  Move cursor
    //---------------------------------------------------------------------
    if ( pad->buttonDown & PAD_BUTTON_X )
    {
        myControl.cursor ++;
    }
    if ( pad->buttonDown & PAD_BUTTON_B )
    {
        myControl.cursor += 4;
    }
    myControl.cursor %= 5;
    
    //---------------------------------------------------------------------
    //  Cycle though all texture coordinates
    //---------------------------------------------------------------------
    if ( pad->buttonDown & PAD_BUTTON_Y )
    {
        myControl.coord++;
        if (myControl.coord > GX_TEXCOORD6)
            myControl.coord = GX_TEXCOORD0;
        myControl.coord1 = (GXTexCoordID)(myControl.coord + 1);
        OSReport("Using texture coord %d and %d\n", 
                myControl.coord, myControl.coord1);
    }
    //---------------------------------------------------------------------
    //  Change item
    //---------------------------------------------------------------------
    if ( pad->buttonDown & PAD_BUTTON_A )
    {
        switch ( myControl.cursor )
        {
        case  0:
            //  Texture
            myModel.texNo %= 3;
            myModel.texNo ++;
            TEXGetGXTexObjFromPalette( texPalette,
                                  &myModel.texObj, myModel.texNo-1 );
            break;
            
        case  1:
            //  Animation
            myControl.animation ^= 1;
            break;
            
        case  2:
            //  Bump scale
            myModel.bumpScale = (u8)( ( myModel.bumpScale < 128 )
                                      ? myModel.bumpScale + 32 : 0 );
            break;

        case  3:
            //  Normal scale    ( 8 - 96 )
            myModel.invNrmScale %= 96;
            myModel.invNrmScale += 8;
            break;
            
        case  4:
            //  Show step of blending
            myModel.blendSteps %= 3;
            myModel.blendSteps ++;
            break;
        }
    }

    //---------------------------------------------------------------------
    //  Animation
    //---------------------------------------------------------------------
    if ( myControl.animation )
    {
        axis.x = 1.0f;
        axis.y = 2.0f;
        axis.z = 0.5f;
        MTXRotAxisDeg( tmp, &axis, 1.0f );
        MTXConcat( myModel.modelMtx, tmp, myModel.modelMtx );
    }

    //---------------------------------------------------------------------
    //  Change light position
    //---------------------------------------------------------------------
    if ( pad->dirs & DEMO_STICK_LEFT  ) myLight.pos.x -= 2.0f;
    if ( pad->dirs & DEMO_STICK_RIGHT ) myLight.pos.x += 2.0f;
    if ( pad->dirs & DEMO_STICK_UP    ) myLight.pos.z += 2.0f;
    if ( pad->dirs & DEMO_STICK_DOWN  ) myLight.pos.z -= 2.0f;
    
    r2 = myLight.pos.x * myLight.pos.x + myLight.pos.z * myLight.pos.z;
    y2 = myLight.dist * myLight.dist - r2;
    
    if ( y2 < 0 )
    {
        scale = myLight.dist / sqrtf(r2);
        myLight.pos.x *= scale;
        myLight.pos.z *= scale;
        myLight.pos.y  = 0.0f;
    }
    else
    {
        myLight.pos.y = - sqrtf(y2);
    }
}

/*---------------------------------------------------------------------------*
    Name:           myDrawModel
    Description:    draw model
    Arguments:      Scene *c
    Returns:        none
 *---------------------------------------------------------------------------*/

#ifdef EMU
static void myDrawSetting_DiffuseLight( u8 );
static void myDrawSetting_BaseBumpTexture( u8 );
static void myDrawSetting_ShiftBumpTexture( );
#endif

void    myDrawModel( Scene* s )
{
    Camera*     c = s->camera;
    Mtx         mvmtx, posMtx, nrmMtx;
    Point3d     lpos;
    f32         nrmScale = 1.0f / (f32)myModel.invNrmScale;
#ifdef EMU
    s32         i;
#endif
	GXColor color1 = {255,64,64,255};

    // set a light object
    MTXMultVec( c->viewMtx, &myLight.pos, &lpos );
    GXInitLightPos   ( &myLight.obj, lpos.x, lpos.y, lpos.z );
    GXInitLightColor ( &myLight.obj, myLight.color );
    GXLoadLightObjImm( &myLight.obj, GX_LIGHT0 );
    
    // set modelview matrix
    GXSetCurrentMtx( GX_PNMTX0 );
    
    // Load ModelView matrix for pos/nrm into view space
    // Note: mvmtx is uniform transformation here, then (mvmtx)^(-T) = mvmtx
    //       With bump mapping, normal must be scaled to match texel size in
    //       texture space.
    MTXConcat      ( c->viewMtx, myModel.modelMtx, mvmtx );
    MTXScale       ( posMtx, 75.0f, 75.0f, 75.0f );
    MTXConcat      ( mvmtx,  posMtx, posMtx );
    GXLoadPosMtxImm( posMtx, GX_PNMTX0 );
    MTXScale       ( nrmMtx, nrmScale, nrmScale, nrmScale );
    MTXConcat      ( mvmtx,  nrmMtx, nrmMtx );
    GXLoadNrmMtxImm( nrmMtx, GX_PNMTX0 );

    // Set texture
    GXLoadTexObj( &myModel.texObj, GX_TEXMAP0 );
    
    // Render mode for Box
    GXClearVtxDesc( );
    
    //  z update for 1st stage
    GXSetZMode  ( GX_TRUE, GX_LESS, GX_TRUE );
    
#ifdef EMU
    //------------------------------------------------------------------------
    //   This works same render setting as Bump Mapping Tev stage on real HW.
    //------------------------------------------------------------------------
    for ( i = 0; i < myModel.blendSteps; i ++ )
    {
        if ( i > 0 )
        {
            //  if 2nd stage or later, overlap mode
            GXSetZMode  ( GX_TRUE, GX_EQUAL, GX_FALSE );
        }
        
        switch ( i )
        {
        case  0:
            //
            //  Get shifted texture T1
            //
            myDrawSetting_ShiftBumpTexture( );
            
            //
            //  Blend mode ~Src
            //
            //            Result = (1.0 - T1)
            //
            GXSetBlendMode( GX_BM_LOGIC, GX_BL_ONE,
                                         GX_BL_ZERO, GX_LO_INVCOPY );
            
            break;
            
        case  1:
            //
            //  Get diffuse light color D
            //  Set source alpha as bump scale
            //  Diffuse color might be biased by ( 0.5 - scale )
            //
            myDrawSetting_DiffuseLight( myModel.bumpScale );
            
            //
            //  Blend mode SrcColor*1+SrcAlpha*DestColor
            //
            //            Result = ( D + 0.5 - scale ) + scale * ( 1.0 - T1 )
            //                   = D + 0.5 - scale * T1
            //
            GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE,
                                         GX_BL_SRCALPHA, GX_LO_CLEAR);
            break;
            
        case  2:
            //
            //  Get base texture T0 scaled => Get T0 * scale
            //
            myDrawSetting_BaseBumpTexture( myModel.bumpScale );
            
            //
            //  Blend mode SrcColor + DesrColor
            //
            //            Resullt: D + 0.5 + scale * ( T0 - T1 )
            //
            GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
            
            break;
        }
#else
    //------------------------------------------------------------------------
    //   Actual code for bump mapping on dolphin HW.
    //------------------------------------------------------------------------
    {
        GXColor bumpScale;
        u32     i;
        GXColor white = {255,255,255,255};
		GXColor grey = {128,128,128,128};

        //  Vertex component
        GXClearVtxDesc();
        GXSetVtxDesc( GX_VA_POS,  GX_DIRECT );
        GXSetVtxDesc( GX_VA_NBT,  GX_DIRECT );
        GXSetVtxDesc( GX_VA_TEX0, GX_DIRECT );
    
        //  Light 0 for bump mapping
        GXSetNumChans(1); // GX_COLOR0A0
        GXSetChanCtrl( GX_COLOR0, GX_ENABLE,     GX_SRC_REG,  GX_SRC_REG,
                                  GX_LIGHT0,     GX_DF_CLAMP, GX_AF_NONE );
        GXSetChanCtrl( GX_ALPHA0, GX_DISABLE,    GX_SRC_REG,  GX_SRC_REG,
                                  GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE );
        GXSetChanMatColor( GX_COLOR0A0, white );
        GXSetChanAmbColor( GX_COLOR0A0, grey );
   
        // clear out unused tex gens
        for (i = 0; i < myControl.coord; i++)
            GXSetTexCoordGen( (GXTexCoordID)i, GX_TG_MTX2x4, 
                              GX_TG_POS, GX_IDENTITY );

        //  Tex Gen
        //     GX_TEXCOORD0: Base texcoord of bump mapping 
        //     GX_TEXCOORD1: Tex gen by bump mapping
        GXSetNumTexGens((u8)(myControl.coord1+1)); // coord1 is always the max coord
        GXSetTexCoordGen( myControl.coord, GX_TG_MTX2x4,
                          GX_TG_TEX0,      GX_IDENTITY );
        GXSetTexCoordGen( myControl.coord1, GX_TG_BUMP0,
                          (GXTexGenSrc)((u32)myControl.coord + GX_TG_TEXCOORD0),
                          GX_IDENTITY );
    
        //  Tev Order
        //     GX_TEVSTAGE0: send base    texture + diffuse color
        //     GX_TEVSTAGE1: send shifted texture
        GXSetTevOrder( GX_TEVSTAGE0, 
                       myControl.coord, GX_TEXMAP0, GX_COLOR0A0   );
        GXSetTevOrder( GX_TEVSTAGE1, 
                       myControl.coord1, GX_TEXMAP0, GX_COLOR_NULL );
        
        //  Tev Register
        //     GX_TEVREG0: ( X, X, X, bumpScale )
        bumpScale.a = myModel.bumpScale;
        GXSetTevColor( GX_TEVREG0, bumpScale );

        //
        //  In many case, bump mapping needs 3 tev stage, but here,
        //  use only 2 tev stages, since no material texture map
        //  
        GXSetNumTevStages( 2 );
        
        //
        //  Tev stage0
        //     Color:   BumpTexture * bumpScale + DiffuseColor => R
        //     Alpha:   Not used
        //
        GXSetTevColorIn( GX_TEVSTAGE0, GX_CC_ZERO,
                                       GX_CC_TEXC,  /* Bump Texture   */
                                       GX_CC_A0,    /* BumpScale      */
                                       GX_CC_RASC   /* Diffuse color  */ );
        GXSetTevColorOp( GX_TEVSTAGE0, GX_TEV_ADD,
                                       GX_TB_ZERO,
                                       GX_CS_SCALE_1,
                                       GX_DISABLE,
                                       GX_TEVPREV   /* To next stage  */ );

        GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                                       GX_CA_ZERO, GX_CA_ZERO );
        GXSetTevAlphaOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                                       GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV );

        //
        //  Tev stage1
        //     Color:   ShiftTexture * bumpScale * (-1) + R  =>  R
        //     Alpha:   Not used
        //
        GXSetTevColorIn( GX_TEVSTAGE1, GX_CC_ZERO,
                                       GX_CC_TEXC,  /* Shift Texture  */
                                       GX_CC_A0,    /* BumpScale      */
                                       GX_CC_CPREV  /* Last stage out */ );
        GXSetTevColorOp( GX_TEVSTAGE1, GX_TEV_SUB,  /* mult (-1)      */
                                       GX_TB_ZERO,
                                       GX_CS_SCALE_1,
                                       GX_ENABLE,
                                       GX_TEVPREV                        );

        GXSetTevAlphaIn( GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO,
                                       GX_CA_ZERO, GX_CA_ZERO );
        GXSetTevAlphaOp( GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                                       GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV );
#endif
    
        //-------------------------------------------------
        //  Draw objects
        //-------------------------------------------------
        GXDrawCube( );
    }

    //-------------------------------------------------
    //  Draw light direction
    //-------------------------------------------------
    
    // Shade mode
    GXSetChanCtrl    ( GX_COLOR0, GX_DISABLE,    GX_SRC_REG,  GX_SRC_REG,
                                  GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE );
    GXSetChanMatColor( GX_COLOR0, color1 );
    
    GXSetNumTexGens  ( 0 );

    // Tev mode
    GXSetNumTevStages( 1 );
    GXSetTevOrder    ( GX_TEVSTAGE0,
                       GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    GXSetTevOp       ( GX_TEVSTAGE0, GX_PASSCLR );
    
    // Render mode
    GXSetBlendMode   ( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );
    GXSetZMode       ( GX_TRUE, GX_LESS, GX_TRUE );
    
    // Draw lines
    GXClearVtxDesc   ( );
    GXSetVtxDesc     ( GX_VA_POS, GX_DIRECT );
    GXSetVtxAttrFmt  ( GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
    GXLoadPosMtxImm  ( c->viewMtx, GX_PNMTX0 );
    GXBegin( GX_LINES, GX_VTXFMT1, 2 );
       GXPosition3f32( 0.0f, 0.0f, 0.0f );
       GXPosition3f32( myLight.pos.x, myLight.pos.y, myLight.pos.z );
    GXEnd( );
    GXBegin( GX_LINESTRIP, GX_VTXFMT1, 5 );
       GXPosition3f32( myLight.pos.x+2.0f, myLight.pos.y, myLight.pos.z+2.0f );
       GXPosition3f32( myLight.pos.x+2.0f, myLight.pos.y, myLight.pos.z-2.0f );
       GXPosition3f32( myLight.pos.x-2.0f, myLight.pos.y, myLight.pos.z-2.0f );
       GXPosition3f32( myLight.pos.x-2.0f, myLight.pos.y, myLight.pos.z+2.0f );
       GXPosition3f32( myLight.pos.x+2.0f, myLight.pos.y, myLight.pos.z+2.0f );
    GXEnd( );
    
    return;
}

#ifdef EMU
/*---------------------------------------------------------------------------*
    Name:           myDrawSetting_DiffuseLight
    Description:    set draw mode for diffuse light color
    Arguments:      u8 alpha: alpha value to output
    Returns:        none
 *---------------------------------------------------------------------------*/
static void  myDrawSetting_DiffuseLight( u8 a )
{
    u8   bias = (u8)( 128 - a );
	GXColor color0, color1;

	color0.r = color0.g = color0.b = 255;
	color0.a = a;
	color1.r = color1.g = color1.b = bias;
	color1.a = 0;
    
    // Vertex component
    GXSetVtxDesc( GX_VA_NBT,  GX_NONE );
    GXSetVtxDesc( GX_VA_TEX0, GX_NONE );
    
    //  light dir will be computed for bump mapping
    GXSetChanCtrl( GX_COLOR0, GX_ENABLE,     GX_SRC_REG,  GX_SRC_REG,
                              GX_LIGHT0,     GX_DF_CLAMP, GX_AF_NONE );
    GXSetChanCtrl( GX_ALPHA0, GX_DISABLE,    GX_SRC_REG,  GX_SRC_REG,
                              GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE );
    GXSetChanMatColor( GX_COLOR0A0, color0 );
    GXSetChanAmbColor( GX_COLOR0A0, color1 );
   
    // Color Gen
    GXSetNumChans(1); // GX_COLOR0A0

    // Tex Gen
    GXSetNumTexGens( 0 );
    
    //  use 1 tev stage
    GXSetNumTevStages( 1 );
    
    // Tev Op
    //    GX_TEVSTAGE0: GX_PASSCLR
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
    
    // Tev Order
    //    GX_TEVSTAGE0: send color value
    GXSetTevOrder( GX_TEVSTAGE0,
                   GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    return;
}

/*---------------------------------------------------------------------------*
    Name:           myDrawSetting_BaseBumpTexture
    Description:    set draw mode to base bump texture
    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void  myDrawSetting_BaseBumpTexture( u8 scale )
{
	GXColor color;

	color.r = color.g = color.b = color.a = scale;

    //  Vertex component
    GXSetVtxDesc( GX_VA_NBT,  GX_NONE );
    GXSetVtxDesc( GX_VA_TEX0, GX_DIRECT );
    
    //  light dir will be computed for bump mapping
    GXSetChanCtrl( GX_COLOR0, GX_DISABLE,    GX_SRC_REG,  GX_SRC_REG,
                              GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE );
    GXSetChanMatColor( GX_COLOR0A0, color );
    
    // Tex Gen
    //    GX_TEXCOORD0: Base texcoord of bump mapping 
    GXSetNumTexGens( 1 );
    GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
    
    //  use 1 tev stage
    GXSetNumTevStages( 1 );
    
    // Tev Op
    //    GX_TEVSTAGE0: GX_MODULATE  T2 * scale
    GXSetTevOp( GX_TEVSTAGE0, GX_MODULATE );
    
    // Tev Order
    //    GX_TEVSTAGE0: send base texture
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
    return;
}

/*---------------------------------------------------------------------------*
    Name:           myDrawSetting_ShiftBumpTexture
    Description:    set draw mode to shift texture
    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void  myDrawSetting_ShiftBumpTexture( void )
{
    // Vertex component
    GXSetVtxDesc( GX_VA_NBT,  GX_DIRECT );
    GXSetVtxDesc( GX_VA_TEX0, GX_DIRECT );
   
    //  light dir will be computed for bump mapping
    GXSetChanCtrl( GX_COLOR0, GX_ENABLE, GX_SRC_REG,  GX_SRC_REG,
                              GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE );
    
    // Tex Gen
    //    GX_TEXCOORD0: Base texcoord of bump mapping 
    //    GX_TEXCOORD1: Tex gen by bump mapping
    GXSetNumTexGens(2);
    GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,     GX_IDENTITY);
    GXSetTexCoordGen( GX_TEXCOORD1, GX_TG_BUMP0, GX_TG_TEXCOORD0, GX_IDENTITY);
            
    //  use 2 tev stages
    GXSetNumTevStages( 1 );
    
    // Tev Op
    //    GX_TEVSTAGE0: GX_REPLACE
    GXSetTevOp( GX_TEVSTAGE0, GX_REPLACE );
    
    // Tev Order
    //    GX_TEVSTAGE0: Fetch shifted texture
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL );
    
    return;
}
#endif

/*---------------------------------------------------------------------------*
    Name:           ModelDrawInfo
    Description:    Draw scene information
    Arguments:
    Returns:        none
 *---------------------------------------------------------------------------*/
void    myInfoModel( Scene* s )
{
    ViewPort*   v = s->viewport;
    
    // Z mode for drawing captions
    GXSetZMode( GX_TRUE, GX_ALWAYS, GX_TRUE );
    
    // Draw paramters to the window
    DEMOInitCaption( DM_FT_XLU, v->width, v->height );
    DEMOPuts  ( 10, (s16)(myControl.cursor*8+10), 0, "\x7f" );
    DEMOPrintf( 18, 10, 0, "Texture      %d", myModel.texNo );
    DEMOPrintf( 18, 18, 0,
                "Animation    %s", myControl.animation ? "ON" : "OFF" );
    DEMOPrintf( 18, 26, 0, "Bump   scale %d", myModel.bumpScale );
    DEMOPrintf( 18, 34, 0, "Normal scale 1/%d", myModel.invNrmScale );
    DEMOPrintf( 18, 42, 0, "Blend  steps %d", myModel.blendSteps );
    
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
    OSReport("**********************************************\n");
    OSReport("tg-emboss: demonstrate bump mapping using the \n");
    OSReport("           embossing technique.               \n");
    OSReport("**********************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick   : Move Light Position\n");
    OSReport("Sub  Stick   : Rotate the Model\n");
    OSReport("X/B Buttons  : Select parameter\n");
    OSReport("A Button     : Change selected parameter\n");
    OSReport("Y Button     : Change texture coordinate\n");
    OSReport("**********************************************\n");
}



/*======== End of tg-emboss.c ========*/
