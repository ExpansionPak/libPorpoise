/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-spheremap.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-spheremap.c $
    
    7     6/01/01 12:43a Hirose
    fixed normal texgen calculation. fixed MAC/HW1 build error too.
    
    6     5/27/01 9:40p Hirose
    added an example 3D object which uses spherical environment mapping.
    
    5     5/12/01 2:13a Hirose
    removed obsolete flags.
    
    4     11/01/00 4:25p Carl
    Changed choices for what can be displayed.
    
    3     5/27/00 11:25a Alligator
    change cube maps used in demo
    
    2     5/21/00 10:51p Alligator
    fixed clamp mode, cleaned up code
    
    1     5/12/00 4:49p Alligator
    initial checkin
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   tg-spheremap
     An example of environment mapping by sphere map (made from cube map)
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define SPHERE_MAP_SIZE  256
#define SPHERE_MAP_FMT   GX_TF_RGB565
#define SPHERE_MAP_TESS  30

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
// defined in spheremap.c so we can look at individual maps and alpha
extern u8 CubeFaceStart;
extern u8 CubeFaceEnd;
extern u8 CubeTevMode;

// static
static Mtx  ViewMtx;
static Mtx  ModelMtx;
static u8   CubeFaceDisp;

static Vec CamLoc = {0.0F, 0.0F, 4.0F};
static Vec UP     = {0.0F, 1.0F, 0.0F};
static Vec ObjPt  = {0.0F, 0.0F, 0.0F};

static u8 CurrentTexture;

static void *mySphere;          // sphere geometry display list
static u32   mySphereSz;        // sphere geometry display list size

static TEXPalettePtr tpl0 = 0;  // cube map files
static TEXPalettePtr tpl1 = 0;

static GXTexObj CubeMap0[6];    // a couple of pre-made cube maps
static GXTexObj CubeMap1[6];
static GXTexObj SphereMap;      // sphere map that is generated from cube map

/*---------------------------------------------------------------------------*
   External function references
 *---------------------------------------------------------------------------*/
// from spheremap.c
extern void genMapSphere        ( void**    display_list, 
                                  u32*      size, 
                                  u16       tess,
                                  GXVtxFmt  fmt );

extern void drawSphereMap       ( GXTexObj* cubemap, 
                                  GXTexObj* spheremap,
                                  void*     dl, 
                                  u32       dlsz );

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main                    ( void );
static void DrawInit                ( void );
static void DrawTick                ( void );
static void InitTexGenMethod        ( void );
static void AnimTick                ( void );
static void SetTexGenForSphereMap   ( Mtx nMtx );
static void SetCamera               ( void );
static void PrintIntro              ( void );

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);

    DrawInit();
    PrintIntro();

    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {
        DEMOPadRead();

        AnimTick();
        DEMOBeforeRender();
        DrawTick();

        DEMODoneRender();
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes textures, some permanent GX settings and
                    all scene parameters.
                    
    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit    ( void )
{
    u32              i;
    TEXDescriptorPtr tdp;
    void*            tex_buffer;

    GXSetCullMode(GX_CULL_BACK);

    // sphere map geometry is using format 7
    // this is for textured quad geometry
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    //
    // Initialize texture object for sphere map
    //   - the data will be filled in each time drawSphereMap is called
    //
    tex_buffer = (void*)OSAlloc(
            GXGetTexBufferSize(SPHERE_MAP_SIZE, SPHERE_MAP_SIZE, 
                               (u32)SPHERE_MAP_FMT,  GX_FALSE, 0));
                               
    OSReport("tex %08x\n", tex_buffer);
    
    GXInitTexObj(
        &SphereMap,
        tex_buffer,
        SPHERE_MAP_SIZE,
        SPHERE_MAP_SIZE,
        SPHERE_MAP_FMT,
        GX_CLAMP,
        GX_CLAMP,
        GX_FALSE);

    // read textures
    // order of loading: right, front, left, back, top, bottom
    OSReport("Opening gxTests/tg-cube.tpl\n");
    TEXGetPalette(&tpl0, "gxTests/tg-cube.tpl");

    for (i = 0; i < 6; i++) 
    {
        tdp = TEXGet(tpl0, i);
        GXInitTexObj(&CubeMap0[i], 
                 tdp->textureHeader->data, 
                 tdp->textureHeader->width, 
                 tdp->textureHeader->height, 
                 (GXTexFmt)tdp->textureHeader->format,
                 GX_CLAMP, 
                 GX_CLAMP, 
                 GX_FALSE); 
    
        // alpha should be zero on edges, clamp so sphere outside
        // projected texture is not overwritten
        GXInitTexObjLOD(&CubeMap0[i], 
                    tdp->textureHeader->minFilter, 
                    tdp->textureHeader->magFilter, 
                    tdp->textureHeader->minLOD, 
                    tdp->textureHeader->maxLOD, 
                    tdp->textureHeader->LODBias, 
                    GX_FALSE,
                    tdp->textureHeader->edgeLODEnable,
                    GX_ANISO_1); 
    }

    // read pre-made cube map textures
    // order of loading: right, front, left, back, top, bottom
    OSReport("Opening gxTests/tg-cube1.tpl\n");
    TEXGetPalette(&tpl1, "gxTests/tg-cube1.tpl");

    for (i = 0; i < 6; i++) 
    {
        tdp = TEXGet(tpl1, i);
        GXInitTexObj(&CubeMap1[i], 
                 tdp->textureHeader->data, 
                 tdp->textureHeader->width, 
                 tdp->textureHeader->height, 
                 (GXTexFmt)tdp->textureHeader->format,
                 GX_CLAMP, 
                 GX_CLAMP, 
                 GX_FALSE); 
    
        // alpha should be zero on edges, clamp so sphere outside
        // projected texture is not overwritten
        GXInitTexObjLOD(&CubeMap1[i], 
                    tdp->textureHeader->minFilter, 
                    tdp->textureHeader->magFilter, 
                    tdp->textureHeader->minLOD, 
                    tdp->textureHeader->maxLOD, 
                    tdp->textureHeader->LODBias, 
                    GX_FALSE,
                    tdp->textureHeader->edgeLODEnable,
                    GX_ANISO_1); 
    }

    // generate sphere geometry once
#ifndef MAC
    genMapSphere(&mySphere, &mySphereSz, SPHERE_MAP_TESS, GX_VTXFMT7);
#endif

    // initialization for contorable parameters
    CubeFaceDisp  = 6;
    CubeFaceStart = 0;
    CubeFaceEnd   = 6;
    
    MTXIdentity(ModelMtx);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Create sphere environment map from cube textures and
                    draw it on a quad. Displays an example 3D object (torus)
                    with texgen for sphere environment mapping.
                    
    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick ( void )
{
    Mtx   mv;

    SetCamera();

    // create sphere map texture
    if (CurrentTexture)
        drawSphereMap(&CubeMap1[0], &SphereMap, mySphere, mySphereSz);
    else
        drawSphereMap(&CubeMap0[0], &SphereMap, mySphere, mySphereSz);

    // fill screen by another background color
    GXSetCopyClear((GXColor){ 0x60, 0x60, 0x60, 0xFF }, 0x00FFFFFF);
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);

    // Texture/rendering path setting
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);   
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetNumChans(0);

    GXInvalidateTexAll();
    GXLoadTexObj(&SphereMap, GX_TEXMAP0);    
   
    // Draw sphere map texture on a quad
    //  (no distortion)
    GXSetViewport(16.0F, 16.0F, 256.0F, 256.0F, 0.0F, 1.0F);

    GXLoadPosMtxImm(ViewMtx, GX_PNMTX0);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-1.0F, 1.0F, 0.0F);
        GXTexCoord2f32(0.0F, 0.0F);
        GXPosition3f32(1.0F, 1.0F, 0.0F);
        GXTexCoord2f32(1.0F, 0.0F);
        GXPosition3f32(1.0F, -1.0F, 0.0F);
        GXTexCoord2f32(1.0F, 1.0F);
        GXPosition3f32(-1.0F, -1.0F, 0.0F);
        GXTexCoord2f32(0.0F, 1.0F);
    GXEnd();

    // Draw environment mapped torus
    //  (uses normal-based spherical projection)
    GXSetViewport(320.0F, 120.0F, 256.0F, 256.0F, 0.0F, 1.0F);

    MTXConcat(ViewMtx, ModelMtx, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInvXpose(mv, mv);
    SetTexGenForSphereMap(mv);

    GXDrawTorus(0.25F, 32, 24);

    // Set background color for the first pass.
    GXSetCopyClear((GXColor){ 0, 0, 0, 0 }, 0x00FFFFFF);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick ( void )
{
    Mtx  mrx, mry;
    u16  buttons = DEMOPadGetButtonDown(0);

    MTXRotDeg(mry, 'x', -(f32)(DEMOPadGetStickY(0))/16.0F);
    MTXRotDeg(mrx, 'y', (f32)(DEMOPadGetStickX(0))/16.0F);
    MTXConcat(mry, ModelMtx, ModelMtx);
    MTXConcat(mrx, ModelMtx, ModelMtx);

    if (buttons & PAD_BUTTON_B)
    {
        CubeFaceDisp++;
        CubeFaceDisp %= 7;
        
        CubeFaceStart = CubeFaceDisp;
        CubeFaceEnd   = (u8)(CubeFaceStart + 1);

        if ( CubeFaceDisp == 6 )
        {
            CubeFaceStart = 0;
            CubeFaceEnd   = 6;
        }

        switch ( CubeFaceDisp )
        {
            case 0: OSReport("right face\n");   break;
            case 1: OSReport("front face\n");   break;
            case 2: OSReport("left face\n");    break;
            case 3: OSReport("back face\n");    break;
            case 4: OSReport("top face\n");     break;
            case 5: OSReport("bottom face\n");  break;
            case 6: OSReport("all faces\n");    break;
        }
    }

    if (buttons & PAD_BUTTON_Y)
    {
        CubeFaceDisp  = 6;
        CubeFaceStart = 0;
        CubeFaceEnd   = 6;
        OSReport("all faces\n");
    }

    if (buttons & PAD_BUTTON_A)
    {
        CurrentTexture ^= 1;
        OSReport("Texture %d\n", CurrentTexture);
    }
    
    if (buttons & PAD_TRIGGER_Z)
    {
        CubeTevMode++;
        if (CubeTevMode > 4)
            CubeTevMode = 0;
            
        switch (CubeTevMode)
        {
    	    case 0: OSReport("Final result\n"); break;
    	    case 1: OSReport("Unclipped texture\n"); break;
    	    case 2: OSReport("texture alpha\n"); break;
    	    case 3: OSReport("Q-clipping (raster) alpha\n"); break;
    	    case 4: OSReport("raster*texture alpha\n"); break;
        }
    }
}

/*---------------------------------------------------------------------------*
    Name:           SetTexGenForSphereMap
    
    Description:    Set up texgen for doing spherical environment mapping
                    
    Arguments:      nMtx : view normal transformation matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetTexGenForSphereMap( Mtx nMtx )
{
    Mtx  m0, m1, m2;

    MTXScale(m1, 0.50F, -0.50F, 0.0F);
    MTXTrans(m0, 0.50F, 0.50F, 1.0F);
    MTXConcat(m0, m1, m2);
#if ( GX_REV != 1 )
    GXLoadTexMtxImm(nMtx, GX_TEXMTX0, GX_MTX3x4);
    GXLoadTexMtxImm(m2, GX_PTTEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM,
                      GX_TEXMTX0, GX_TRUE, GX_PTTEXMTX0);
#else
    MTXConcat(m2, nMtx, m0);
    GXLoadTexMtxImm(nMtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0);
#endif
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    Set projection and view matrix
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetCamera ( void )
{
    Mtx44 p;

    MTXOrtho(p, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 100.0f);
    GXSetProjection(p, GX_ORTHOGRAPHIC);
   
    MTXLookAt(ViewMtx, &CamLoc, &UP, &ObjPt);
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
    OSReport("******************************************************\n");
    OSReport("tg-spheremap: create a sphere map from a cube map\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick : rotate the torus\n");
    OSReport("A button   : select the environment cube map texture\n");
    OSReport("B button   : display individual cube faces\n");
    OSReport("Y button   : display all cube faces in the sphere map\n");
    OSReport("Z button   : select alpha/color channel to display\n");
    OSReport("******************************************************\n");
    OSReport("\n\n");
}

/*===========================================================================*/
