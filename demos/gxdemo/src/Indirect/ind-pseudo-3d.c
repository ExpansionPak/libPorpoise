/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     ind-psuedo-3d.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Indirect/ind-pseudo-3d.c $
    
    4     5/13/01 1:45a Hirose
    removed obsolete flags
    
    3     10/25/00 9:59p Hirose
    flag change. MAC -> EMU for PC emulator compatibility
    
    2     10/24/00 9:41a Hirose
    added explicit NULL initialization of TEXPalletePtr
    
    1     7/18/00 4:50p Carl
    Demo for pseudo-3D texturing.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

// Demonstrate psuedo-3D maps

#include <demo.h>
#include <stdlib.h>     // for rand()
#include <math.h>       // for sqrtf()

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/

#define TPL_NAME "gxTests/pseudo.tpl"

#define PI    3.14159265358979323846f

#define HFW 40     // height-field width
#define HFH 40     // height-field height (length)

#define NMT  8     // number of tiles

#define TIW 64     // tile width
#define TIH 64     // tile height

#define IMW 128    // indirect map width
#define IMH 4      // indirect map height

/*---------------------------------------------------------------------------*
   Rendering parameters
 *---------------------------------------------------------------------------*/

typedef struct
{
    f32 x, y, z;
    f32 nx, ny, nz;
    f32 s, t;
} coord;

coord height[HFH][HFW];                 // height field coordinates

void *indMap;                           // indirect map data
GXTexObj indMapObj, materialObj;        // texture object data

Mtx cameraMtx, objectMtx;               // persistent matrices

TEXPalettePtr tpl = NULL;               // tpl file data

u32 sendNormal, sendCoords;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void createMap(u8 *tex);
void adjustPoint(coord height[HFH][HFW], u16 r, u16 c, f32 h);
void createHeightField();
void sendVertex(coord *d);
void setupMatrices(void);
void commonInit(void);
void updateMatrices(Mtx obj, Mtx cam);
void textureMapSetup();
void textureMapSetdown();
void renderSetup();
void updateScene(Mtx cam, f32 r1, f32 r2, f32 x, f32 y, f32 z, f32 mag);
void drawHeightField();
void drawScene();
void main(void);
void printIntro( void );

/*---------------------------------------------------------------------------*
   Procedures
 *---------------------------------------------------------------------------*/

// createMap: create the indirect texture map data

void createMap(u8 *tex)
{
    u16 i, j, n;
    u32 off;
    
    for(i=0; i<IMW; i++) 
    {
        for(j=0; j<IMH; j++)
        {
            // compute T offset base (will be multiplied in mtx by tile size)
            n  = (u16) (i * NMT/IMW);

            // add in computed bump alpha
            n |= (u16) (((i) & 0xf)<<4);

            // compute offset, store texel
            off = (u32) ((i&7) | ((i>>3)<<5) | (j<<3));
            tex[off] = (u8) n;
        }
    }
    // very important: must flush data from CPU cache
    DCFlushRange((void *) tex, IMW*IMH);
}

// adjustPoint: adjust height field point vertically based upon random function

void adjustPoint(coord height[HFH][HFW], u16 r, u16 c, f32 h)
{
    s16 i;
    
    i = (s16) ((rand() % 100) - 50);
    h += i/40.0f;
    if (h < -10.0f || h > 10.0f) 
    {
        h -= i/20.0f;
    }
    height[r][c].z = h;
}

// createHeightField: Create height map coordinates

void createHeightField()
{
    u16 r, c;
    f32 h;
    f32 dx, dy;
    f32 imag;
    
    srand(1);

    // initialize the field

    for(r=0; r<HFH; r++) 
    {
        for(c=0; c<HFW; c++) 
        {
            height[r][c].x  = (f32) c - HFW/2.0f;
            height[r][c].y  = (f32) r - HFH/2.0f;
            height[r][c].z  = (f32) 0;
            height[r][c].nx = (f32) 0;
            height[r][c].ny = (f32) 0;
            height[r][c].nz = (f32) 1;
            height[r][c].s  = (f32) (c & 3)/3.0f;
            height[r][c].t  = (f32) (r & 3)/3.0f;
        }
    }

    // tweak the leading edge heights

    for(r=1; r<HFH; r++) 
    {
        adjustPoint(height, r, 0, height[r-1][0].z);
    }
    for(c=1; c<HFW; c++) 
    {
        adjustPoint(height, 0, c, height[0][c-1].z);
    }
    
    // tweak the interior heights

    for(r=1; r<HFH; r++) 
    {
        for(c=1; c<HFW; c++) 
        {
            h = (height[r-1][c].z + height[r][c-1].z)/2.0f;
            adjustPoint(height, r, c, h);
        }
    }

    // compute normals

    for(r=0; r<HFH; r++) 
    {
        for(c=0; c<HFW; c++) 
        {
            if (c < 1) 
            {
                dx = height[r][c+1].z - height[r][c].z;
            }
            else if (c < HFW-1)
            {
                dx = height[r][c+1].z - height[r][c-1].z;
            }
            else // c == HFW-1
            {
                dx = height[r][c].z - height[r][c-1].z;
            }

            if (r < 1) 
            {
                dy = height[r+1][c].z - height[r][c].z;
            }
            else if (c < HFH-1)
            {
                dy = height[r+1][c].z - height[r-1][c].z;
            }
            else // r == HFH-1
            {
                dy = height[r][c].z - height[r-1][c].z;
            }
            
            imag = 1.0f / sqrtf((dx * dx) + (dy * dy) + 1);
            
            height[r][c].nx =  -dx * imag;
            height[r][c].ny =  -dy * imag;
            height[r][c].nz = 1.0f * imag;
        }
    }
}

// sendVertex: Send one vertex from the given coordinate structure

void sendVertex(coord *d)
{
    GXPosition3f32(d->x, d->y, d->z);

    if (sendNormal)
        GXNormal3f32(d->nx, d->ny, d->nz);

    if (sendCoords)
        GXTexCoord2f32(d->s, d->t);
}

// setupMatrices: Set up persistent matrices

void setupMatrices(void)
{
    Point3d position = {0, -20, 5};
    Point3d target   = {0, 0, 0};
    Vec	    up       = {0, 0, 1};

    MTXLookAt(cameraMtx, &position, &up, &target );

    MTXIdentity(objectMtx);
}

// commonInit: various state initialization

void commonInit(void)
{
    GXColor back = {0, 0, 128, 255};

    DEMOInit(NULL);
    GXSetCopyClear( back, GX_MAX_Z24 );

    GXSetZMode(TRUE, GX_LEQUAL, TRUE);
    GXSetZCompLoc(TRUE);
    GXSetCullMode(GX_CULL_NONE);

    setupMatrices();
}

// updateMatrices: Set up position/normal/texture matrices and load them

void updateMatrices(Mtx obj, Mtx cam)
{
    Mtx vertexMtx, normalMtx, heightMtx;
    Mtx textureMtx, tempMtx;

    // matrix for transforming vertices from object space to eye space.
    MTXConcat(cam, obj, vertexMtx);

    // the normal mtx starts out as the inverse transpose of the vertex mtx
    if (!MTXInverse(vertexMtx, tempMtx))
        { ASSERTMSG(0,"Singular matrix!\n"); }
    MTXTranspose(tempMtx, normalMtx);

    // matrix for specifying material repetitions
    MTXScale(textureMtx, 1.0f, 1.0f, 1.0f); // no repetitions for this demo

    // matrix for mapping height to indirect index (0-6)
    // S gets Z scaled and translated, T gets nothing
    heightMtx[0][0] =  0.0f;
    heightMtx[0][1] =  0.0f;
    heightMtx[0][2] = -0.875f/20.0f;
    heightMtx[0][3] =  7.0f/16.0f;
    heightMtx[1][0] =  0.0f;
    heightMtx[1][1] =  0.0f;
    heightMtx[1][2] =  0.0f;
    heightMtx[1][3] =  0.0f;

    GXLoadPosMtxImm(vertexMtx,  GX_PNMTX0);
    GXLoadNrmMtxImm(normalMtx,  GX_PNMTX0);
    GXLoadTexMtxImm(textureMtx, GX_TEXMTX0, GX_MTX2x4);
    GXLoadTexMtxImm(heightMtx,  GX_TEXMTX1, GX_MTX2x4);
}

// textureMapSetup: Initial setup for texture maps

void textureMapSetup(void)
{
    // Pre-existing textures are in a tpl file.
    TEXGetPalette(&tpl, TPL_NAME);

    // Indirect Map Texture (generated)

    // Allocate texture in main memory.
    indMap = OSAlloc(GXGetTexBufferSize(IMW, IMH, GX_TF_IA4, FALSE, 0));

    // Create the actual map
    createMap((u8 *) indMap);

    // Create texture object.
    GXInitTexObj(&indMapObj, indMap, IMW, IMH, GX_TF_IA4,
                 GX_CLAMP, GX_CLAMP, FALSE);

    // Must set sampling to NEAR to avoid bad artifacts
    GXInitTexObjLOD( &indMapObj,
        GX_NEAR, GX_NEAR, 0.0F, 0.0F, 0.0F,
        GX_FALSE, GX_FALSE, GX_ANISO_1 );

    GXLoadTexObj(&indMapObj, GX_TEXMAP1);   // Indirect map will be TEXMAP1.

    // Material Texture (pre-existing)
    //
    TEXGetGXTexObjFromPalette(tpl, &materialObj, 0);

    // Set sampling to NEAR to avoid boundary artifact
    GXInitTexObjLOD( &materialObj,
        GX_NEAR, GX_NEAR, 0.0F, 0.0F, 0.0F,
        GX_FALSE, GX_FALSE, GX_ANISO_1 );

    GXLoadTexObj(&materialObj, GX_TEXMAP0); // Material will be TEXMAP0

    // Set up texgen for rendering. The matrices can be changed per-object,
    // but the texgen needn't change.
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX1);

    // Treat the material texture as if it is only one tile large.
    // Subtract one to deal with boundary artifact
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_ENABLE, TIW-1, TIH-1);

    // Use position matrix 0 for geometry.
    GXSetCurrentMtx(GX_PNMTX0);
}

// textureMapSetdown: free up texture space

void textureMapSetdown(void)
{
    TEXReleasePalette(&tpl);

    OSFree(indMap);
}

// renderSetup: setup for actual render

void renderSetup(void)
{
    Mtx44 pMtx;
    f32 asp = 10.0f/7.0f;
    GXLightObj myLight;
    GXColor white = {255, 255, 255, 255};
    GXColor grey  = { 63,  63,  63, 255};
    
    // Perspective projection
    MTXFrustum(pMtx, 1, -1, -1*asp, 1*asp, 1, 50);
    GXSetProjection(pMtx, GX_PERSPECTIVE);

    // Set up a light
    GXInitLightPos(&myLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&myLight, white);
    GXLoadLightObjImm(&myLight, GX_LIGHT0);

    // Enable it in one channel
    GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
                  GX_DF_CLAMP, GX_AF_NONE);

    // default mat color = white; set ambient
    GXSetChanAmbColor(GX_COLOR0A0, grey);

#ifndef EMU
    // total of 4 rendering stages per pixel
    GXSetNumTevStages(3);
    GXSetNumIndStages(1);
#endif
    // Two texture coordinates, one rasterized color.
    GXSetNumTexGens(2);
    GXSetNumChans(1);

    // Indirect Stage 0 -- Sample pseudo-3D map
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);

    // TEV Stage 0 -- Apply first layer of pseudo-3D
    //
    // TEVPREV = TEXC/TEXA
    //
    GXSetTevIndTile(GX_TEVSTAGE0,       // tev stage
                    GX_INDTEXSTAGE0,    // ind stage
                    TIW,                // tile size S
                    TIH,                // tile size T
                    0,                  // tile spacing S
                    TIH,                // tile spacing T
                    GX_ITF_4,           // ind tex format
                    GX_ITM_0,           // ind matrix select
                    GX_ITB_NONE,        // bias select
                    GX_ITBA_OFF);       // bump alpha select

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);

#ifndef EMU
    // TEV Stage 1 -- Apply second layer of pseudo 3D
    //                Use bump alpha (normalized) to blend with first layer
    //                The second layer is "lower" than the first.
    //
    //              A        1-C       B        C      D
    // TEVPREVC = PREVC * (1-BUMPA) + TEXC * (BUMPA) + 0
    // TEVPREVA = PREVA
    //
    GXSetTevIndTile(GX_TEVSTAGE1,       // tev stage
                    GX_INDTEXSTAGE0,    // ind stage
                    TIW,                // tile size S
                    TIH,                // tile size T
                    0,                  // tile spacing S
                    TIH,                // tile spacing T
                    GX_ITF_4,           // ind tex format
                    GX_ITM_0,           // ind matrix select
                    GX_ITB_T,           // bias select
                    GX_ITBA_S);         // bump alpha select

    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_ALPHA_BUMP);
    GXSetTevColorIn(GX_TEVSTAGE1,
                    GX_CC_CPREV, GX_CC_TEXC, GX_CC_RASA, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1,
                    GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevColorOp(GX_TEVSTAGE1,
                    GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1,
                    GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);

    // TEV Stage 2 -- Apply lighting
    //
    // TEVPREVC = PREVC * RASC
    //
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2,
                    GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE2,
                    GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevColorOp(GX_TEVSTAGE2,
                    GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2,
                    GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);
#endif
    // Vertex packet specification -- Position, normal, and
    //                                one pair of texture coordinates.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    sendNormal = TRUE;
    sendCoords = TRUE;
}

// updateScene: Prepare for scene drawing.

void updateScene(Mtx cam, f32 r1, f32 r2, f32 x, f32 y, f32 z, f32 mag)
{
    Mtx myobjMtx, tempMtx;

    // update object matrix
    MTXRotDeg(tempMtx, 'x', -r2);
    MTXConcat(tempMtx, objectMtx, objectMtx);
    MTXRotDeg(tempMtx, 'z', r1);
    MTXConcat(tempMtx, objectMtx, objectMtx);
    MTXTrans(tempMtx, x, y, z);
    MTXConcat(tempMtx, objectMtx, myobjMtx);
    MTXScale(tempMtx, mag, mag, mag);
    MTXConcat(tempMtx, myobjMtx, myobjMtx);

    // Create all other matrices derived from this object matrix.
    updateMatrices(myobjMtx, cam);
}

// drawHeightField: Draw the height field using T-strips

void drawHeightField()
{
    u16 r, c;
    
    for(r=0; r<HFH-1; r++) 
    {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, HFW*2 );

        for(c=0; c<HFW; c++) 
        {
            sendVertex(&height[r][c]);
            sendVertex(&height[r+1][c]);
        }

        GXEnd();
    }
}

// drawScene: Draw the full scene

void drawScene()
{
    // The scene consists only of the height field
    drawHeightField();
}

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main(void)
{
    f32 rot1;
    f32 rot2;
    f32 mag;
    f32 xp, yp, zp;

    commonInit();       // Initialize the pipe

    rot1 = rot2 = 0.0f;
    mag = 1.0f;
    xp = yp = zp = 0.0f;

    textureMapSetup();  // Setup texture objects and all other static things

    createHeightField();

    printIntro();

    renderSetup();      // Setup for actual render

    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {
        DEMOPadRead();

        rot1 = DEMOPadGetStickX(0)/10.0f;
        rot2 = DEMOPadGetStickY(0)/10.0f;

        xp += DEMOPadGetSubStickX(0)/100.0f;
        zp += DEMOPadGetSubStickY(0)/100.0f;

        if (DEMOPadGetButtonDown(0) & PAD_BUTTON_A)
        mag *= 1.25f;
    
        if (DEMOPadGetButtonDown(0) & PAD_BUTTON_B)
        mag *= 0.8f;
    
        DEMOBeforeRender();

        updateScene(cameraMtx, rot1, rot2, xp, yp, zp, mag);

        drawScene();

        DEMODoneRender();
    }

    textureMapSetdown(); // Free memory and such.

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
    Name:           printIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void printIntro( void )
{
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("ind-pseudo-3D: demonstrate pseudo-3D texturing\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("  Stick X/Y    : rotate model\n");
    OSReport("  Substick X/Y : translate model\n");
    OSReport("  A Button     : zoom in\n");
    OSReport("  B Button     : zoom out\n");
    OSReport("************************************************\n\n");
}

