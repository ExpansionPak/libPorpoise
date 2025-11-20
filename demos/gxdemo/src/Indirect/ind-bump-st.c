/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     ind-bump-st.c

  Copyright 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Indirect/ind-bump-st.c $
    
    7     5/13/01 1:45a Hirose
    removed obsolete flags
    
    6     10/30/00 12:19p Carl
    Fixed warning.
    
    5     10/24/00 9:41a Hirose
    added explicit NULL initialization of TEXPalletePtr
    
    4     10/18/00 10:28a Hirose
    removed GXSetTevClampMode calls / relaced MAC flag by EMU
    
    3     7/07/00 5:57p Dante
    PC Compatibility
    
    2     7/01/00 2:35p Alligator
    added intro
    
    1     6/22/00 6:51p Carl
    Indirect bump ST demo
    
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/******************************************************************************
 *
 *   (C) 1999, 2000 ARTX INC..  ALL RIGHTS RESERVED.  UNPUBLISHED -- RIGHTS
 *   RESERVED UNDER THE COPYRIGHT LAWS OF THE UNITED STATES.  USE OF A
 *   COPYRIGHT NOTICE IS PRECAUTIONARY ONLY AND DOES NOT IMPLY PUBLICATION
 *   OR DISCLOSURE.
 *
 *   THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 *   ARTX INC..  USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 *   THE PRIOR EXPRESS WRITTEN PERMISSION OF ARTX INC..
 *
 *                   RESTRICTED RIGHTS LEGEND
 *
 *   Use, duplication, or disclosure by the Government is subject to
 *   restrictions as set forth in subparagraph (c)(1)(ii) of the Rights
 *   in Technical Data and Computer Software clause at DFARS 252.227-7013
 *   or subparagraphs (c)(1) and (2) of Commercial Computer Software --
 *   Restricted Rights at 48 CFR 52.227-19, as applicable.
 *
 *   ArtX Inc.
 *   3400 Hillview Ave, Bldg 5
 *   Palo Alto, CA 94304
 *
 *****************************************************************************/

// Bump mapping test for bumpST method

#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/

#define TPL_NAME "gxTests/dnt-02.tpl"

#define CAM_DIST 4                // Camera's distance from origin.
#define LIT_X_ANGLE 55.0f         // Rotation around X for light.
#define LIT_Y_ANGLE 45.0f         // Rotation around Y for light.

#define LIT_TEX_FMT GX_TF_IA8     // 8-bits each for diffuse and specular.

#define PI    3.14159265358979323846f

#define NRM_SCALE 0.8f

/*---------------------------------------------------------------------------*
   Rendering parameters
 *---------------------------------------------------------------------------*/

u16 W=640, H=448;                 // Display size
u16 IW=256, IH=256;               // Indirect (bump) map size.
u16 TW=64, TH=64;                 // Light map size.
u16 TT=32, ST=32;                 // Light map tesselation
u16 TRS=5, TRT=3;                 // Material repetitions around donut.
u32 N1=20, N2=10;                 // Donut tesselation
f32 R1=1.0f, R2=0.5f;             // Donut radii.

typedef struct
{
    f32 x, y, z;
    f32 nx, ny, nz;
    f32 bx, by, bz;
    f32 tx, ty, tz;
    f32 s, t;
} coord;

void * lightMap;
GXTexObj lightMapObj, materialObj, nPtrbObj;

Mtx cameraMtx, objectMtx, lightMtx;

coord * donutData;

TEXPalettePtr tpl = NULL;

u32 sendNormal, sendBinormals, sendCoords;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

coord donutCoord (u32 a, u32 b, u32 N1, u32 N2, f32 R1, f32 R2);
void createDonut();
void sendVertex(coord *d);
void setupMatrices(void);
void setupSize(u16 w, u16 h);
void commonInit(void);
void updateMatrices(u32 id, Mtx obj, Mtx cam);
void setLight(GXLightID litID, GXChannelID diffID, GXChannelID specID,
              Mtx obj, Mtx cam);
void bumpMapSetup();
void bumpMapSetdown();
void lightMapSetup();
void renderSetup1();
void renderSetup2();
void updateDonut(Mtx cam, f32 r1, f32 r2, f32 x, f32 y, f32 z);
void drawDonut();
void drawHemisphere();
void showMaps();
void drawScene(Mtx cam, f32 r1, f32 r2);
void main(void);
static void PrintIntro( void );

/*---------------------------------------------------------------------------*
   Procedures
 *---------------------------------------------------------------------------*/

// Fill out a coordinate entry for a donut vertex.
coord donutCoord (u32 a, u32 b, u32 N1, u32 N2, f32 R1, f32 R2)
{
    coord result;
    f32 angle1, angle2;

    angle1 = (f32) a*2*PI/N1;
    angle2 = (f32) b*2*PI/N2;

    result.x = (R1 - (R2*cosf(angle2))) * cosf(angle1);
    result.y = (R1 - (R2*cosf(angle2))) * sinf(angle1);
    result.z = R2 * sinf(angle2);

    result.nx = - cosf(angle2) * cosf(angle1);
    result.ny = - cosf(angle2) * sinf(angle1);
    result.nz = sinf(angle2);

    result.bx = - sinf(angle1);
    result.by = cosf(angle1);
    result.bz = 0;

    result.tx = sinf(angle2) * cosf(angle1);
    result.ty = sinf(angle2) * sinf(angle1);
    result.tz = cosf(angle2);

    result.s = (f32) a/(f32) N1;
    result.t = (f32) b/(f32) N2;

    return result;
}

// Make coordinate structures for all points on the donut.
void createDonut()
{
    u32 i,j;

    for (i=0; i<=N1; i++)
        for (j=0; j<=N2; j++)
            donutData[i*(N2+1) + j] = donutCoord(i, j, N1, N2, R1, R2);
}

// Send one vertex from the given coordinate structure, sending only
// the information specified in the global variables:
// sendNormal, sendBinormals, sendCoords.
void sendVertex(coord *d)
{
    GXPosition3f32(d->x, d->y, d->z);

    if (sendNormal)
        GXNormal3f32(d->nx, d->ny, d->nz);

    if (sendNormal && sendBinormals)
    {
        GXNormal3f32(d->bx, d->by, d->bz);
        GXNormal3f32(d->tx, d->ty, d->tz);
    }

    if (sendCoords)
        GXTexCoord2f32(d->s, d->t);
}

// Set up static matrices
void setupMatrices(void)
{
    Mtx tempMtx;

    MTXTrans(cameraMtx, 0, 0, -CAM_DIST);

    MTXTrans(lightMtx, 0, 0, -1000); // Make light "infinite"
    MTXRotDeg(tempMtx, 'y', -LIT_Y_ANGLE);
    MTXConcat(lightMtx, tempMtx, lightMtx);
    MTXRotDeg(tempMtx, 'x', -LIT_X_ANGLE);
    MTXConcat(lightMtx, tempMtx, lightMtx);

    MTXIdentity(objectMtx);
}

// Setup viewport, scissor, copy, etc. based on w/h.
void setupSize(u16 w, u16 h)
{
    GXSetViewport(0, 0, w, h, 0, 1);
    GXSetScissor(0, 0, w, h);
    GXSetDispCopySrc(0, 0, w, h);
    GXSetDispCopyDst(w, h);
    GXSetDispCopyYScale(1);
    GXSetTexCopySrc(0, 0, w, h);
    GXSetTexCopyDst(w, h, LIT_TEX_FMT, FALSE);
}

// Initialization routine common to all my tests.
void commonInit(void)
{
	GXColor black = {0,0,0,0};
    DEMOInit(NULL);
    GXInvalidateTexAll();
    GXSetCopyClear( black, GX_MAX_Z24 );

    GXSetZMode(TRUE, GX_LEQUAL, TRUE);
    GXSetZCompLoc(TRUE);
    GXSetCullMode(GX_CULL_BACK);
    
    setupMatrices();
}

// Set up position/normal/texture matrices and load.
void updateMatrices(u32 id, Mtx obj, Mtx cam)
{
    Mtx vertexMtx, normalMtx, binormalMtx;
    Mtx normalTexMtx, binormalTexMtx, textureMtx, tempMtx;

    // matrix for transforming vertices from object space to eye space.
    MTXConcat(cam, obj, vertexMtx);

    // the normal mtx starts out as the inverse transpose of the vertex mtx
    if (!MTXInverse(vertexMtx, tempMtx)) {ASSERTMSG(0,"Singular matrix!\n");}
    MTXTranspose(tempMtx, normalMtx);

    // matrix for transforming normals from object space to eye space, then
    // offset by 0.5,0.5, scale by NRM_SCALE/2.
    MTXTrans(tempMtx, 0.5f, 0.5f, 0.0f);
    MTXConcat(tempMtx, normalMtx, normalTexMtx);
    MTXScale(tempMtx, NRM_SCALE/2, NRM_SCALE/2, NRM_SCALE/2);
    MTXConcat(normalTexMtx, tempMtx, normalTexMtx);

    // the binormal/tangent mtx starts out as the vertex mtx without translate
    // it should not use the inverse transpose of the vertex mtx
    MTXCopy(vertexMtx, binormalMtx);
    binormalMtx[0][3] = 0.0f;
    binormalMtx[1][3] = 0.0f;
    binormalMtx[2][3] = 0.0f;

    // matrix for transforming binormals from object space to eye space, then
    // scale by NRM_SCALE/2.
    MTXConcat(binormalMtx, tempMtx, binormalTexMtx);

    // matrix for specifying material repetitions
    MTXScale(textureMtx, TRS, TRT, 1.0f);

    GXLoadPosMtxImm(vertexMtx, GX_PNMTX0 + id*3);
    GXLoadNrmMtxImm(normalMtx, GX_PNMTX0 + id*3);
    GXLoadTexMtxImm(textureMtx, GX_TEXMTX0, GX_MTX2x4);
    GXLoadTexMtxImm(normalTexMtx, GX_TEXMTX1 + id*6, GX_MTX2x4);
    GXLoadTexMtxImm(binormalTexMtx, GX_TEXMTX2 + id*6, GX_MTX2x4);
}

// Create an infinite light with diffuse and specular components.
void setLight(GXLightID litID, GXChannelID diffID, GXChannelID specID,
              Mtx obj, Mtx cam)
{
    GXLightObj light;
    Mtx   tempMtx, litMtx;
    Vec   lPos = {0,0,0}, lDir = {0,0,-1};
    GXColor color0 = {0x88,0x88,0x88,0xff};
    GXColor color1 = {0x20,0x20,0x20,0x20};
    GXColor color2 = {0xff,0xff,0xff,0xff};
    GXColor color3 = {0x00,0x00,0x00,0x00};
    GXColor color4 = {0x7f,0x7f,0x7f,0x7f};

    // Calculate light position and direction.
    if (!MTXInverse(obj, tempMtx)) {ASSERTMSG(0,"Singular matrix!\n");}
    MTXConcat(cam, tempMtx, litMtx);
    MTXMultVec(litMtx, &lPos, &lPos);

    if (!MTXInverse(litMtx, tempMtx)) {ASSERTMSG(0,"Singular matrix!\n");}
    MTXTranspose(tempMtx, litMtx);
    MTXMultVec(litMtx, &lDir, &lDir);

    // Setup light object.
    // GXInitLightPos(&light, lPos.x, lPos.y, lPos.z);
    GXInitSpecularDir(&light, lDir.x, lDir.y, lDir.z);
    GXInitLightColor(&light, color0 );
    
    // Light equation 4x^2 - 3. x=N*H.
    GXInitLightAttn(&light, -3.0f, 0.0f, 4.0f, 1.0f, 0.0f, 0.0f);
    GXLoadLightObjImm(&light, litID);

    // Set material and ambient color.
    GXSetChanAmbColor(diffID, color1 );
    GXSetChanMatColor(diffID, color2 );
    GXSetChanAmbColor(specID, color3 );
    GXSetChanMatColor(specID, color4 );
}

// Initial setup for shadowing (setup texture).
void bumpMapSetup(void)
{
    u32 size;
    f32 indMtx[2][3] = { { 0.5F, 0.0F, 0.0F, },   // Indirect "identity" mtx
                         { 0.0F, 0.5F, 0.0F, } }; // (not used)

    // Pre-existing textures are in a tpl file.
    TEXGetPalette(&tpl, TPL_NAME);

    // Light Map Texture (generated)

    // Allocate texture in main memory.
    lightMap = OSAlloc(GXGetTexBufferSize(TW, TH, LIT_TEX_FMT, FALSE, 0));

    // Create texture object.
    GXInitTexObj(&lightMapObj, lightMap, TW, TH, LIT_TEX_FMT,
                 GX_CLAMP, GX_CLAMP, FALSE);

    // Load object into hw. Lightmap will be TEXMAP1.
    GXLoadTexObj(&lightMapObj, GX_TEXMAP1);

    // Material Texture (pre-existing)

    TEXGetGXTexObjFromPalette(tpl, &materialObj, 1);
    GXLoadTexObj(&materialObj, GX_TEXMAP0); // Material will be TEXMAP0

    // Normal Perturbation Texture (pre-existing)

    TEXGetGXTexObjFromPalette(tpl, &nPtrbObj, 0);
#ifndef EMU
    GXLoadTexObj(&nPtrbObj, GX_TEXMAP2); // Normal perturbation will be TEXMAP2
#endif
    // Set up texgen for rendering. The matrices can be changed per-object,
    // but the texgen needn't change.
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
#ifndef EMU
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_BINRM,   GX_TEXMTX2);
    GXSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TANGENT, GX_TEXMTX2);
    GXSetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX1);
#else
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
#endif

    // I will always use position matrix 0 for my geometry.
    GXSetCurrentMtx(GX_PNMTX0);

    // Diffuse is color 0, specular is alpha 0.
    setLight(GX_LIGHT0, GX_COLOR0, GX_ALPHA0, lightMtx, cameraMtx);

    // Set indirect matrix and scale to be "identity" for 64x64 textures.
    GXSetIndTexMtx(GX_ITM_0, indMtx, 0);

    // Alpha component in frame buffer will be written to.
    GXSetAlphaUpdate(TRUE);

    // Make a donut.
    size = sizeof(coord) * (N1+1)*(N2+1);
    donutData = OSAlloc(size);
    createDonut();
}

void bumpMapSetdown(void)
{
    TEXReleasePalette(&tpl);

    OSFree(lightMap);
    OSFree(donutData);
}

// Per-frame setup for actual render.
void renderSetup1(void)
{
    Mtx44 pMtx;

    // Set the rendering size.
    setupSize(W, H);

    // Perspective projection
    // Note that NEGATIVE Y is up for this demo!
    MTXFrustum(pMtx, -1, 1, -1, 1, 1, 15);
    GXSetProjection(pMtx, GX_PERSPECTIVE);

    // Pixel format
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    // We don't need to clear the display again

    // Disable lights
    GXSetChanCtrl(GX_COLOR0, FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
                  GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
                  GX_DF_NONE, GX_AF_NONE);
}

void renderSetup2(void)
{
#ifndef EMU
    // Four tev stages, one Bump stage.
    GXSetNumTevStages(4);
    GXSetNumIndStages(1);

    // Four texture coordinates, no colors.
    GXSetNumTexGens(4);
    GXSetNumChans(0);

    // Indirect Stage 0 -- Sample normal perturbation map
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP2);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);

    // Stage 0 -- Save material texture
    //
    // TEVPREV = TEXC/TEXA
    //
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    
    // Set up the indirect bump calculation for Stages 1-3
    //
    GXSetTevIndBumpST(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITM_0);

    // Stage 1 -- Accumulate binormal perturbation component in Bump
    //
    // TEVPREV = PREV
    //
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1,
                  (GXTexMapID)(GX_TEXMAP1|GX_TEX_DISABLE), GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE1, GX_PASSCLR);

    // Stage 2 -- Accumulate tangent perturbation component in Bump
    //
    // TEVPREV = PREV
    //
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2,
                  (GXTexMapID)(GX_TEXMAP1|GX_TEX_DISABLE), GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE2, GX_PASSCLR);

    // Stage 3 -- Add source normal in Bump. Index lightmap with result of
    //            perturbation. Apply diffuse and specular components.
    //
    // TEVPREVC = PREVC * TEXC + TEXA
    // TEVPREVA = PREVA
    //
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3,
                    GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_TEXA);
    GXSetTevAlphaIn(GX_TEVSTAGE3,
                    GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevColorOp(GX_TEVSTAGE3,
                    GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE3,
                    GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);

#else
    // for emulator
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);

    // Four texture coordinates, no colors.
    GXSetNumTexGens(4);
    GXSetNumChans(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
#endif

    // Vertex packet specification -- Position, normal/tangent/binormal,
    //                                and one pair of texture coordinates.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NBT, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_NRM_NBT, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    sendNormal = TRUE;
    sendBinormals = TRUE;
    sendCoords = TRUE;
}

// Per-frame setup for light map.
void lightMapSetup(void)
{
    Mtx44 pMtx;
    Mtx   idMtx;

    // Set the rendering size.
    setupSize(TW, TH);

    // Orthographic projection
    MTXOrtho(pMtx, -1, 1, -1, 1, 1, 15);
    GXSetProjection(pMtx, GX_ORTHOGRAPHIC);

    // Pixel format
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    // don't need to reclear, since we will overwrite everything we use

    // Enable diffuse and specular lights.
    // COLOR0 = diffuse
    // ALPHA0 = specular
    GXSetChanCtrl(GX_COLOR0, TRUE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
                  GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, TRUE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
                  GX_DF_NONE, GX_AF_SPEC);

    // One tev stage.
    GXSetNumTevStages(1);

    // No textures, one color.
    GXSetNumTexGens(0);
    GXSetNumChans(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // Stage 0 -- Send out rasterized color.
    //
    // TEVPREV = RAS
    //
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    // Vertex packet specification -- Position and normal.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);

    sendNormal = TRUE;
    sendBinormals = FALSE;
    sendCoords = FALSE;

    // Identity matrices for object and matrix.
    MTXIdentity(idMtx);
    updateMatrices(0, idMtx, idMtx);
}

// Prepare for donut drawing.
void updateDonut(Mtx cam, f32 r1, f32 r2, f32 x, f32 y, f32 z)
{
    Mtx donutMtx, tempMtx;

    // update object matrix for this donut.
    MTXRotDeg(tempMtx, 'x', r2);
    MTXConcat(tempMtx, objectMtx, objectMtx);
    MTXRotDeg(tempMtx, 'y', r1);
    MTXConcat(tempMtx, objectMtx, objectMtx);
    MTXTrans(tempMtx, x, y, z);
    MTXConcat(tempMtx, objectMtx, donutMtx);

    // Create all other matrices derived from this object matrix.
    updateMatrices(0, donutMtx, cam);
}

// Draw a donut.
void drawDonut()
{
    u32 i,j;

    for (i=0; i<N1; i++)
    {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16) ((N2+1)*2) );

        for (j=0; j<=N2; j++)
        {
            sendVertex(&donutData[(i+1)*(N2+1) + j]);
            sendVertex(&donutData[i*(N2+1) + j]);
        }

        GXEnd();
    }
}

// Draw a tesselated square, with normals that make it look like a hemisphere.
void drawHemisphere()
{
    u32 i, j;
    coord c;

    for (i=0; i<ST; i++)
    {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16) (TT*2 + 2));

        for (j=0; j<=TT; j++)
        {
            c.x = ((f32) 2.0f*i/ST) - 1.0f;
            c.y = ((f32) 2.0f*j/TT) - 1.0f;
            c.z = -2.0f;
            c.nx = c.x / NRM_SCALE;
            c.ny = c.y / NRM_SCALE;
            c.nz = c.x*c.x + c.y*c.y;
            c.nz = c.nz < 1 ? sqrtf(1 - c.nz) : 0;
            sendVertex(&c);

            c.x = ((f32) 2.0f*(i+1)/ST) - 1.0f;
            c.nx = c.x / NRM_SCALE;
            c.nz = c.x*c.x + c.y*c.y;
            c.nz = c.nz < 1 ? sqrtf(1 - c.nz) : 0;
            sendVertex(&c);
        }

        GXEnd();
    }
}

// Draw the full scene from a specific camera.
void drawScene(Mtx cam, f32 r1, f32 r2)
{
    // Draw one donut, centered at 0,0,0, rotating at normal speed.
    updateDonut(cam, r1, r2, 0.0f, 0.0f, 0.0f);
    drawDonut();
}

//  Show the generated light maps in the corners of the screen
//  Also show bump textures
void showMaps()
{
    Mtx   idMtx;
    coord c;

    // One tev stage.
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);

    // One texture, no color.
    GXSetNumTexGens(1);
    GXSetNumChans(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);

    // Stage 0 -- Send out texture color.
    //
    // TEVPREV = TEXC
    //
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevAlphaIn(GX_TEVSTAGE0,GX_CA_ZERO,GX_CA_ZERO,GX_CA_ZERO,GX_CA_ONE);

    // Vertex packet specification -- Position and normal.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    sendNormal = FALSE;
    sendBinormals = FALSE;
    sendCoords = TRUE;

    // Identity matrices for object and matrix.
    MTXIdentity(idMtx);
    GXLoadPosMtxImm(idMtx, GX_PNMTX0);
    GXLoadTexMtxImm(idMtx, GX_TEXMTX0, GX_MTX2x4);

    // Show diffuse light map
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    c.x = -1.0f;
    c.y = -1.0f;
    c.z = -1.0f;
    c.s =  0.0f;
    c.t =  0.0f;
    sendVertex(&c);
    c.x += (f32) TW/W*2;
    c.s  = 1.0f;
    sendVertex(&c);
    c.y += (f32) TH/H*2;
    c.t  = 1.0f;
    sendVertex(&c);
    c.x -= (f32) TW/W*2;
    c.s  = 0.0f;
    sendVertex(&c);
    GXEnd();

    // Now show alpha (specular) map
    GXSetTevColorIn(GX_TEVSTAGE0,GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO,GX_CC_TEXA);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    c.x =  1.0f;
    c.y = -1.0f;
    c.z = -1.0f;
    c.s =  1.0f;
    c.t =  0.0f;
    sendVertex(&c);
    c.y += (f32) TH/H*2;
    c.t  = 1.0f;
    sendVertex(&c);
    c.x -= (f32) TW/W*2;
    c.s  = 0.0f;
    sendVertex(&c);
    c.y -= (f32) TH/H*2;
    c.t  = 0.0f;
    sendVertex(&c);
    GXEnd();

    // Now show the S component of the bump map
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR_NULL);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    c.x = -1.0f;
    c.y =  1.0f;
    c.z = -1.0f;
    c.s =  0.0f;
    c.t =  1.0f;
    sendVertex(&c);
    c.y -= (f32) IH/H;
    c.t  = 0.0f;
    sendVertex(&c);
    c.x += (f32) IW/W;
    c.s  = 1.0f;
    sendVertex(&c);
    c.y += (f32) IH/H;
    c.t  = 1.0f;
    sendVertex(&c);
    GXEnd();

    // Now show the T component of the bump map
    GXSetTevColorIn(GX_TEVSTAGE0,GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO,GX_CC_TEXC);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    c.x =  1.0f;
    c.y =  1.0f;
    c.z = -1.0f;
    c.s =  1.0f;
    c.t =  1.0f;
    sendVertex(&c);
    c.x -= (f32) IW/W;
    c.s  = 0.0f;
    sendVertex(&c);
    c.y -= (f32) IH/H;
    c.t  = 0.0f;
    sendVertex(&c);
    c.x += (f32) IW/W;
    c.s  = 1.0f;
    sendVertex(&c);
    GXEnd();
}

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main(void)
{
    f32 rot1=0.0f;
    f32 rot2=0.0f;

    commonInit();  // Same init for all tests.

    bumpMapSetup(); // Setup texture objects and all other static things.

    // This is only done once in this test since the camera and light never
    // change direction with respect to one another. If they ever do, the
    // lightmap must be regenerated.
    lightMapSetup();  // Setup for lightmap render.
    drawHemisphere(); // Render hemisphere lightmap

    GXCopyTex(lightMap, FALSE); // Copy to texture.
    GXPixModeSync();

    PrintIntro();

    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {
        DEMOPadRead();

        rot1 = DEMOPadGetStickX(0)/10;
        rot2 = DEMOPadGetStickY(0)/10;
        DEMOBeforeRender();

        renderSetup1();   // Setup for actual render, part 1.

        showMaps();       // Show the lightmaps used

        renderSetup2();   // Setup for actual render, part 2.

        drawScene(cameraMtx, rot1, rot2);

        DEMODoneRender();
    }

    bumpMapSetdown(); // Free memory and such.

    OSHalt("End of demo");
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
    OSReport("ind-bump-st: show indirect texture bump mapping\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("  Stick X/Y    : rotate the donut\n");
    OSReport("************************************************\n\n");
}

