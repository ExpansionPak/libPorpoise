/*---------------------------------------------------------------------------*
  Project:  Dolphin GD demo
  File:     gd-tev-gc.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-tev-gc.c $
    
    3     10/19/02 6:53p Hirose
    Changed location of data file.
    
    2     10/13/01 2:29a Hirose
    Fixes due to GDSetTexCoordGen API change.
    
    1     10/04/01 2:48p Hirose
    Initial check in.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   gd-tev
     Displaylist demo with multitexture shader commands
     [Main source codr for GAMECUBE exectable]
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

#include "gd-tev.h"

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI    3.14159265F
#define PI_2  6.28318531F

#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
   Structure definitions
 *---------------------------------------------------------------------------*/
// for camera
typedef struct
{
    Vec     location;
    Vec     up;
    Vec     target;
    f32     left;
    f32     top;
    f32     znear;
    f32     zfar;
} CameraConfig;

typedef struct
{
    CameraConfig    cfg; 
    Mtx             view;
    Mtx44           proj;
} MyCameraObj;

// for lighting
typedef struct
{
    GXLightObj      lobj;
    s32             theta;
    s32             phi;
} MyLightCtrlObj;

typedef struct
{
    void*       dlPtr;
    u32         dlSize;
    u32*        plPtr;
    u32         plSize;
} MyDLObj;

// for model object
typedef struct
{
    s16*        posArray;
    s16*        nrmArray;
    s16*        tcdArray;
    MyDLObj     dl;
} MyModelObj;

// for entire scene control
typedef struct
{
    MyCameraObj     cam;
    MyLightCtrlObj  lightCtrl;
    Mtx             modelRot;
    f32             modelScale;
    u16             viewWidth;
    u16             viewHeight;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main                ( void );

static void DrawInit            ( MySceneCtrlObj* sc );
static void DrawTick            ( MySceneCtrlObj* sc );
static void AnimTick            ( MySceneCtrlObj* sc );
static void SetCamera           ( MyCameraObj* cam );
static void SetLight            ( MyLightCtrlObj* le, Mtx view );
static void PrintIntro          ( void );

static void PrepareDL           ( void );
static void CreateModelVtxArray ( void );
static void PatchShaderDLTCScale( MyDLObj* mdl );


/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 900.0F }, // location
    { 0.0F, 1.0F,   0.0F }, // up
    { 0.0F, 0.0F,   0.0F }, // tatget
    -320.0F, // left
    240.0F,  // top
    400.0F,  // near
    2000.0F  // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;          // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;    // texture palette

static GXTexObj         MyTexObjs[NUM_TEXTURES];
static MyDLObj          ShaderDLs[NUM_SHADERDLS];
static MyModelObj       ModelObj;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);  // Init the OS, game pad, graphics and video.
    
    DrawInit(&SceneCtrl);       // Initialize vertex formats and scene parameters.

    PrintIntro();  // Print demo directions
         
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);    // Draw the model.
        DEMODoneRender();
        DEMOPadRead();           // Update pad status.
        AnimTick(&SceneCtrl);    // Update animation.
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format and
                    sets up default scene parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{ 
    GXRenderModeObj* rmode;
    u32  i, nd;

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, QUANTIZE_SHIFT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_NRM_NBT, GX_S16, QUANTIZE_SHIFT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, QUANTIZE_SHIFT);

    // Disable all automatic texcoord scaling settings that
    // cannot work well with GD display lists.
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD2, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD3, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD4, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD5, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD6, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD7, GX_ENABLE, 1, 1);


    // Get screen information
    rmode = DEMOGetRenderModeObj();
    sc->viewWidth  = rmode->fbWidth;    // Screen Width
    sc->viewHeight = rmode->efbHeight;  // Screen Height


    // Load TPL file
    TEXGetPalette(&MyTplObj, "gxTests/tev-03.tpl");
    nd = MyTplObj->numDescriptors;
    // get texture maps from a texture palette
    for ( i = 0 ; i < NUM_TEXTURES ; ++i )
    {
        TEXGetGXTexObjFromPalette(MyTplObj, &MyTexObjs[i], i % nd);
    }


    // Prepare model display list and shader display lists
    PrepareDL();
    CreateModelVtxArray();


    // Default scene parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // light parameters
    sc->lightCtrl.theta = 30;
    sc->lightCtrl.phi   = 0;

    // model control
    MTXRotDeg(sc->modelRot, 'x', -30);
    sc->modelScale = 750.0F;

}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
// Viewport setup
inline void SetWindow( u32 num, u32 wd, u32 ht )
{
    u32 xorg, yorg;
    xorg = (num&1) * wd;
    yorg = ((num>>1)&1) * ht;
    GXSetScissor(xorg, yorg, wd, ht);
    GXSetViewport(xorg, yorg, wd, ht, 0.0F, 1.0F);
}

static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mn = { { 0.5F, 0.0F, 0.0F, -0.5F },   // Fixed matrix
                { 0.0F, 0.5F, 0.0F, -0.5F },   // to regularize normal
                { 0.0F, 0.0F, 0.0F,  1.0F } }; // texgen
    Mtx  ms;  // Scaling matrix.
    Mtx  mr;  // Rotation matrix.
    Mtx  mv;  // Modelview matrix.
    u32  i, vw, vh;

    // modelview matrix
    //
    // Binormal and tangent will not be normalized.
    // So we should consider scale factor of matrix for normal transformation matrix.
    MTXScale(ms, sc->modelScale, sc->modelScale, sc->modelScale);
    MTXConcat(sc->cam.view, sc->modelRot, mr);
    MTXConcat(mr, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    MTXScale(ms, 0.03F, 0.03F, 0.03F);
    MTXConcat(mr, ms, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    // texgen matrices
    GXLoadTexMtxImm(sc->modelRot, GX_TEXMTX2, GX_MTX3x4);  // for reflection map
    GXLoadTexMtxImm(mn, GX_PTTEXMTX0, GX_MTX3x4);           //
    MTXScale(ms, 6.0F, 6.0F, 0.0F);
    GXLoadTexMtxImm(ms, GX_TEXMTX0, GX_MTX2x4); // for bump map
    MTXScale(ms, 3.5F, 5.0F, 0.0F);
    GXLoadTexMtxImm(ms, GX_TEXMTX1, GX_MTX2x4); // for base map
    MTXScale(ms, 1.0F, 2.0F, 0.0F);
    GXLoadTexMtxImm(ms, GX_TEXMTX3, GX_MTX2x4); // for gloss map
    
    // Lighting
    SetLight(&sc->lightCtrl, sc->cam.view);

    // Texmaps
    for ( i = 0 ; i < NUM_TEXTURES ; ++i )
    {
        GXLoadTexObj(&MyTexObjs[i], (GXTexMapID)i);
    }

    // Viewport size
    vw = (u32)(sc->viewWidth / 2);
    vh = (u32)(sc->viewHeight / 2);
    
    // Call display list (shader + model)
    for ( i = 0 ; i < 4 ; ++i )
    {
        SetWindow(i, vw, vh);
        GXCallDisplayList(ShaderDLs[i].dlPtr, ShaderDLs[i].dlSize);
        GXCallDisplayList(ModelObj.dl.dlPtr, ModelObj.dl.dlSize);
    }
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    Mtx  mrx, mry;

    // Model Rotation Calculation
    MTXRotDeg(mry, 'x', -(DEMOPadGetStickY(0) / 24));
    MTXRotDeg(mrx, 'y',  (DEMOPadGetStickX(0) / 24));
    MTXConcat(mry, sc->modelRot, sc->modelRot);
    MTXConcat(mrx, sc->modelRot, sc->modelRot);

    // Light Position Calculation
    sc->lightCtrl.theta += (DEMOPadGetSubStickX(0) / 24);
    sc->lightCtrl.theta %= 360;
    sc->lightCtrl.phi += (DEMOPadGetSubStickY(0) / 24);
    Clamp(sc->lightCtrl.phi, -90, 90);
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    Sets view matrix and loads projection matrix into hardware
                    
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

    MTXFrustum(
        cam->proj,
        cam->cfg.top,
        - (cam->cfg.top),
        cam->cfg.left,
        - (cam->cfg.left),
        cam->cfg.znear,
        cam->cfg.zfar );
    GXSetProjection(cam->proj, GX_PERSPECTIVE);
}

/*---------------------------------------------------------------------------*
    Name:           SetLight
    
    Description:    Sets up lights and lighting channel parameters
                    
    Arguments:      le    : pointer to a MyLightCtrlObj structure
                    view  : view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetLight( MyLightCtrlObj* le, Mtx view )
{
    GXColor litc0 = { 0xC0, 0xC0, 0xC0, 0xC0 };
    GXColor ambc0 = { 0x40, 0x40, 0x40, 0x40 };
    GXColor matc0 = { DIFFUSE_BASE, DIFFUSE_BASE, DIFFUSE_BASE, DIFFUSE_BASE };
    GXColor litc1 = { 0xE0, 0xE0, 0xE0, 0xE0 };
    GXColor ambc1 = { 0x00, 0x00, 0x00, 0x00 };
    GXColor matc1 = { SPECULAR_BASE, SPECULAR_BASE, SPECULAR_BASE, SPECULAR_BASE };
    GXLightObj lo0, lo1;
    Vec        lpos, ldir;
    f32        theta, phi;

    // Light position/direction
    theta = (f32)le->theta * PI / 180.0F;
    phi   = (f32)le->phi   * PI / 180.0F;
    // Direction of specular light
    ldir.x = - cosf(phi) * sinf(theta);
    ldir.y = - sinf(phi);
    ldir.z = - cosf(phi) * cosf(theta);
    // Position of diffuse light
    VECScale(&ldir, &lpos, -1000.0F);

    // Set a diffuse light
    MTXMultVec(view, &lpos, &lpos);
    GXInitLightPosv(&lo0, &lpos);
    GXInitLightColor(&lo0, litc0);
    GXLoadLightObjImm(&lo0, GX_LIGHT0);

    // Set a specular light
    MTXMultVecSR(view, &ldir, &ldir);
    GXInitSpecularDirv(&lo1, &ldir);
    GXInitLightShininess(&lo1, 48.0F);
    GXInitLightColor(&lo1, litc1);
    GXLoadLightObjImm(&lo1, GX_LIGHT1);

    // Lighting channel
    GXSetChanCtrl(GX_COLOR0, GX_ENABLE,         // Diffuse
                  GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1, GX_ENABLE,         // Specular
                  GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT1, GX_DF_NONE, GX_AF_SPEC);
    GXSetChanCtrl(GX_ALPHA0, GX_DISABLE,        // Unused
                  GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA1, GX_DISABLE,        // Unused
                  GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

    // set up ambient/material color
    GXSetChanAmbColor(GX_COLOR0A0, ambc0);
    GXSetChanAmbColor(GX_COLOR1A1, ambc1);
    GXSetChanMatColor(GX_COLOR0A0, matc0);
    GXSetChanMatColor(GX_COLOR1A1, matc1);
}

/*---------------------------------------------------------------------------*
    Name:           PrepareDL
    
    Description:    [create mode] Allocate memory for display list and call
                    the function CreateModelDL to create the display list.
                    [load mode] Load GDL file from the disc.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
#ifndef LOAD_DL_FROM_FILE
static void(*CreateShdDLFunc[NUM_SHADERDLS])() =
{
    CreateShader0DL, CreateShader1DL, CreateShader2DL, CreateShader3DL
};
#endif

static void PrepareDL( void )
{
    u32     i;

#ifdef LOAD_DL_FROM_FILE
    //---------------------------------------------------------
    // File mode : Read pre-generated GDL file from file
    //---------------------------------------------------------
    s32         err;
    GDGList*    dlArray;
    GDGList*    plArray;
    u32         nDls, nPls;
    
    err = GDReadDLFile("gddemo/gdTev.gdl", &nDls, &nPls, &dlArray, &plArray);
    
    ASSERTMSG( err == 0, "GD file read error.\n" );
    
    // Check number of lists.
    ASSERTMSG( ( nDls == NUM_DLS && nPls == NUM_PLS ),
               "This data doesn't match requirement of this demo.\n" );


    OSReport("\nGD file read completed.\n");
    
    // Obtain shader display lists.
    for ( i = 0 ; i < NUM_SHADERDLS ; ++i )
    {
        ShaderDLs[i].dlPtr  = dlArray[i].ptr;
        ShaderDLs[i].dlSize = dlArray[i].byteLength;
        ShaderDLs[i].plPtr  = plArray[i].ptr;
        ShaderDLs[i].plSize = plArray[i].byteLength;
    }

    // Obtain model display list.
    ModelObj.dl.dlPtr  = dlArray[i].ptr;
    ModelObj.dl.dlSize = dlArray[i].byteLength;
    ModelObj.dl.plPtr  = NULL;
    ModelObj.dl.plSize = 0;

#else
    //---------------------------------------------------------
    // Create mode : Create display list in this application
    //---------------------------------------------------------
    
    // Create shader display lists
    for ( i = 0 ; i < NUM_SHADERDLS ; ++i )
    {
        ShaderDLs[i].dlPtr = OSAlloc(SDL_SIZE_MAX);
        ShaderDLs[i].plPtr = OSAlloc(PL_SIZE_MAX);
        (*CreateShdDLFunc[i])(ShaderDLs[i].dlPtr, &ShaderDLs[i].dlSize,
                              ShaderDLs[i].plPtr, &ShaderDLs[i].plSize);
        OSReport("ShaderDL %d size = %d\n", i, ShaderDLs[i].dlSize);
    }    

    // Create model display list
    ModelObj.dl.dlPtr  = OSAlloc(MDL_SIZE_MAX);
    ModelObj.dl.plPtr  = NULL;
    ModelObj.dl.plSize = 0;
    CreateModelDL(ModelObj.dl.dlPtr, &ModelObj.dl.dlSize);
    OSReport("ModelDL size = %d\n", ModelObj.dl.dlSize);
#endif


    // Patch texcoord scale data of all shader display lists
    for ( i = 0 ; i < NUM_SHADERDLS ; ++i )
    {
        PatchShaderDLTCScale(&ShaderDLs[i]);
    }
}

/*---------------------------------------------------------------------------*
    Name:           PatchShaderDLTCScale
    
    Description:    Patch given shader display list and overwrite texcoord
                    scale informations that were not determinable during
                    creation time.
                    
    Arguments:      mdl : MyDLObj which holds a display list to be patched. 
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PatchShaderDLTCScale( MyDLObj* mdl )
{
    GDLObj  gdlObj;
    u32     plIndex = 0;
    
    // Set up a GDL object and make it current.
    GDInitGDLObj(&gdlObj, mdl->dlPtr, mdl->dlSize);
    GDSetCurrent(&gdlObj);

    while( plIndex < mdl->plSize / sizeof(u32) )
    {
        u16             scaleS, scaleT;
        GXBool          biasS, biasT;
        u32             tmid;
        GXTexCoordID    tcid;
    
        // Patch list contains following entries per patch:
        //   [0] : patch location offset
        //   [1] : target texcoord ID
        //   [2] : texture which scales the texcoord
    
        // Move pointer to the patch location.
        GDSetCurrOffset(mdl->plPtr[plIndex++]);
        
        // Get texcoord target
        tcid = (GXTexCoordID)(mdl->plPtr[plIndex++]);
        
        // Get texture size which scales the texcoord
        tmid = mdl->plPtr[plIndex++];
        scaleS = GXGetTexObjWidth(&MyTexObjs[tmid]);
        scaleT = GXGetTexObjHeight(&MyTexObjs[tmid]);
        // Bias should be turned on if repeat mode
        biasS  = (GXGetTexObjWrapS(&MyTexObjs[tmid]) == GX_REPEAT)
               ? GX_ENABLE : GX_DISABLE;
        biasT  = (GXGetTexObjWrapT(&MyTexObjs[tmid]) == GX_REPEAT)
               ? GX_ENABLE : GX_DISABLE;
        
        // Patch command
        GDSetTexCoordScale2(tcid, scaleS, biasS, GX_DISABLE,
                                  scaleT, biasT, GX_DISABLE);
    }

    GDFlushCurrToMem();
    
    GDSetCurrent(NULL);
}

/*---------------------------------------------------------------------------*
    Name:           CreateModelVtxArray
    
    Description:    Create indexed data array for the model
                    
    Arguments:      mo : pointer to model object
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CreateModelVtxArray( void )
{
    MyModelObj* mo = &ModelObj;
    u32  size_p, size_n, size_t;
    u32  cnt_p, cnt_n, cnt_t;
    s32  i, j;
    f32  theta, phi;
    f32  ct, cp, dzdt, dzdp, nt, np, nn;

    size_p = MODEL_MESHX * MODEL_MESHY * 3;
    size_n = MODEL_MESHX * MODEL_MESHY * 9;
    size_t = MODEL_MESHX * MODEL_MESHY * 2;

    mo->posArray = (s16*)OSAlloc(size_p * sizeof(s16));
    mo->nrmArray = (s16*)OSAlloc(size_n * sizeof(s16));
    mo->tcdArray = (s16*)OSAlloc(size_t * sizeof(s16));

    // make array data
    cnt_p = cnt_n = cnt_t = 0;
    for ( i = 0 ; i < MODEL_MESHY ; ++i )
    {
        for ( j = 0 ; j < MODEL_MESHX ; ++j )
        {
            theta = (f32)j * PI_2 / MODEL_MESHX;
            phi   = (f32)i * PI_2 / MODEL_MESHX;
            
            ct = cosf(theta);
            cp = cosf(phi);
            
            dzdt = MODEL_ZSCALE * PI * -sinf(theta) * cp;
            dzdp = MODEL_ZSCALE * PI * -sinf(phi)   * ct;
            
            nt = 1.0F / sqrtf( 1.0F + dzdt*dzdt );
            np = 1.0F / sqrtf( 1.0F + dzdp*dzdp );
            nn = nt * np;
            
            // Position
            mo->posArray[cnt_p++] = (s16)(((f32)j*2 / MODEL_MESHX - 1.0F) * QSCALE);
            mo->posArray[cnt_p++] = (s16)(((f32)i*2 / MODEL_MESHY - 1.0F) * QSCALE);
            mo->posArray[cnt_p++] = (s16)(MODEL_ZSCALE * ct * cp * QSCALE);
            
            // Normal
            mo->nrmArray[cnt_n++] = (s16)(nn * -dzdt * QSCALE);
            mo->nrmArray[cnt_n++] = (s16)(nn * -dzdp * QSCALE);
            mo->nrmArray[cnt_n++] = (s16)(nn * QSCALE);
            
            // Binormal
            mo->nrmArray[cnt_n++] = (s16)(nt * QSCALE);
            mo->nrmArray[cnt_n++] = (s16)0;
            mo->nrmArray[cnt_n++] = (s16)(nt * dzdt * QSCALE);

            // Tangent
            mo->nrmArray[cnt_n++] = (s16)0;
            mo->nrmArray[cnt_n++] = (s16)(np * QSCALE);
            mo->nrmArray[cnt_n++] = (s16)(np * dzdp * QSCALE);

            // Texcoord
            mo->tcdArray[cnt_t++] = (s16)(j * QSCALE / MODEL_MESHX);
            mo->tcdArray[cnt_t++] = (s16)(i * QSCALE / MODEL_MESHY);
        }
    }

    // make sure data is written to main memory
    DCFlushRange(mo->posArray, size_p * sizeof(s16));
    DCFlushRange(mo->nrmArray, size_n * sizeof(s16));
    DCFlushRange(mo->tcdArray, size_t * sizeof(s16));

    // set up array pointers
    GXSetArray(GX_VA_POS,  mo->posArray, sizeof(s16) * 3);
    GXSetArray(GX_VA_NRM,  mo->nrmArray, sizeof(s16) * 9);
    GXSetArray(GX_VA_TEX0, mo->tcdArray, sizeof(s16) * 2);
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
    OSReport("    GD Multitexturing (TEV) demo\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
    OSReport("Main Stick   : Rotate the model\n");
    OSReport("Sub  Stick   : Move Light Position\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
