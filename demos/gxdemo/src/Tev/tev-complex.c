/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tev-complex.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Tev/tev-complex.c $
    
    12    6/12/01 8:37p Hirose
    got rid of tev-emu stuff because it is no longer necessary.
    
    11    5/13/01 2:48p Hirose
    removed obsolete flags
    
    10    10/30/00 3:41p Hirose
    removed GXSetTevClampMode call / flag change MAC -> EMU.
    
    9     10/17/00 6:31p Hirose
    changed to use indexed NBT
    
    8     5/24/00 6:08p Hirose
    fixed parameter and added comments
    
    7     5/12/00 6:08p Hirose
    fix due to GXInitLight*v definition change
    
    6     3/24/00 3:45p Hirose
    changed to use DEMOPad library
    
    5     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    4     3/20/00 7:51p Hirose
    avoided to use indexed NBT
    
    3     3/17/00 2:16p Hirose
    
    2     3/16/00 11:52p Hirose
    parameter changes
    
    1     3/15/00 5:24p Hirose
    initial version
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   tev-complex
       complex multitexture example
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define MAX_TEXMAPS    8
#define MAX_TEVSTAGES  8
#define MAX_TEXCOORDS  8
#define MAX_CHANNELS   2

#define MODEL_DIV0     32
#define MODEL_DIV1     16

#define BUMP_SCALE     0x50
#define REFLEX_SCALE   0x70
#define DIFFUSE_BASE   0xB0
#define SPECULAR_BASE  0x90

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
} MyCameraObj;

// for lighting
typedef struct
{
    GXLightObj  lobj;
    s32         theta;
    s32         phi;
} MyLightCtrlObj;

// for TEV stage control
typedef struct
{
    GXTexMapID     texMap;
    GXTexCoordID   texCoord;
    GXChannelID    channel;
    GXTevColorArg  colArgs[4];
    GXTevOp        colOp;
    GXTevBias      colBias;
    GXTevScale     colScale;
    GXBool         colClamp;
    GXTevRegID     colOut;
    GXTevAlphaArg  alpArgs[4];
    GXTevOp        alpOp;
    GXTevBias      alpBias;
    GXTevScale     alpScale;
    GXBool         alpClamp;
    GXTevRegID     alpOut;
} MyTevStgObj;

// for Texture coord generation
typedef struct
{
    GXTexGenType   func;
    GXTexGenSrc    src;
    GXTexMtx       mt;
} MyTexGenObj;

// for multi-texture shading configuration
typedef struct
{
    u8             numTevStages;
    u8             numTexMaps;
    u8             numTexCoords;
    u8             numChannels;
    GXTexObj*      texMapArray;
    MyTexGenObj*   texGenArray;
    MyTevStgObj*   tevStgArray;
    GXColor        tevRegColor[3];
} MyMTConfig;

// for model
typedef struct
{
    Mtx            rot;
    f32            scale;
    u32            numDiv0;
    u32            numDiv1;
    s16*           posArray;
    s16*           nrmArray;
    s16*           tcdArray;
} MyModelObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    MyLightCtrlObj lightCtrl;
    MyMTConfig*    mtConfig;
    MyModelObj     model;
    u16            viewWidth;
    u16            viewHeight;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main                ( void );
static void DrawInit            ( MySceneCtrlObj* sc );
static void DrawTick            ( MySceneCtrlObj* sc );
static void AnimTick            ( MySceneCtrlObj* sc );
static void DrawModel           ( MyModelObj* mo );
static void CreateModel         ( MyModelObj* mo );
static void SetCamera           ( MyCameraObj* cam );
static void SetLight            ( MyLightCtrlObj* le, Mtx view );
static void SetMultiTex         ( MyMTConfig* mtc );
static void PrintIntro          ( void );

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
   Multi-texture configuration data
 *---------------------------------------------------------------------------*/
static MyTexGenObj TexGenConfig0[] =
{
   // TEXCOORD0
   { GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0 },
   // TEXCOORD1
   { GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1 },
   // TEXCOORD2
   { GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2 },
   // TEXCOORD3 (for reflectioin map)
   { GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX9 },
   // TEXCOORD4 (for emboss bump)
   { GX_TG_BUMP0, GX_TG_TEXCOORD1, GX_IDENTITY }
};

static MyTevStgObj TevStgConfig0[] =
{
    // ------------------------ TEVSTAGE 0 ------------------------
    // REGPREV(C) = diffuse lit color(COLOR0A0)
    //            + bump texture(TEXMAP0/TEXCOORD1) * bump scale(REG0(A))
    {
        GX_TEXMAP0,      // texture map ID (emboss bump texture)
        GX_TEXCOORD1,    // texcoord ID
        GX_COLOR0A0,     // channel ID
        // Color operation
        { GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_RASC },
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        // Alpha operation
        { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO },
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV
    },
    // ------------------------ TEVSTAGE 1 ------------------------
    // REGPREV(C) = REGPREV(C)
    //            - bump texture(TEXMAP0/TEXCOORD4) * bump scale(REG0(A))
    {
        GX_TEXMAP0,      // texture map ID (emboss bump texture)
        GX_TEXCOORD4,    // texcoord ID (perturbed coordinate)
        GX_COLOR0A0,     // channel ID
        // Color operation
        { GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_CPREV },
        GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV,
        // Alpha operation
        { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO },
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV
    },
    // ------------------------ TEVSTAGE 2 ------------------------
    // REG2(C) = REGPREV(C) * base texture(TEXMAP1/TEXCOORD2)
    {
        GX_TEXMAP1,      // texture map ID
        GX_TEXCOORD2,    // texcoord ID
        GX_COLOR0A0,     // channel ID
        // Color operation
        { GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO },
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVREG2,
        // Alpha operation
        { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO },
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVREG2
    },
    // ------------------------ TEVSTAGE 3 ------------------------
    // REGPREV(C) = specular lit color(COLOR1A1)
    //            + reflection map(TEXMAP2/TEXCOORD3) * reflection scale(REG1(A))
    {
        GX_TEXMAP2,      // texture map ID
        GX_TEXCOORD3,    // texcoord ID
        GX_COLOR1A1,     // channel ID
        // Color operation
        { GX_CC_ZERO, GX_CC_A1, GX_CC_TEXC, GX_CC_RASC },
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV,
        // Alpha operation
        { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO },
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV
    },
    // ------------------------ TEVSTAGE 4 ------------------------
    // REGPREV(C) = diffusion map texture(TEXMAP3/TEXCOORD0) * REG2(C)
    //            + (1 - diffusion map texture) * REGPREV(C)
    {
        GX_TEXMAP3,      // texture map ID
        GX_TEXCOORD0,    // texcoord ID
        GX_COLOR0A0,     // channel ID
        // Color operation
        { GX_CC_CPREV, GX_CC_C2, GX_CC_TEXC, GX_CC_ZERO },
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV,
        // Alpha operation
        { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO },
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV
    }
};

static MyMTConfig MultiTexConfig0 =
{
    5,    // number of TEV stages
    4,    // number of texture maps
    5,    // number of texture coordinate generators
    2,    // number of channels
    NULL,          // texmap array (set in run-time)
    TexGenConfig0, // texgen array
    TevStgConfig0, // tevstage array
    {
        { 0, 0, 0, BUMP_SCALE    }, // TEVREG0 preset value
        { 0, 0, 0, REFLEX_SCALE  }, // TEVREG1 preset value
        { 0, 0, 0, 0 }              // TEVREG2 preset value
    }
};

/*---------------------------------------------------------------------------*
   Other data tables
 *---------------------------------------------------------------------------*/
static GXTevStageID StageIDTbl[MAX_TEVSTAGES] =
{
    GX_TEVSTAGE0, GX_TEVSTAGE1, GX_TEVSTAGE2, GX_TEVSTAGE3,
    GX_TEVSTAGE4, GX_TEVSTAGE5, GX_TEVSTAGE6, GX_TEVSTAGE7
};

static GXTexCoordID CoordIDTbl[MAX_TEXCOORDS] =
{
    GX_TEXCOORD0, GX_TEXCOORD1, GX_TEXCOORD2, GX_TEXCOORD3,
    GX_TEXCOORD4, GX_TEXCOORD5, GX_TEXCOORD6, GX_TEXCOORD7
};

static GXTexMtx TexMtxIDTbl[MAX_TEXCOORDS] =
{
    GX_TEXMTX0, GX_TEXMTX1, GX_TEXMTX2, GX_TEXMTX3,
    GX_TEXMTX4, GX_TEXMTX5, GX_TEXMTX6, GX_TEXMTX7
};

static GXTexMapID MapIDTbl[MAX_TEXMAPS] =
{
    GX_TEXMAP0, GX_TEXMAP1, GX_TEXMAP2, GX_TEXMAP3,
    GX_TEXMAP4, GX_TEXMAP5, GX_TEXMAP6, GX_TEXMAP7
};

static GXChannelID ChannelIDTbl[MAX_CHANNELS] =
{
    GX_COLOR0A0, GX_COLOR1A1
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;          // scene control parameters
static TEXPalettePtr   MyTplObj = NULL;    // texture palette

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);    // Init the OS, game pad, graphics and video.
    
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
    u32  i, nt, nd;

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 13);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_NRM_NBT, GX_S16, 13);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 13);

    // Load TPL file
    TEXGetPalette(&MyTplObj, "gxTests/tev-03.tpl");
    nd = MyTplObj->numDescriptors;

    // Get screen information
    rmode = DEMOGetRenderModeObj();
    sc->viewWidth  = rmode->fbWidth;    // Screen Width
    sc->viewHeight = rmode->efbHeight;  // Screen Height


    // Default scene parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // light parameters
    sc->lightCtrl.theta = 30;
    sc->lightCtrl.phi   = 0;

    // multi-textureing configuration
    sc->mtConfig = &MultiTexConfig0;

    // get texture maps from a texture palette
    nt = sc->mtConfig->numTexMaps;
    sc->mtConfig->texMapArray = (GXTexObj*)OSAlloc(nt * sizeof(GXTexObj));
    for ( i = 0 ; i < nt ; ++i )
    {
        TEXGetGXTexObjFromPalette(
            MyTplObj,
            &sc->mtConfig->texMapArray[i],
            i % nd );
    }

    // model
    MTXRotDeg(sc->model.rot, 'x', 30);
    sc->model.scale    = 300.0F;
    sc->model.numDiv0  = MODEL_DIV0;
    sc->model.numDiv1  = MODEL_DIV1;
    sc->model.posArray = NULL;
    sc->model.nrmArray = NULL;
    sc->model.tcdArray = NULL;
    CreateModel(&sc->model);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mn = { { 0.5F, 0.0F, 0.0F, -0.5F },   // Fixed matrix
                { 0.0F, 0.5F, 0.0F, -0.5F },   // to regularize normal
                { 0.0F, 0.0F, 0.5F, -0.5F } }; // texgen
    Mtx  ms;  // Scaling matrix.
    Mtx  mr;  // Rotation matrix.
    Mtx  mv;  // Modelview matrix.

    // viewport
    GXSetViewport(0, 0, sc->viewWidth, sc->viewHeight, 0.0F, 1.0F);

    // modelview matrix
    //
    // Binormal and tangent will not be normalized.
    // So we should consider scale factor of matrix for normal transformation matrix.
    MTXScale(ms, sc->model.scale, sc->model.scale, sc->model.scale);
    MTXConcat(sc->cam.view, sc->model.rot, mr);
    MTXConcat(mr, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    MTXScale(ms, 0.03F, 0.03F, 0.03F);
    MTXConcat(mr, ms, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    // texgen matrix
    MTXConcat(mn, sc->model.rot, mv);
    GXLoadTexMtxImm(mv, GX_TEXMTX9, GX_MTX2x4);
    MTXScale(ms, 1.0F, 2.0F, 1.0F);
    GXLoadTexMtxImm(ms, GX_TEXMTX0, GX_MTX2x4);
    MTXScale(ms, 5.0F, 5.0F, 5.0F);
    GXLoadTexMtxImm(ms, GX_TEXMTX1, GX_MTX2x4);
    MTXScale(ms, 4.0F, 6.0F, 4.0F);
    GXLoadTexMtxImm(ms, GX_TEXMTX2, GX_MTX2x4);
    
    // enable lighting
    SetLight(&sc->lightCtrl, sc->cam.view);

    // set multitexture environment
    SetMultiTex(sc->mtConfig);

    // draw the model
    DrawModel(&sc->model);
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
    MTXConcat(mry, sc->model.rot, sc->model.rot);
    MTXConcat(mrx, sc->model.rot, sc->model.rot);

    // Light Position Calculation
    sc->lightCtrl.theta += (DEMOPadGetSubStickX(0) / 24);
    sc->lightCtrl.theta %= 360;
    sc->lightCtrl.phi += (DEMOPadGetSubStickY(0) / 24);
    Clamp(sc->lightCtrl.phi, -90, 90);
}

/*---------------------------------------------------------------------------*
    Name:           CreateModel
    
    Description:    Create indexed data array for the model
                    (a torus with NBT, texcoord)
                    
    Arguments:      mo : pointer to model object
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CreateModel( MyModelObj* mo )
{
#define  MODEL_R   0.35F
#define  SCALE_Q   0x2000
    u32  size_p, size_n, size_t;
    u32  cnt_p, cnt_n, cnt_t;
    s32  i, j;
    f32  fs, ft, fr;

    // allocate necessary array memories
    if ( mo->posArray != NULL )
        OSFree(mo->posArray);
    if ( mo->nrmArray != NULL )
        OSFree(mo->nrmArray);
    if ( mo->tcdArray != NULL )
        OSFree(mo->tcdArray);

    size_p = mo->numDiv0 * mo->numDiv1 * 3;
    size_n = mo->numDiv0 * mo->numDiv1 * 9;
    size_t = ( mo->numDiv0 + 1 ) * ( mo->numDiv1 + 1 ) * 2;

    mo->posArray = (s16*)OSAlloc(size_p * sizeof(s16));
    mo->nrmArray = (s16*)OSAlloc(size_n * sizeof(s16));
    mo->tcdArray = (s16*)OSAlloc(size_t * sizeof(s16));

    // make array data
    cnt_p = cnt_n = cnt_t = 0;
    for ( i = 0 ; i <= mo->numDiv0 ; ++i )
    {
        for ( j = 0 ; j <= mo->numDiv1 ; ++j )
        {
            // Tex coord
            mo->tcdArray[cnt_t++] = (s16)(i * SCALE_Q / mo->numDiv0);
            mo->tcdArray[cnt_t++] = (s16)(j * SCALE_Q / mo->numDiv1);
            
            if ( i == mo->numDiv0 || j == mo->numDiv1 )
                continue;
            
            // Position
            fs = i * PI_2 / mo->numDiv0;
            ft = j * PI_2 / mo->numDiv1;
            fr = 1.0F + MODEL_R * cosf(fs);

            mo->posArray[cnt_p++] = (s16)(fr * cosf(ft) * SCALE_Q);
            mo->posArray[cnt_p++] = (s16)(fr * sinf(ft) * SCALE_Q);
            mo->posArray[cnt_p++] = (s16)(MODEL_R * sinf(fs) * SCALE_Q);

            // Normal
            mo->nrmArray[cnt_n++] = (s16)(cosf(ft) * cosf(fs) * SCALE_Q);
            mo->nrmArray[cnt_n++] = (s16)(sinf(ft) * cosf(fs) * SCALE_Q);
            mo->nrmArray[cnt_n++] = (s16)(sinf(fs) * SCALE_Q);
            
            // Bi-normal
            mo->nrmArray[cnt_n++] = (s16)(-cosf(ft) * sinf(fs) * SCALE_Q);
            mo->nrmArray[cnt_n++] = (s16)(-sinf(ft) * sinf(fs) * SCALE_Q);
            mo->nrmArray[cnt_n++] = (s16)( cosf(fs) * SCALE_Q);
            
            // Tangent
            mo->nrmArray[cnt_n++] = (s16)(-sinf(ft) * SCALE_Q);
            mo->nrmArray[cnt_n++] = (s16)( cosf(ft) * SCALE_Q);
            mo->nrmArray[cnt_n++] = 0;
        }
    }

    // make sure data is written to main memory
    DCFlushRange(mo->posArray, size_p * sizeof(s16));
    DCFlushRange(mo->nrmArray, size_n * sizeof(s16));
    DCFlushRange(mo->tcdArray, size_t * sizeof(s16));

}

/*---------------------------------------------------------------------------*
    Name:           DrawModel
    
    Description:    Draws a model by using prepared array data
                    
    Arguments:      mo : pointer to model object
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModel( MyModelObj* mo )
{
    u32  i, j, k, s, t;
    u16  idx;

    // set up array pointer
    GXSetArray(GX_VA_POS,  mo->posArray, sizeof(s16) * 3);
    GXSetArray(GX_VA_NRM,  mo->nrmArray, sizeof(s16) * 9);
    GXSetArray(GX_VA_TEX0, mo->tcdArray, sizeof(s16) * 2);

    // set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxDesc(GX_VA_NBT, GX_INDEX16);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);

    for ( i = 0 ; i < mo->numDiv0 ; i++ )
    {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)((mo->numDiv1+1)*2));
            for ( j = 0 ; j <= mo->numDiv1 ; j++ )
            {
                for ( k = 0 ; k <= 1 ; k++ )
                {
                    s = (i + k) % mo->numDiv0;
                    t = j % mo->numDiv1;
                    
                    // Position
                    idx = (u16)(s * mo->numDiv1 + t);
                    GXPosition1x16(idx);
                    
                    // Normal-Binormal-Tangent
                    GXNormal1x16(idx);

                    // Tex coord
                    idx = (u16)((i+k) * (mo->numDiv1+1) + j);
                    GXTexCoord1x16(idx);
                }
            }
        GXEnd();
    }
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
    GXInitLightShininess(&lo1, 16.0F);
    GXInitLightColor(&lo1, litc1);
    GXLoadLightObjImm(&lo1, GX_LIGHT1);

    // Lighting channel
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_ENABLE,       // enable channel
        GX_SRC_REG,      // amb source
        GX_SRC_REG,      // mat source
        GX_LIGHT0,       // light mask
        GX_DF_CLAMP,     //
        GX_AF_NONE );    // used as diffuse light 
    GXSetChanCtrl(
        GX_COLOR1A1,
        GX_ENABLE,       // enable channel
        GX_SRC_REG,      // amb source
        GX_SRC_REG,      // mat source
        GX_LIGHT1,       // light mask
        GX_DF_NONE,      //
        GX_AF_SPEC );    // used as specular light
    
    // set up ambient/material color
    GXSetChanAmbColor(GX_COLOR0A0, ambc0);
    GXSetChanAmbColor(GX_COLOR1A1, ambc1);
    GXSetChanMatColor(GX_COLOR0A0, matc0);
    GXSetChanMatColor(GX_COLOR1A1, matc1);
}

/*---------------------------------------------------------------------------*
    Name:           SetMultiTex
    
    Description:    Set up texture maps, texcoord generators and TEV
                    for complex multitextureing.
                    
    Arguments:      mtc : pointer to a multi-texture configuration structure

    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetMultiTex( MyMTConfig* mtc )
{
    u32  i;

    ASSERTMSG(mtc->numChannels <= MAX_CHANNELS, "Too much channels\n");
    ASSERTMSG(mtc->numTexCoords <= MAX_TEXCOORDS, "Too much texcoords\n");
    ASSERTMSG(mtc->numTexMaps <= MAX_TEXMAPS, "Too much texmaps\n");
    ASSERTMSG(mtc->numTevStages <= MAX_TEVSTAGES, "Too much tevstages\n");

    // Number of color channels
    GXSetNumChans(mtc->numChannels);

    // Texmaps
    for ( i = 0 ; i < mtc->numTexMaps ; ++i )
    {
        GXLoadTexObj(&mtc->texMapArray[i], MapIDTbl[i]);
    }

    // Texcoord generators 
    GXSetNumTexGens(mtc->numTexCoords);
    for ( i = 0 ; i < mtc->numTexCoords ; ++i )
    {
        GXSetTexCoordGen(
            CoordIDTbl[i],
            mtc->texGenArray[i].func,
            mtc->texGenArray[i].src,
            mtc->texGenArray[i].mt );
    }
    
    // TEV
    GXSetNumTevStages(mtc->numTevStages);
    GXSetTevColor(GX_TEVREG0, mtc->tevRegColor[0]);
    GXSetTevColor(GX_TEVREG1, mtc->tevRegColor[1]);
    GXSetTevColor(GX_TEVREG2, mtc->tevRegColor[2]);

    // each TEV stage
    for ( i = 0 ; i < mtc->numTevStages ; ++i )
    {
        // color operations
        GXSetTevColorIn(
            StageIDTbl[i],
            mtc->tevStgArray[i].colArgs[0],
            mtc->tevStgArray[i].colArgs[1],
            mtc->tevStgArray[i].colArgs[2],
            mtc->tevStgArray[i].colArgs[3] );
        GXSetTevColorOp(
            StageIDTbl[i],
            mtc->tevStgArray[i].colOp,
            mtc->tevStgArray[i].colBias,
            mtc->tevStgArray[i].colScale,
            mtc->tevStgArray[i].colClamp,
            mtc->tevStgArray[i].colOut );
        GXSetTevAlphaIn(
            StageIDTbl[i],
            mtc->tevStgArray[i].alpArgs[0],
            mtc->tevStgArray[i].alpArgs[1],
            mtc->tevStgArray[i].alpArgs[2],
            mtc->tevStgArray[i].alpArgs[3] );
        GXSetTevAlphaOp(
            StageIDTbl[i],
            mtc->tevStgArray[i].alpOp,
            mtc->tevStgArray[i].alpBias,
            mtc->tevStgArray[i].alpScale,
            mtc->tevStgArray[i].alpClamp,
            mtc->tevStgArray[i].alpOut );
       
        // TEV order
        GXSetTevOrder(
            StageIDTbl[i],
            mtc->tevStgArray[i].texCoord,
            mtc->tevStgArray[i].texMap,
            mtc->tevStgArray[i].channel );
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
    OSReport("************************************************\n");
    OSReport("tev-complex: complex multitexture\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
    OSReport("Main Stick   : Rotate the model\n");
    OSReport("Sub  Stick   : Move Light Position\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
