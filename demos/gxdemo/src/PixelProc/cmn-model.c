/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     cmn-model.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/PixelProc/cmn-model.c $
    
    5     10/25/00 9:59p Hirose
    flag change. MAC -> EMU for PC emulator compatibility
    
    4     7/07/00 3:33p Carl
    Added support for fog range tests.
    
    3     5/17/00 8:06p Hirose
    deleted tab codes /
    set appropriate Z mode before calling DEMOPuts library because
    DEMOInitCaption doesn't set Z mode any more
    
    2     3/23/00 6:31p Hirose
    updated to use DEMOPad library instead of cmn-pad
    
    1     3/06/00 12:11p Alligator
    move from tests/gx and rename
    
    3     2/24/00 11:53p Yasu
    
    2     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    13    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    12    11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    11    11/01/99 4:12p Alligator
    changed GXSetTevStages to GXSetNumTevStages
    
    10    10/22/99 7:42p Yasu
    Add GXSetTevStages and GXSetTevOrder
    
    9     9/30/99 10:08p Yasu
    Renamed functions and enums
    
    8     9/23/99 8:52p Yasu
    Reshape { } 
    
    7     9/21/99 5:03p Yasu
    Fixed model #1 with no flat shading
    
    6     9/20/99 10:57a Alligator
    remove GXSetShadeMode, update Tlut api
    
    5     9/14/99 6:02p Yasu
    Add model#4
    Change cmPuts function set to DEMOPuts function set.
    
    4     9/10/99 6:41a Yasu
    Insert some type cast to avoid warning messages
    
    3     9/08/99 7:25p Yasu
    Add  GXSetZMode/Update in the code of model #0 to #2
    
    2     9/08/99 6:30p Yasu
    Add model #3
    
    1     9/07/99 9:48p Yasu
    Initial version
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <stdlib.h>     // for rand()
#include <math.h>       // for cosf(), sinf()
#include "cmn-model.h"

#ifdef  EMU
// Not implemented on emulator
#  define GXPosition2s16(x,y)  GXPosition3s16((x),(y),(s16)0)
#  define GX_POS_XY            GX_POS_XYZ
#  define GXSetTevStages(x)
#endif

#define PAI     3.1415926535898f

static  void    cmModel0_Draw( Camera* );
static  void    cmModel1_Anim( Model*, DEMOPadStatus*, Camera* );
static  void    cmModel1_Draw( Camera* );
static  void    cmModel2_Anim( Model*, DEMOPadStatus*, Camera* );
static  void    cmModel2_Draw( Camera* );
static  void    cmModel3_Anim( Model*, DEMOPadStatus*, Camera* );
static  void    cmModel3_Draw( Camera* );
static  void    cmModel4_Anim( Model*, DEMOPadStatus*, Camera* );
static  void    cmModel4_Draw( Camera* );

Model   cmModel[] = {
    { cmModel0_Draw, 0,             0 },
    { cmModel1_Draw, cmModel1_Anim, 0 },
    { cmModel2_Draw, cmModel2_Anim, 0 },
    { cmModel3_Draw, cmModel3_Anim, 0 },
    { cmModel4_Draw, cmModel4_Anim, 0 },
};

//============================================================================
//  Model
//============================================================================

/*---------------------------------------------------------------------------*
    Name:           ModelAnim
    Description:    Animate all models
    Arguments:      Scene* s
    Returns:        none
 *---------------------------------------------------------------------------*/
void    cmModelAnime( DEMOPadStatus* pad, Camera* c )
{
    int  i;
    
    // walk through all models
    for ( i = 0; i < sizeof(cmModel)/sizeof(Model); i ++ )
    {
        // if valid model, do animation
        if ( cmModel[i].anim )
        {
            (*cmModel[i].anim)( &cmModel[i], pad, c );
        }
    }
    return;
}

//============================================================================
//  Model 0
//  Static grid stage
//============================================================================

/*---------------------------------------------------------------------------*
    Name:           cmModel0_Draw
    Description:    Draw mesh grid
    Arguments:      Camera*
    Returns:        none
 *---------------------------------------------------------------------------*/
static  void    cmModel0_Draw( Camera* c )
{
    s32     i;
    GXColor red = {255, 0, 0, 255};
    
    // Set modelview matrix
    GXSetCurrentMtx( GX_PNMTX0 );
    GXLoadPosMtxImm( c->viewMtx, GX_PNMTX0 );
    
    // Use constant material color
    GXSetChanCtrl( GX_COLOR0,
                   GX_FALSE,            // enable Channel
                   GX_SRC_REG,          // amb source           (Don't care)
                   GX_SRC_REG,          // mat source
                   GX_LIGHT0,           // light mask           (Don't care)
                   GX_DF_NONE,          // diffuse function     (Don't care)
                   GX_AF_NONE );        // atten   function     (Don't care)
    GXSetChanCtrl( GX_ALPHA0,
                   GX_FALSE,            // enable Channel
                   GX_SRC_REG,          // amb source           (Don't care)
                   GX_SRC_REG,          // mat source
                   GX_LIGHT0,           // light mask           (Don't care)
                   GX_DF_NONE,          // diffuse function     (Don't care)
                   GX_AF_NONE );        // atten   function     (Don't care)
    GXSetChanMatColor( GX_COLOR0A0, red );
    GXSetNumChans(1);
    
    // Set TEV parameters
    GXSetNumTexGens( 0 );               // # of Tex Gens
    GXSetNumTevStages( 1 );             // # of Tev Stages
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,GX_COLOR0A0);
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
    
    // Set vertex descripter
    GXClearVtxDesc( );
    GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0 );
    
#define GRIDX             11
#define GRIDY             11
#define STEP             100
#define ORGX            -500
#define ORGY               0
    
    // Send vertex data for grid
    GXBegin( GX_LINES, GX_VTXFMT0, (GRIDX+GRIDY)*2 );
    
        // Vertical lines
        for ( i = 0; i < GRIDX; i ++ )
        {
            GXPosition2s16( (s16)(i*STEP+ORGX), (s16)(               ORGY) );
            GXPosition2s16( (s16)(i*STEP+ORGX), (s16)((GRIDY-1)*STEP+ORGY) );
        }
        
        // Horizonal lines
        for ( i = 0; i < GRIDY; i ++ )
        {
            GXPosition2s16( (s16)(               ORGX), (s16)(i*STEP+ORGY) );
            GXPosition2s16( (s16)((GRIDX-1)*STEP+ORGX), (s16)(i*STEP+ORGY) );
        }
        
    GXEnd( );
    
    return;
}

//============================================================================
//  Model 1
//  Flying depth tags
//============================================================================

// Data area
typedef struct {                // for z coord tag data
  f32     x, y, z, dir;
  GXColor color;
} Obj1;

Obj1 obj1_data[10] = {
  {  6.0f,   4.0f,  100.0f, -1.0f, {255,255,255,255} },
  {-20.0f,  10.0f,  200.0f,  0.5f, {255,255,  0,255} },
  {-20.0f,   0.0f,  300.0f, -0.6f, {255,  0,255,255} },
  {-20.0f, -30.0f,  400.0f,  1.0f, {  0,255,255,255} },
  { 10.0f, -30.0f,  500.0f,  0.9f, {255,  0,  0,255} },
  { 40.0f, -30.0f,  600.0f, -0.2f, {  0,  0,255,255} },
  { 40.0f,   0.0f,  700.0f,  0.4f, {  0,255,  0,255} },
  { 40.0f,  50.0f,  800.0f, -0.6f, {255,255,128,255} },
  {  0.0f,  54.0f,  900.0f, -0.7f, {255,128,255,255} },
  {-60.0f,  60.0f, 1000.0f,  0.3f, {128,255,255,255} },
};

Obj1 *obj1 = obj1_data;

/*---------------------------------------------------------------------------*
    Name:           cmModel1_Anim
    Description:    Move z coord tags
    Arguments:      Model*, DEMOPadStatus*, Camera*
    Returns:        none
 *---------------------------------------------------------------------------*/
static  void    cmModel1_Anim( Model* m, DEMOPadStatus* pad, Camera* c )
{
    s32         i;
    Obj1*       p;
    static  u32 anim = 1;
    
    // do animation only if referred
    if ( !(m->flag & MODEL_REFERRED) ) return;
    
    // control animation flag
    if ( pad->buttonDown & PAD_TRIGGER_R ) anim ^= 1;
    if ( anim == 0 ) return;
    
    p = obj1;
    for ( i = 0; i < 10; i ++ )
    {
        // Move object
        if ( (int)(p->y) & 1 )
        {
            f32 ang, dx, dz, dist;
            ang = (i * 44.4f/10.0f - 44.4f*9.0f/20.0f) * 3.1415926535f/180.0f;
            dx =  sinf(ang);
            dz =  cosf(ang);
            p->x += dx * p->dir;
            p->z += dz * p->dir;
            dist = (p->x * p->x + p->z * p->z);
            if (dist > c->zfar*c->zfar || dist < c->zfar*c->zfar/9)
            {
                p->dir = -p->dir;
            }
        }
        else if ( (int)(p->x) & 1 )
        {
            p->z += p->dir;
        
            if ( p->z > c->zfar || p->z < c->zfar / 3 )
            {
                p->dir = -p->dir;
            }
        }
        else
        {
            p->z += p->dir;
        
            // Check if near/far plane
            if ( p->z > c->zfar || p->z < c->znear )
            {
                // Set new x,y coordinates
                p->x += 16.0f;
                p->y -= 12.0f;
                if ( p->x >  48.0f ) p->x -= 96.0f;
                if ( p->y < -48.0f ) p->y += 96.0f;

                // Reset z coordinate
                p->z = ( p->z > c->zfar ) ? c->znear : c->zfar;
            }
        }
        
        p ++;
    }
    return;
}

/*---------------------------------------------------------------------------*
    Name:           cmModel1_Draw
    Description:    Draw the tags of z coordinate
    Arguments:      Camera*
    Returns:        none
 *---------------------------------------------------------------------------*/
static  void    cmModel1_Draw( Camera* c )
{
    s32         i, j;
    Mtx         mmtx;
    Mtx         vmtx;
    Mtx         fmtx;
    
    // Draw grid
    cmModel0_Draw( c );
    
    // Compute model matrix
    for ( i = 0; i < 3; i ++ )
    {
        for ( j = 0; j < 4; j ++ )
        {
            mmtx[i][j] = 0.0f;
        }
    }
    mmtx[0][0] =  1.0f;
    mmtx[1][2] =  1.0f;
    mmtx[2][1] = -1.0f;
    
    // Set modelview matrix
    MTXConcat( c->viewMtx, mmtx, vmtx );
    GXSetCurrentMtx( GX_PNMTX0 );
    
    // use constant material for color
    GXSetChanCtrl( GX_COLOR0,
                   GX_FALSE,            // enable Channel
                   GX_SRC_REG,          // amb source           (Don't care)
                   GX_SRC_REG,          // mat source
                   GX_LIGHT0,           // light mask           (Don't care)
                   GX_DF_NONE,          // diffuse function     (Don't care)
                   GX_AF_NONE );        // atten   function     (Don't care)
    GXSetChanCtrl( GX_ALPHA0,
                   GX_FALSE,            // enable Channel
                   GX_SRC_REG,          // amb source           (Don't care)
                   GX_SRC_REG,          // mat source
                   GX_LIGHT0,           // light mask           (Don't care)
                   GX_DF_NONE,          // diffuse function     (Don't care)
                   GX_AF_NONE );        // atten   function     (Don't care)
    GXSetNumChans(1);
    
    // set several mode
    GXSetCullMode( GX_CULL_BACK );              // Cull mode
    GXSetNumTevStages( 1 );             // # of Tev Stage
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_MODULATE );
    
    // Draw tags
    DEMOLoadFont( GX_TEXMAP0, GX_TEXMTX0, DMTF_BILERP );
    for ( i = 0; i < 10; i ++ )
    {
        // okay, this is very inefficient, but it works!
        MTXTrans( mmtx, obj1[i].x-12.0f, obj1[i].y-4.0f, obj1[i].z);
        MTXConcat( vmtx, mmtx, fmtx );
        MTXScale( mmtx, 2.0f, 2.0f, 1.0f );
        MTXConcat( fmtx, mmtx, fmtx );
        GXLoadPosMtxImm( fmtx, GX_PNMTX0 );

        GXSetChanMatColor( GX_COLOR0A0, obj1[i].color );
        DEMOPrintf( 0, 0, 0, "%3d", (s32)obj1[i].z );
    }
    
    return;
}

//============================================================================
//  Model 2
//  Fly on the hill
//============================================================================

// Data area
static  s16     obj2_height[11][12] = { 0 };
static  s32     obj2_ybase = 0;
static  s32     obj2_yofst = 0;
#define MESHSTEP        100

/*---------------------------------------------------------------------------*
    Name:           cmModel2_Anim
    Description:    Generate new map and move camera.
    Arguments:      Model*, DEMOPadStatus*, Camera*
    Returns:        none
 *---------------------------------------------------------------------------*/
static  void    cmModel2_Anim( Model* m, DEMOPadStatus* pad, Camera* c )
{
    s32         i, y, h;
    f32         z, z0, z1, t;
    static  u32 anim = 1;
    
    // do animation only when referred
    if ( !(m->flag & MODEL_REFERRED) ) return;
    
    // control animation flag
    if ( pad->buttonDown & PAD_TRIGGER_R ) anim ^= 1;
    if ( anim == 0 ) return;
    
    obj2_yofst ++;
    if ( obj2_yofst >= MESHSTEP )
    {
        // update mesh sub-strip
        y = ( obj2_ybase + 11 ) % 12;
        for ( i = 0; i < 11; i ++ )
        {
            h = obj2_height[i][y] + ((signed)(rand() & 63)) - 32;
            if ( h > 100 ) h -= 48;
            if ( h <   0 ) h = -h;
            obj2_height[i][obj2_ybase] = (s16)h;
        }
        
        // update y coordinate
        obj2_yofst -= MESHSTEP;
        obj2_ybase  = ( obj2_ybase + 1 ) % 12;
    }
    
    // camera does trail on middle of 4th and 5th column of the mesh
    // compute Z coordinate at that point
    y = ( obj2_ybase + 1 ) % 12;
    t = (float)obj2_yofst / (float)MESHSTEP;
    z0 = (float)( obj2_height[4][obj2_ybase] + obj2_height[5][obj2_ybase] );
    z1 = (float)( obj2_height[4][y         ] + obj2_height[5][y         ] );
    z  = ( z0 * (1.0f - t) + z1 * t ) / 2.0f + 30.0f;
    
    // change height of camera
    c->position.z = ( z > 30.0f ) ? z : 30.0f;
    
    return;
}

/*---------------------------------------------------------------------------*
    Name:           cmModel2_Draw
    Description:    Draw ground
    Arguments:      Camera* c
    Returns:        none
 *---------------------------------------------------------------------------*/
// Intensity of diffuse/ambient light
#define ITN_DIF         0.60f
#define ITN_AMB         0.40f

static  void    cmModel2_Draw( Camera* c )
{
    s32         i, j, y0, y1, sx, sy;
    Vec         n0, n1;
    f32         itn0, itn1;
    u8          r0, g0, b0, r1, g1, b1;
    static Vec  lightDir = { 0.577f, -0.577f, 0.577f };
    GXColor     black = { 0, 0, 0, 255 };
    
    // set modelview matrix
    GXSetCurrentMtx( GX_PNMTX0 );
    GXLoadPosMtxImm( c->viewMtx, GX_PNMTX0 );
    
    // use constant material color
    GXSetChanCtrl( GX_COLOR0,
                   GX_FALSE,            // enable Channel
                   GX_SRC_REG,          // amb source
                   GX_SRC_VTX,          // mat source
                   GX_LIGHT0,           // light mask
                   GX_DF_NONE,          // diffuse function
                   GX_AF_NONE );
    GXSetChanCtrl( GX_ALPHA0,
                   GX_FALSE,            // enable Channel
                   GX_SRC_REG,          // amb source           (Don't care)
                   GX_SRC_REG,          // mat source
                   GX_LIGHT0,           // light mask           (Don't care)
                   GX_DF_NONE,          // diffuse function     (Don't care)
                   GX_AF_NONE );        // atten   function     (Don't care)
    GXSetChanMatColor( GX_ALPHA0, black );
    GXSetNumChans(1);
    
    // set several mode
    GXSetCullMode( GX_CULL_BACK );      // Cull mode
    GXSetNumTexGens( 0 );               // # of Tex Gens
    GXSetNumTevStages( 1 );             // # of Tev Stage
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL,GX_TEXMAP_NULL,GX_COLOR0A0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );     // Stage 0
    
    // set vertex descripter
    GXClearVtxDesc();
    GXSetVtxDesc( GX_VA_POS,  GX_DIRECT );
    GXSetVtxDesc( GX_VA_CLR0, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ, GX_S16,  0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0 );
    
    // draw the ground
    y0 = obj2_ybase;
    for ( j = 0; j < 11; j ++ )
    {
        y1 = ( y0 + 1 ) % 12;
        GXBegin( GX_TRIANGLES, GX_VTXFMT0, 3*2*10 );
        for ( i = 0; i < 10; i ++ )
        {
            // get vertex
            sx = (i-5)*MESHSTEP+MESHSTEP/2;
            sy = (j  )*MESHSTEP-obj2_yofst;
            
            // compute intensity of light, then get color
            n0.x = (float)( obj2_height[i  ][y0] - obj2_height[i-1][y0] );
            n0.y = (float)( obj2_height[i-1][y1] - obj2_height[i-1][y0] );
            n0.z = (float)MESHSTEP;
            VECNormalize( &n0, &n0 );
            itn0 = VECDotProduct( &n0, &lightDir );
            itn0 = ( itn0 < 0.0f ? 0.0f : itn0 ) * ITN_DIF + ITN_AMB;
            r0 = (u8)(itn0 * 64.0f );
            g0 = (u8)(itn0 * 255.0f);
            b0 = (u8)(itn0 * 128.0f);
            
            // send vertex of triangle 0
            GXPosition3s16( (s16)(sx), (s16)(sy),
                            (s16)obj2_height[i][y0] );
            GXColor3u8( r0, g0, b0 );
            GXPosition3s16( (s16)(sx), (s16)(sy+MESHSTEP),
                            (s16)obj2_height[i][y1] );
            GXColor3u8( r0, g0, b0 );
            GXPosition3s16( (s16)(sx+MESHSTEP), (s16)(sy),
                            (s16)obj2_height[i+1][y0] );
            GXColor3u8( r0, g0, b0 );
            
            // compute intensity of light, then get color
            n1.x = (float)( obj2_height[i-1][y1] - obj2_height[i][y1] );
            n1.y = (float)( obj2_height[i  ][y0] - obj2_height[i][y1] );
            n1.z = (float)MESHSTEP;
            VECNormalize( &n1, &n1 );
            itn1 = VECDotProduct( &n1, &lightDir );
            itn1 = ( itn1 < 0.0f ? 0.0f : itn1 ) * ITN_DIF + ITN_AMB;
            r1 = (u8)(itn1 * 255.0f);
            g1 = (u8)(itn1 *  92.0f);
            b1 = (u8)(itn1 *  32.0f);
            
            // send vertex of triangle 1
            GXPosition3s16( (s16)(sx), (s16)(sy+MESHSTEP),
                            (s16)obj2_height[i][y1] );
            GXColor3u8( r1, g1, b1 );
            GXPosition3s16( (s16)(sx+MESHSTEP), (s16)(sy+MESHSTEP),
                            (s16)obj2_height[i+1][y1] );
            GXColor3u8( r1, g1, b1 );
            GXPosition3s16( (s16)(sx+MESHSTEP), (s16)(sy),
                            (s16)obj2_height[i+1][y0] );
            GXColor3u8( r1, g1, b1 );
        }
        GXEnd( );
        y0 = y1;
    }
    return;
}

//============================================================================
//  Model 3
//  Rotating polygons
//============================================================================

// Data area
typedef struct {
  f32       x, y, z;
  f32       w, h;
  f32       ang, avel;  // rotation anime parameter
  f32       r, dr;      // deform   anime parameter
  GXColor   color;
} Obj3;

Obj3    obj3[] = {
//  x    y    z   w    h  ang  avel  r     dr   color
{  50, 500,  30, 10,  80,   0,  1,   0,  0.1f, {255,  0,  0,255} },
{-100, 502,  20, 30,  60,  30,  1,  90, -1.5f, {  0,255,  0,255} },
{  50, 504, -50, 20,  80, 120,  1,  72,  3.0f, {  0,  0,255,255} },
{  10, 501,  75, 25, 120,  90,  2,   0,  1.0f, {255,128,128, 64} },
{ -20, 505, -15, 30,  30,  20,  1, 180,  0.2f, {128,255,128,128} },
{  40, 503,   0,  5,  80, 150,  3,   0, -0.1f, {128,128,255,192} },
};

/*---------------------------------------------------------------------------*
    Name:           cmModel3_Anim
    Description:    Rotate quadrangles
    Arguments:      Model*, DEMOPadStatus*, Camera*
    Returns:        none
 *---------------------------------------------------------------------------*/
static  void    cmModel3_Anim( Model* m, DEMOPadStatus* pad, Camera* c )
{
    #pragma     unused(c)
    s32         i;
    static  u32 anim = 1;
    
    // do animation only if referred
    if ( !(m->flag & MODEL_REFERRED) ) return;
    
    // control animation flag
    if ( pad->buttonDown & PAD_TRIGGER_R ) anim ^= 1;
    if ( anim == 0 ) return;
    
    for ( i = 0; i < sizeof(obj3)/sizeof(Obj3); i ++ )
    {
        // Animation of the object
        obj3[i].r += obj3[i].dr;
        
        // Rotation 
        obj3[i].ang += obj3[i].avel * cosf( obj3[i].r * PAI / 180.0f );
    }
    return;
}

/*---------------------------------------------------------------------------*
    Name:           cmModel3_Draw
    Description:    Draw the rotating polygons
    Arguments:      Camera* c
    Returns:        none
 *---------------------------------------------------------------------------*/
static  void    cmModel3_Draw( Camera* c )
{
    s32   i;
    f32   cs, sn, wcos, wsin, hcos, hsin;
    
    // Draw grid
    cmModel0_Draw( c );
    
    // Set modelview matrix
    GXSetCurrentMtx( GX_PNMTX0 );
    GXLoadPosMtxImm( c->viewMtx, GX_PNMTX0 );
    
    // use constant material color
    GXSetChanCtrl( GX_COLOR0,
                   GX_FALSE,            // enable Channel
                   GX_SRC_REG,          // amb source           (Don't care)
                   GX_SRC_REG,          // mat source
                   GX_LIGHT0,           // light mask           (Don't care)
                   GX_DF_NONE,          // diffuse function     (Don't care)
                   GX_AF_NONE );        // atten   function     (Don't care)
    GXSetChanCtrl( GX_ALPHA0,
                   GX_FALSE,            // enable Channel
                   GX_SRC_REG,          // amb source           (Don't care)
                   GX_SRC_REG,          // mat source
                   GX_LIGHT0,           // light mask           (Don't care)
                   GX_DF_NONE,          // diffuse function     (Don't care)
                   GX_AF_NONE );        // atten   function     (Don't care)
    GXSetNumChans(1);
    
    // set several mode
    GXSetCullMode( GX_CULL_BACK );      // Cull mode
    GXSetNumTexGens( 0 );               // # of Tex Gens
    GXSetNumTevStages( 1 );             // # of Tev Stage
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL,GX_TEXMAP_NULL,GX_COLOR0A0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );     // Stage 0
    
    // set vertex descripter
    GXClearVtxDesc();
    GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ, GX_F32, 0 );
    
    // Draw paramters to the window and turn on bilerp filtering
    for ( i = 0; i < sizeof(obj3)/sizeof(Obj3); i ++ )
    {
        cs = cosf( obj3[i].ang * PAI / 180.0f );
        sn = sinf( obj3[i].ang * PAI / 180.0f );
        wcos = obj3[i].w * cs;
        wsin = obj3[i].w * sn;
        hcos = obj3[i].h * cs;
        hsin = obj3[i].h * sn;
        GXSetChanMatColor( GX_COLOR0A0, obj3[i].color );
        GXBegin( GX_QUADS, GX_VTXFMT0, 4 );
            GXPosition3f32( obj3[i].x + wcos + hsin, obj3[i].y,
                            obj3[i].z - wsin + hcos );
            GXPosition3f32( obj3[i].x + wcos - hsin, obj3[i].y,
                            obj3[i].z - wsin - hcos );
            GXPosition3f32( obj3[i].x - wcos - hsin, obj3[i].y,
                            obj3[i].z + wsin - hcos );
            GXPosition3f32( obj3[i].x - wcos + hsin, obj3[i].y,
                            obj3[i].z + wsin + hcos );
        GXEnd();
    }
    
    return;
}

//============================================================================
//  Model 4
//  Wondering depth tags
//============================================================================

// Data area
typedef struct {                // for z coord tag data
  f32     x,  y,  z;
  f32     dx, dy, dz;
  GXColor color;
} Obj4;

Obj4    obj4[10] = {
  {  6.0f,   3.0f,  400.0f,  0.00f,-0.01f, 0.00f, {255,255,255,255} },
  {-20.0f,  10.0f,  400.0f,  0.00f, 0.05f, 0.00f, {255,255,  0,255} },
  {-20.0f,   0.0f,  400.0f,  0.00f,-0.02f, 0.00f, {255,  0,255,255} },
  {-20.0f, -30.0f,  401.0f,  0.00f, 0.04f, 0.00f, {  0,255,255,255} },
  { 10.0f, -30.0f,  399.0f,  0.00f, 0.09f, 0.00f, {255,  0,  0,255} },
  { 40.0f, -30.0f,  500.0f, -0.02f, 0.00f, 0.00f, {  0,  0,255,255} },
  { 40.0f,   0.0f,  300.0f,  0.04f, 0.00f, 0.00f, {  0,255,  0,255} },
  { 40.0f,  50.0f,  402.0f, -0.06f, 0.00f, 0.00f, {255,255,128,255} },
  {  0.0f,  55.0f,  398.0f, -0.07f, 0.00f, 0.00f, {255,128,255,255} },
  {-60.0f,  60.0f,  400.0f,  0.03f, 0.00f, 0.00f, {128,255,255,255} },
};

/*---------------------------------------------------------------------------*
    Name:           cmModel4_Anim
    Description:    Move depth tags
    Arguments:      Model*, DEMOPadStatus*, Camera*
    Returns:        none
 *---------------------------------------------------------------------------*/
static  void    cmModel4_Anim( Model* m, DEMOPadStatus* pad, Camera* c )
{
    #pragma     unused(c)
    s32         i;
    static  u32 anim = 1;
    
    // do animation only if referred
    if ( !(m->flag & MODEL_REFERRED) ) return;
    
    // control animation flag
    if ( pad->buttonDown & PAD_TRIGGER_R ) anim ^= 1;
    if ( anim == 0 ) return;
    
    for ( i = 0; i < sizeof(obj4)/sizeof(Obj4); i ++ )
    {
        obj4[i].x += obj4[i].dx;
        obj4[i].y += obj4[i].dy;
        obj4[i].z += obj4[i].dz;
        if ( obj4[i].x >  300.0f ) obj4[i].x -= 600.0f;
        if ( obj4[i].x < -300.0f ) obj4[i].x += 600.0f;
        if ( obj4[i].y >  300.0f ) obj4[i].y -= 600.0f;
        if ( obj4[i].y < -300.0f ) obj4[i].y += 600.0f;
    }
    return;
}

/*---------------------------------------------------------------------------*
    Name:           cmModel4_Draw
    Description:    Draw the wondering depth tags
    Arguments:      Camera* c
    Returns:        none
 *---------------------------------------------------------------------------*/
static  void    cmModel4_Draw( Camera* c )
{
    s32         i, j;
    Mtx         mmtx;
    
    // Draw grid
    cmModel0_Draw( c );
    
    // Compute model matrix
    for ( i = 0; i < 3; i ++ )
    {
        for ( j = 0; j < 4; j ++ )
        {
            mmtx[i][j] = 0.0f;
        }
    }
    mmtx[0][0] =  2.0f;
    mmtx[1][2] =  1.0f;
    mmtx[2][1] = -2.0f;
    
    // Set modelview matrix
    MTXConcat( c->viewMtx, mmtx, mmtx );
    GXLoadPosMtxImm( mmtx, GX_PNMTX0 );
    GXSetCurrentMtx( GX_PNMTX0 );
    
    // Draw paramters to the window and turn on bilerp filtering
    DEMOLoadFont( GX_TEXMAP0, GX_TEXMTX0, DMTF_BILERP );
    for ( i = 0; i < 10; i ++ )
    {
        DEMOPrintf( (s16)(obj4[i].x-12.0f),
                    (s16)(obj4[i].y- 4.0f),
                    (s16)(obj4[i].z), "%3d", (s32)obj4[i].z );
    }
    
    return;
}

/*======== End of cmn-model.c ========*/
