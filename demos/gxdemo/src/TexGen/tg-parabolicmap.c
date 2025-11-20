/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-spheremap.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-parabolicmap.c $
    
    5     5/12/01 2:13a Hirose
    removed obsolete flags.
    
    4     11/01/00 4:47p Carl
    Fixed face labels.  Changed output options.
    
    3     5/27/00 11:25a Alligator
    change cube maps used in demo
    
    2     5/21/00 10:51p Alligator
    fixed clamp mode, cleaned up code
    
    1     5/12/00 4:49p Alligator
    initial checkin
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

#define SPHERE_MAP_SIZE  128
#define SPHERE_MAP_FMT   GX_TF_RGB565
#define SPHERE_MAP_TESS  40

/*>*******************************(*)*******************************<*/
// defined in spheremap.c so we can look at individual maps and alpha
extern u8 CubeFaceStart;
extern u8 CubeFaceEnd;
extern u8 CubeTevMode;

/*>*******************************(*)*******************************<*/
static Mtx v, m, mv;
static Mtx44 proj;

static Vec CamLoc = {0.0F, 0.0F, 6.0F};
static Vec UP     = {0.0F, 1.0F, 0.0F};
static Vec ObjPt  = {0.0F, 0.0F, 0.0F};

static u8 CurrentTexture;

static void *mySphere;   // sphere geometry display list
static u32   mySphereSz; // sphere geometry display list size

static TEXPalettePtr tpl0 = 0; // cube map files
static TEXPalettePtr tpl1 = 0;

static GXTexObj CubeMap0[6];    // a couple of pre-made cube maps
static GXTexObj CubeMap1[6];
static GXTexObj SphereMap[2];   // sphere map that is generated

/*>*******************************(*)*******************************<*/
void        main                ( void );
static void InitCamera          ( void );
static void DrawInit            ( void );
static void DrawTick            ( void );
static void InitTexGenMethod    ( void );
static void AnimTick            ( void );
static void PrintIntro          ( void );

/*>*******************************(*)*******************************<*/
//
// from spheremap.c
// 
extern void genMapSphere        ( void**    display_list, 
                                  u32*      size, 
                                  u16       tess,
                                  GXVtxFmt  fmt );

extern void drawParaboloidMap   ( GXTexObj* cubemap, 
                                  GXTexObj* spheremap,
                                  void*     dl, 
                                  u32       dlsz,
                                  GXBool    front );
/*>*******************************(*)*******************************<*/


void main ( void )
{
    DEMOInit(NULL);

    InitCamera();

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

/*>*******************************(*)*******************************<*/
static void InitCamera ( void )
{
    Mtx44 p;

    MTXOrtho(p, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 100.0f);
    GXSetProjection(p, GX_ORTHOGRAPHIC);
   
    MTXLookAt(v, &CamLoc, &UP, &ObjPt);
}

/*>*******************************(*)*******************************<*/
static void DrawInit    ( void )
{
    u32              i;
    TEXDescriptorPtr tdp;
    void*            tex_buffer;

    GXSetCullMode(GX_CULL_BACK);

    // no zbuffer needed to create the sphere map
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);

    // sphere map geometry is using format 7
    // this is for textured quad geometry
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    //
    // Initialize texture object for sphere map
    //   - the data will be filled in each time drawSphereMap is called
    //
    for (i = 0; i < 2; i++)
    {
        tex_buffer = (void*)OSAlloc(
            GXGetTexBufferSize(SPHERE_MAP_SIZE, SPHERE_MAP_SIZE, 
                               (u32)SPHERE_MAP_FMT,  GX_FALSE, 0));

        GXInitTexObj(
            &SphereMap[i],
            tex_buffer,
            SPHERE_MAP_SIZE,
            SPHERE_MAP_SIZE,
            SPHERE_MAP_FMT,
            GX_CLAMP,
            GX_CLAMP,
            GX_FALSE);
    }

    // read pre-made cube map textures
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
}

/*>*******************************(*)*******************************<*/
static void DrawTick    ( void )
{
    // create sphere map texture
    if (CurrentTexture) {
        drawParaboloidMap(&CubeMap1[0], &SphereMap[0], mySphere, mySphereSz, 0);
        drawParaboloidMap(&CubeMap1[0], &SphereMap[1], mySphere, mySphereSz, 1);
    }
    else {
        drawParaboloidMap(&CubeMap0[0], &SphereMap[0], mySphere, mySphereSz, 0);
        drawParaboloidMap(&CubeMap0[0], &SphereMap[1], mySphere, mySphereSz, 1);
    }

    //
    //  Draw sphere map texture on a quad
    //
    GXSetViewport(16.0F, 16.0F, 
                  400.0F, 400.0F, 
                  0.0F, 1.0F);

    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL); 
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetNumChans(0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
   
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GXInvalidateTexAll();
    GXLoadTexObj(&SphereMap[0], GX_TEXMAP0);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-1.0F, 1.0F, 0.0F);
        GXTexCoord2f32( 0.0F, 0.0F);
        GXPosition3f32( 0.0F, 1.0F, 0.0F);
        GXTexCoord2f32( 1.0F, 0.0F);
        GXPosition3f32( 0.0F, 0.0F, 0.0F);
        GXTexCoord2f32( 1.0F, 1.0F);
        GXPosition3f32(-1.0F, 0.0F, 0.0F);
        GXTexCoord2f32( 0.0F, 1.0F);
    GXEnd();

    GXLoadTexObj(&SphereMap[1], GX_TEXMAP0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(0.0F, 1.0F, 0.0F);
        GXTexCoord2f32(0.0F, 0.0F);
        GXPosition3f32(1.0F, 1.0F, 0.0F);
        GXTexCoord2f32(1.0F, 0.0F);
        GXPosition3f32(1.0F, 0.0F, 0.0F);
        GXTexCoord2f32(1.0F, 1.0F);
        GXPosition3f32(0.0F, 0.0F, 0.0F);
        GXTexCoord2f32(0.0F, 1.0F);
    GXEnd();
}

/*>*******************************(*)*******************************<*/
static void AnimTick    ( void )
{
    u16 buttons = DEMOPadGetButtonDown(0);

    MTXIdentity(m);
    MTXConcat(v, m, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, m);
    MTXTranspose(m, m);
    GXLoadNrmMtxImm(m, GX_PNMTX0);

    if (buttons & PAD_BUTTON_B) {
        CubeFaceStart++;
        CubeFaceEnd = (u8)(CubeFaceStart + 1);

        if (CubeFaceStart == 5) {
            CubeFaceStart = 0;
            CubeFaceEnd   = 1;
        }

        switch(CubeFaceStart)
        {
            case 0: OSReport("left/right face\n"); break;
            case 1: OSReport("right/front face\n"); break;
            case 2: OSReport("back/left face\n"); break;
            case 3: OSReport("bottom/top face\n"); break;
            case 4: OSReport("top/bottom face\n"); break;
        }
    }

    if (buttons & PAD_BUTTON_A) {
        CubeFaceStart = 0;
        CubeFaceEnd   = 5;
        OSReport("all faces\n");
    }

    if (buttons & PAD_BUTTON_X) {
        CurrentTexture ^= 1;
        OSReport("Texture %d\n", CurrentTexture);
    }
    
    if (buttons & PAD_BUTTON_Y) {
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
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n");
    OSReport("*************************************************************\n");
    OSReport("tg-parabolicmap: create a dual-paraboloid map from a cube map\n");
    OSReport("*************************************************************\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
    OSReport("A button   : display all cube faces in the paraboloid map\n");
    OSReport("B button   : display individual cube faces\n");
    OSReport("X button   : select the cube map texture\n");
    OSReport("Y button   : select alpha/color channel to display\n");
    OSReport("******************************************************\n");
    OSReport("\n\n");
}

/*===========================================================================*/