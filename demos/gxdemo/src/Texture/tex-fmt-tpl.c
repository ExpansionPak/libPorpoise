/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-fmt-tpl.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-fmt-tpl.c $    
    
    5     5/11/01 12:29a Hirose
    removed obsolete flags. fixed some verify messages.
    
    4     3/24/00 4:30p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    9     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    8     2/26/00 11:11p Hirose
    fixed intro message
    
    7     2/25/00 2:50p Hirose
    updated pad control function to match actual HW1 gamepad
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     2/08/00 7:12p Hirose
    fixed wrap mode setting to use values provided by TPL
    
    4     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    3     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    19    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    18    11/15/99 4:49p Yasu
    Change datafile name
    
    17    11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    16    10/14/99 6:37p Hirose
    changed tpl file list
    
    15    10/12/99 10:19a Hirose
    a small bug fix
    
    14    10/06/99 9:08p Hirose
    added single frame test
    
    13    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    12    9/20/99 11:51a Ryan
    update to use new DEMO calls
    
    11    9/17/99 6:38p Hirose
    changed GXInitTlutObj and GXLoadTlut parameters
    
    10    9/16/99 8:01p Hirose
    arbitrary TLUT size support
    
    9     9/14/99 4:05p Hirose
    checked alighment/coherency
    removed all warnings
    
    8     9/07/99 8:14p Hirose
    changed file location and removed view reset
    
    7     9/01/99 9:42p Hirose
    added odd size tpl files
    
    6     9/01/99 5:45p Ryan
    
    5     8/27/99 3:42p Yasu
    Change the parameter of GXSetZMode and append GXSetZUpdate
    
    4     8/26/99 10:29p Hirose
    fixed file path
    
    3     8/17/99 12:02p Alligator
    updated to reflect arbitrary tlut size
    
    2     8/12/99 4:43p Hirose
    
    1     8/12/99 3:58p Hirose
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-fmt-tpl
        display various format textures ( tpl version )
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <string.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define MAX_SCOORD         0x4000  // for 16b, 2.14 fixed point format
#define MAX_TCOORD         0x4000  // for 16b, 2.14 fixed point format
#define STEP_SCOORD        2
#define STEP_TCOORD        3

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

// for texture
typedef struct
{
    GXTexObj      tobj;
    u32           width;
    u32           height;
} MyTexObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    MyTexObj       texture;
    Mtx            modelCtrl;
    f32            modelScale; 
    u32            texNumber;
    u32            texFileNumber;
    GXTevMode      tevMode;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main           ( void );
static void DrawInit       ( MySceneCtrlObj* sc );
static void DrawTick       ( MySceneCtrlObj* sc );
static void AnimTick       ( MySceneCtrlObj* sc );
static void GetTplTexture  ( MyTexObj* to, u32 num );
static void LoadTplFile    ( char* fileName );
static void DrawBox        ( void );
static void SetCamera      ( MyCameraObj* cam );
static void PrintIntro     ( void );

/*---------------------------------------------------------------------------*
   Model Data
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   Data arrays for indexed primitives must be 32B aligned.  Normally, memory
   for the arrays would be OSAlloc'd (which returns 32B aligned pointers) and
   the data would be loaded from ROM.  The pragma variable_align provides a
   convenient way to align initialized arrays.
 *---------------------------------------------------------------------------*/
static s16 TopVerts[] ATTRIBUTE_ALIGN(32) = 
{
    -1, -1,  1, // 0
     1, -1,  1, // 1
     1,  1,  1, // 2
    -1,  1,  1, // 3
    -1, -1, -1, // 4
     1, -1, -1, // 5
     1,  1, -1, // 6
    -1,  1, -1  // 7
};

static u8 Colors[] ATTRIBUTE_ALIGN(32) =
{
    0x80, 0x00, 0x00, 0x80,
    0x00, 0x80, 0x00, 0x80,
    0x00, 0x00, 0x80, 0x80,
    0x80, 0x80, 0x00, 0x80,
    0x80, 0x00, 0x80, 0x80,
    0x00, 0x80, 0x80, 0x80,
};

/*---------------------------------------------------------------------------*
   Debug Message Strings
 *---------------------------------------------------------------------------*/
static char* TxFmtStr[] =
{
    "I4",
    "I8",
    "IA4",
    "IA8",
    "RGB565",
    "RGB5A3",
    "RGBA8",
    "?",
    "CI4",
    "CI8",
    "CI14X2",
    "?",
    "?",
    "?",
    "Compressed"
};

static char* TxWrapStr[] =
{
    "Clamp",
    "Repeat",
    "Mirror"
};

static char* TxFilterStr[] =
{
    "Near",
    "Linear",
    "Near/Near",
    "Linear/Near",
    "Near/Linear",
    "Linear/Linear"
};

static char* TxSwStr[] =
{
    "OFF",
    "ON"
};

/*---------------------------------------------------------------------------*
  Tpl File List
 *---------------------------------------------------------------------------*/
#define NUM_OF_TPL_FILES    26

static char* TplFileList[] =
{
    // I4
    "i4_1",     "i4_1mm",     "i4_odd",
    // I8
    "i8_1",     "i8_1mm",     "i8_odd",
    // IA4
    "ia4_1",    "ia4_1mm",    "ia4_odd",
    // IA8
    "ia8_1",    "ia8_1mm",    "ia8_odd",
    // RGB565
    "rgb565_1", "rgb565mm",   "rgb565od",
    // RGB5A3
    "rgb5a3_1", "rgb5a3mm",   "rgb5a3od",
    // RGBA8
    "rgba8_1",  "rgba8mm",    "rgba8od",
    // Compressed
    "cmp_1",    "cmp_1mm",    "cmp_even",
    // CI8
    "ci8_1",    "ci8_2"
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, -5000.0F, 0.0F }, // location
    { 0.0F,     0.0F, 1.0F }, // up
    { 0.0F,     0.0F, 0.0F }, // tatget
    -320.0F, // left
    240.0F,  // top
    0.0F,    // near
    10000.0F // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj    SceneCtrl;      // scene control parameters
static TEXPalettePtr     MyTplObj = 0;   // texure palette
static u32               TexNumMax;      // number of textures

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);  // Init the OS, game pad, graphics and video.

    PrintIntro(); // Print demo directions

    DrawInit(&SceneCtrl); // Initialize vertex formats, array pointers
                          // and default scene settings.

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
static void DrawInit( MySceneCtrlObj* sc )
{
    // Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);   
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 14);

    // Array Pointers and Strides
    GXInvalidateVtxCache();
    // stride = 3 elements (x,y,z) each of type s16
    GXSetArray(GX_VA_POS, TopVerts, 3*sizeof(s16));
    // stride = 4 elements (r,g,b,a) each of type u8
    GXSetArray(GX_VA_CLR0, Colors, 4*sizeof(u8));

    // backface culling off
    GXSetCullMode(GX_CULL_NONE);

	GXSetNumChans(1); // GXInit passes vertex color to color channel/default
	GXSetNumTexGens(1);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    // Default scene control parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // Load the first Tpl Texture
    sc->texFileNumber = 0;
    sc->texNumber     = 0;
    LoadTplFile(TplFileList[sc->texFileNumber]);
    GetTplTexture(&sc->texture, sc->texNumber);

    // model control parameters
    sc->modelScale = 1.0F;
    MTXIdentity(sc->modelCtrl);

    // tev mode
    sc->tevMode = GX_REPLACE;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    static u32 s = 0;
    static u32 t = 0;

    Mtx  ms; // Model scale matrix.
    Mtx  mv; // Modelview matrix.

    // Set modelview matrix
    MTXConcat(sc->cam.view, sc->modelCtrl, mv); 
    MTXScale(
        ms, 
        sc->modelScale * sc->texture.width / 2,
        sc->modelScale * sc->texture.height / 2,
        sc->modelScale * sc->texture.height / 2 );
    MTXConcat(mv, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    // tev mode
    GXSetTevOp(GX_TEVSTAGE0, sc->tevMode);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    // load texture obj
    GXLoadTexObj(&sc->texture.tobj, GX_TEXMAP0);
    
    // draw a box
    DrawBox();
    
    // translate s, t coordinates
    s += STEP_SCOORD;
    t += STEP_TCOORD;
    if (s > MAX_SCOORD) s = 0;
    if (t > MAX_TCOORD) t = 0;

}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    u16  down;
    Mtx  mrx, mry;
    
    // PAD
    down = DEMOPadGetButtonDown(0);

    // Model Rotation Calculation   
    MTXRotDeg(mry, 'x', -(DEMOPadGetStickY(0) / 16));
    MTXRotDeg(mrx, 'z',  (DEMOPadGetStickX(0) / 16));
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);

    // Changes model scale
    sc->modelScale += (DEMOPadGetSubStickY(0) / 16) * 0.0125F;
    Clamp(sc->modelScale, 0.1F, 4.0F);
    

    // Change texture
    if ( down & PAD_BUTTON_Y )
    {
        sc->texNumber = ( sc->texNumber + 1 ) % TexNumMax;
        GetTplTexture(&sc->texture, sc->texNumber);
    }
    if ( down & PAD_BUTTON_X )
    {
        sc->texNumber = ( sc->texNumber + TexNumMax - 1 ) % TexNumMax;
        GetTplTexture(&sc->texture, sc->texNumber);
    }

    // Change tpl file
    if ( down & PAD_TRIGGER_R )
    {
        sc->texFileNumber = ( sc->texFileNumber + 1 ) % NUM_OF_TPL_FILES;
        LoadTplFile(TplFileList[sc->texFileNumber]);
        sc->texNumber = 0;
        GetTplTexture(&sc->texture, sc->texNumber);
    }
    if ( down & PAD_TRIGGER_L )
    {
        sc->texFileNumber = ( sc->texFileNumber + NUM_OF_TPL_FILES - 1 ) % NUM_OF_TPL_FILES;
        LoadTplFile(TplFileList[sc->texFileNumber]);
        sc->texNumber = 0;
        GetTplTexture(&sc->texture, sc->texNumber);
    }

    // Change Tev mode
    if ( down & PAD_BUTTON_B )
    {
        sc->tevMode = ( sc->tevMode == GX_REPLACE ) ? GX_DECAL : GX_REPLACE;
    }

    // Reset view
    if ( down & PAD_BUTTON_A )
    {
        sc->modelScale = 1.0F;
        MTXIdentity(sc->modelCtrl);
    }

}

/*---------------------------------------------------------------------------*
    Name:           GetTplTexture
    
    Description:    get current texture from the texture palette
                    
    Arguments:      to  : a pointer to MyTexObj structure to be set
                    num : texture number in the tpl
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void GetTplTexture( MyTexObj* to, u32 num )
{
    TEXDescriptorPtr tdp;
    u32              fmt;
    GXBool           mipMapFlag;

    if ( num >= MyTplObj->numDescriptors )
    {
        num = 0;
    }

    // Get a texture descriptor
    tdp = TEXGet(MyTplObj, num);
    
    mipMapFlag =
        ( tdp->textureHeader->minLOD == tdp->textureHeader->maxLOD ) ?
        GX_FALSE : GX_TRUE;

    fmt = (u32)tdp->textureHeader->format;
    to->width  = tdp->textureHeader->width;
    to->height = tdp->textureHeader->height;

    if ( fmt == GX_TF_C4 || fmt == GX_TF_C8 )
    {
        GXTlutObj tlutObj;
    
        GXInitTexObjCI(
            &to->tobj,
            tdp->textureHeader->data,
            tdp->textureHeader->width,
            tdp->textureHeader->height,
            (GXCITexFmt)fmt,
            tdp->textureHeader->wrapS, // s
            tdp->textureHeader->wrapT, // t
            mipMapFlag, // Mipmap
            GX_TLUT0 );
    
        GXInitTlutObj(
            &tlutObj,
            tdp->CLUTHeader->data,
            (GXTlutFmt)tdp->CLUTHeader->format,
            tdp->CLUTHeader->numEntries );

        GXLoadTlut(&tlutObj, GX_TLUT0);
    }
    else
    {
        GXInitTexObj(
            &to->tobj,
            tdp->textureHeader->data,
            tdp->textureHeader->width,
            tdp->textureHeader->height,
            (GXTexFmt)fmt,
            tdp->textureHeader->wrapS, // s
            tdp->textureHeader->wrapT, // t
            mipMapFlag ); // Mipmap
    }

    GXInitTexObjLOD(
        &to->tobj,
        tdp->textureHeader->minFilter, 
        tdp->textureHeader->magFilter, 
        tdp->textureHeader->minLOD, 
        tdp->textureHeader->maxLOD, 
        tdp->textureHeader->LODBias, 
        GX_FALSE,
        tdp->textureHeader->edgeLODEnable, 
        GX_ANISO_1 );

    OSReport("*******************************************\n");
    OSReport("Width  = %04d\t\t", tdp->textureHeader->width);
    OSReport("Height = %04d\n", tdp->textureHeader->height);
    OSReport("Format = %s ", TxFmtStr[tdp->textureHeader->format]);
    OSReport("(%d)\n", tdp->textureHeader->format);
    OSReport("Wrap_s = %s\t\t", TxWrapStr[tdp->textureHeader->wrapS]);
    OSReport("Wrap_t = %s\n", TxWrapStr[tdp->textureHeader->wrapT]);
    OSReport("MinFilter = %s\t", TxFilterStr[tdp->textureHeader->minFilter]);
    OSReport("MagFilter = %s\n", TxFilterStr[tdp->textureHeader->magFilter]);
    OSReport("MinLOD = %d\t\t\t", tdp->textureHeader->minLOD);  
    OSReport("MaxLOD = %d\n", tdp->textureHeader->maxLOD);  
    OSReport("LODBias = %f\n", tdp->textureHeader->LODBias);    
    OSReport("MipMap  = %s\n", TxSwStr[mipMapFlag]);
    OSReport("*******************************************\n");
}

/*---------------------------------------------------------------------------*
    Name:           LoadTplFile
    
    Description:    Load tpl file with specified name.
                    
    Arguments:      fileName : tpl file name
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void LoadTplFile( char* fileName )
{
    char strBuffer[64];

    if ( MyTplObj != 0 )
    {
        TEXReleasePalette(&MyTplObj);
    }

    strcpy(strBuffer,"gxTests/tex-02/");
    strcat(strBuffer, fileName);
    strcat(strBuffer, ".tpl");
    OSReport("File: %s\n", strBuffer);

    TEXGetPalette(&MyTplObj, strBuffer);
    TexNumMax = MyTplObj->numDescriptors;

    GXInvalidateTexAll();
}

/*---------------------------------------------------------------------------*
    Name:           VertexT
    
    Description:    Draw a vertex with texture, direct data
                    
    Arguments:      v        8-bit position index
                    s, t     16-bit tex coord
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void VertexT( u8 v, u32 s, u32 t, u8 c )
{
    GXPosition1x8(v);
    GXColor1x8(c);
    GXTexCoord2u16((u16)s, (u16)t);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTexQuad
    
    Description:    Draw a textured quad.  Map extends to corners of the quad.
                    MAX_SCOORD is the value of 1.0 in the fixed point format.

    Arguments:      v0        8-bit position
                    v1        8-bit position
                    v2        8-bit position
                    v3        8-bit position
                    s0        s tex coord at v0
                    t0        t tex coord at v0
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void DrawTexQuad(
    u8  v0,
    u8  v1,
    u8  v2,
    u8  v3,
    u32 s0,
    u32 t0,
    u8  c ) 
{
    VertexT(v0, s0+MAX_SCOORD, t0, c);
    VertexT(v1, s0, t0, c);
    VertexT(v2, s0, t0+MAX_TCOORD, c);
    VertexT(v3, s0+MAX_SCOORD, t0+MAX_TCOORD, c);
}

/*---------------------------------------------------------------------------*
    Name:           DrawBox
    
    Description:    Draw a box model.
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawBox( void )
{
    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // draw the box
    GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);
        DrawTexQuad(1, 0, 3, 2, 0, 0, 0);
        DrawTexQuad(6, 7, 4, 5, 0, 0, 1);
        DrawTexQuad(3, 0, 4, 7, 0, 0, 2);
        DrawTexQuad(0, 1, 5, 4, 0, 0, 3);
        DrawTexQuad(1, 2, 6, 5, 0, 0, 4);
        DrawTexQuad(2, 3, 7, 6, 0, 0, 5);
    GXEnd();
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
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void PrintIntro(void)
{
    OSReport("***************************************\n");
    OSReport("Instructions:\n");
    OSReport("    Start       : quit this program\n");
    OSReport("    Main Stick  : rotate the box\n");
    OSReport("    Sub Stick Y : change scale of the box\n");
    OSReport("    X Button    : view next TPL file\n");
    OSReport("    Y Button    : view previous TPL file\n");
    OSReport("    B Button    : change tev mode (to check alpha)\n");
    OSReport("    A Button    : reset the view\n");
    OSReport("***************************************\n");
}

/*============================================================================*/
