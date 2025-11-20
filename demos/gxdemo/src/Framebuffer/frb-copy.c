/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     frb-copy.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Framebuffer/frb-copy.c $    
    
    8     5/05/01 10:05p Hirose
    removed obsolete flags
    
    7     10/13/00 12:49a Hirose
    added Rev.B(HW2) new copy format test, and misc changes
    
    6     10/04/00 10:22p Hirose
    fixed screen space projection setting.
    texture copy width should be multipue of 4 pixels.
    
    5     5/02/00 4:03p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    4     3/24/00 10:21p Hirose
    fixed copy region alignment problem caused by FB size change
    
    3     3/23/00 3:24p Hirose
    updated to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:09p Alligator
    move from tests/gx and rename
    
    10    3/03/00 4:21p Alligator
    integrate with ArtX source
    
    9     2/24/00 7:29p Hirose
    updated pad control functions to match HW1
    
    8     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    7     2/14/00 6:10p Hirose
    added dummy GXCopyDisp to clear framebuffer 
    
    6     2/12/00 5:16p Alligator
    Integrate ArtX source tree changes
    
    5     2/06/00 1:52a Hirose
    added A8 format test
    
    4     1/24/00 2:36p Carl
    Removed #ifdef EPPC stuff
    
    3     1/19/00 7:05p Alligator
    fix EPPC errors
    
    2     1/13/00 8:53p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    13    11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    12    11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    11    11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    10    10/29/99 6:31p Hirose
    
    9     10/29/99 3:46p Hirose
    replaced GXSetTevStages(GXTevStageID) by GXSetNumTevStages(u8)
    
    8     10/25/99 4:29a Yasu
    Add GXSetTevOrder/TevStages
    
    7     10/21/99 4:53p Hirose
    added mipmap filter option test and clear option test
    
    6     10/20/99 3:59p Hirose
    put GXSetPixelFmt in DrawInit
    
    5     10/19/99 11:37a Hirose
    added single frame test
    
    4     10/18/99 3:19p Hirose
    added cube animation and camera control
    
    3     10/17/99 3:52a Hirose
    
    2     10/15/99 9:49p Hirose
    
    1     10/14/99 8:10p Hirose
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    frb-copy
        texture copy test
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

#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

#define ClearEFB() \
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE)

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
    f32           theta;
} MyCameraObj;

// for copied texture
typedef struct
{
    GXTexObj      tobj;
    u8*           data;
    s32           top;
    s32           left;
    s32           width;
    s32           height;
    u32           format;
    GXBool        mmFilter;
    GXBool        clear;
} MyCopyTexObj;

// for entire scene control
typedef struct
{
    MyCameraObj       cam;
    MyCopyTexObj      copyTex;
    u16               screen_width;
    u16               screen_height;
    u16               copy_width_max;
    u16               copy_height_max;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main               ( void );
static void DrawInit           ( MySceneCtrlObj* sc );
static void DrawTick           ( MySceneCtrlObj* sc );
static void AnimTick           ( MySceneCtrlObj* sc );
static void DrawFloor          ( void );
static void DrawScreenPanel    ( void );
static void DrawCubes          ( Mtx view );
static void DrawTexCopyFrame   ( MyCopyTexObj* ct );
static void CopyTextureFromFB  ( MyCopyTexObj* ct );
static void SetCamera          ( MyCameraObj* cam );
static void SetScreenSpaceMode ( MySceneCtrlObj* sc );
static void SetLight           ( void );
static void DisableLight       ( void );
static void PrintIntro         ( void );

/*---------------------------------------------------------------------------*
  Model and texture data
 *---------------------------------------------------------------------------*/
// for cube models
#define NUM_CUBES    8
#define REG_AMBIENT  ColorArray[NUM_CUBES]
#define LIGHT_COLOR  ColorArray[NUM_CUBES+1]
#define BG_COLOR     ColorArray[NUM_CUBES+2]

static GXColor ColorArray[NUM_CUBES+3] ATTRIBUTE_ALIGN(32) = 
{
    { 0x80, 0xFF, 0x80, 0xFF },
    { 0x00, 0xFF, 0xFF, 0xFF },
    { 0xFF, 0x00, 0xFF, 0xFF },
    { 0xFF, 0xFF, 0x00, 0xFF },
    { 0x20, 0x20, 0xFF, 0x80 },
    { 0x20, 0xA0, 0x00, 0x80 },
    { 0xC0, 0xC0, 0xC0, 0x40 },
    { 0xFF, 0x80, 0x80, 0x40 },
    { 0x40, 0x40, 0x40, 0xFF },    // Gray  (Ambient etc.)
    { 0xFF, 0xFF, 0xFF, 0xFF },    // White (Light etc.)
    { 0x00, 0x00, 0x00, 0x00 }     // Background
};

static Vec CubePosArray[NUM_CUBES] =
{
    { -400, -150,  30 },
    {  400, -150,  30 },
    { -400,  250,   0 },
    {  400,  250,   0 },
    { -600,   50, -50 },
    {  600,   50, -50 },
    { -300,   50, -80 },
    {  300,   50, -80 }
};

static f32 CubeDegArray[NUM_CUBES] =
{
     45.0F,
    245.0F,
    120.0F,
    135.0F,
    315.0F,
     60.0F,
    105.0F,
    345.0F
};

// floor texture
#define FLOOR_TEX_WIDTH   4
#define FLOOR_TEX_HEIGHT  4
#define FLOOR_TEX_FORMAT  GX_TF_RGB5A3
static u16 FloorTexData[] ATTRIBUTE_ALIGN(32) =
{
    0x7CCC, 0x7CCC, 0x700F, 0x700F,
    0x7CCC, 0x7CCC, 0x700F, 0x700F,
    0x700F, 0x700F, 0x7CCC, 0x7CCC,
    0x700F, 0x700F, 0x7CCC, 0x7CCC
};

// blank texture for the first frame
#define BLANK_TEX_WIDTH   4
#define BLANK_TEX_HEIGHT  4
#define BLANK_TEX_FORMAT  GX_TF_RGB5A3
static u16 BlankTexData[] ATTRIBUTE_ALIGN(32) =
{
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000
};

// texture formats for copy and display
#if ( GX_REV == 1 )     // GX revision 1
#define TEX_FORMATS     8
#else                   // GX revision 2 or later
#define TEX_FORMATS     22
#endif

static struct
{
    GXTexFmt    cpyFmt;
    GXTexFmt    useFmt;
    char*       cpyFmtStr;
    char*       useFmtStr;
}
TexFormats[TEX_FORMATS] =
{
    // Copy format, Display format
    { GX_TF_RGB565, GX_TF_RGB565, "RGB565", "RGB565" },
    { GX_TF_RGB5A3, GX_TF_RGB5A3, "RGB5A3", "RGB5A3" },
    { GX_TF_RGBA8,  GX_TF_RGBA8,  "RGBA8",  "RGBA8"  },
    { GX_TF_I4,     GX_TF_I4,     "I4",     "I4"     },
    { GX_TF_I8,     GX_TF_I8,     "I8",     "I8"     },
    { GX_TF_IA4,    GX_TF_IA4,    "IA4",    "IA4"    },
    { GX_TF_IA8,    GX_TF_IA8,    "IA8",    "IA8"    },
    { GX_TF_A8,     GX_TF_I8,     "A8",     "I8"     },

#if ( GX_REV != 1 )     // GX revision 2 or later

    { GX_CTF_R4,    GX_TF_I4,     "R4",     "I4"     }, // copy as R4,  use as I4
    { GX_CTF_RA4,   GX_TF_IA4,    "RA4",    "IA4"    }, // copy as RA4, use as IA4
    { GX_CTF_RA8,   GX_TF_IA8,    "RA8",    "IA8"    }, // copy as RA8, use as IA8
    { GX_CTF_R8,    GX_TF_I8,     "R8",     "I8"     }, // copy as R8,  use as I8
    { GX_CTF_G8,    GX_TF_I8,     "G8",     "I8"     }, // copy as G8,  use as I8
    { GX_CTF_B8,    GX_TF_I8,     "B8",     "I8"     }, // copy as B8,  use as I8
    { GX_CTF_RG8,   GX_TF_IA8,    "RG8",    "IA8"    }, // copy as RG8, use as IA8
    { GX_CTF_GB8,   GX_TF_IA8,    "GB8",    "IA8"    }, // copy as GB8, use as IA8
    { GX_TF_Z16,    GX_TF_IA8,    "Z16",    "IA8"    }, // copy as Z16, use as IA8
    { GX_TF_Z8,     GX_TF_I8,     "Z8",     "I8"     }, // copy as Z8,  use as I8
    { GX_CTF_Z4,    GX_TF_I4,     "Z4",     "I4"     }, // copy as Z4,  use as I4
    { GX_CTF_Z8M,   GX_TF_I8,     "Z8M",    "I8"     }, // copy as Z8M, use as I8
    { GX_CTF_Z8L,   GX_TF_I8,     "Z8L",    "I8"     }, // copy as Z8L, use as I8
    { GX_CTF_Z16L,  GX_TF_IA8,    "Z16L",   "IA8"    }, // copy as Z16L,use as IA8

#endif // ( GX_REV != 1 )
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    {   0.0F, 0.0F, 800.0F }, // location
    {   0.0F, 1.0F, 0.0F }, // up
    {   0.0F, 0.0F, 0.0F }, // target
    -320.0F,  // left
    240.0F,   // top
    400.0F,   // near
    2000.0F   // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;                // scene control parameters
static GXTexObj         MyFloorTexture;           // texture for the floor
static GXTexObj         MyBlankTexture;           // blank texture

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
static void DrawInit( MySceneCtrlObj* sc )
{
    GXRenderModeObj *rmp;

    // Get framebuffer size of current rendering mode
    rmp = DEMOGetRenderModeObj();
    sc->screen_width  = rmp->fbWidth;
    sc->screen_height = rmp->efbHeight;
    sc->copy_width_max  = (u16)((sc->screen_width  * 5 / 32) * 4);
    sc->copy_height_max = (u16)((sc->screen_height * 5 / 12) * 2);

    //  Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    //  Array Pointers and Strides
    GXSetArray(GX_VA_CLR0, ColorArray, 4 * sizeof(u8));

    // Z compare, pixel format and background color
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    GXSetCopyClear(BG_COLOR, MAX_Z);
    GXSetAlphaUpdate(GX_ENABLE);

    // Set number of tev stages, tex coord generators and color channels
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetNumChans(1);


    // Default scene control parameter settings

    // camera
    sc->cam.cfg   = DefaultCamera;
    sc->cam.theta = 0;

    // for floor texture and blank texture
    GXInitTexObj(
        &MyFloorTexture,
        FloorTexData,
        FLOOR_TEX_WIDTH,
        FLOOR_TEX_HEIGHT,
        FLOOR_TEX_FORMAT,
        GX_REPEAT,
        GX_REPEAT,
        GX_FALSE );
    GXInitTexObjLOD(
        &MyFloorTexture,
        GX_LINEAR,
        GX_NEAR,
        0.0F,
        0.0F,
        0.0F,
        GX_FALSE,
        GX_FALSE,
        GX_ANISO_1 );
    GXInitTexObj(
        &MyBlankTexture,
        BlankTexData,
        BLANK_TEX_WIDTH,
        BLANK_TEX_HEIGHT,
        BLANK_TEX_FORMAT,
        GX_CLAMP,
        GX_CLAMP,
        GX_FALSE );

    // for copied texture
    sc->copyTex.top      = 64;
    sc->copyTex.left     = 64;
    sc->copyTex.width    = 32;
    sc->copyTex.height   = 32;
    sc->copyTex.format   = 0;
    sc->copyTex.data     = NULL;
    sc->copyTex.mmFilter = GX_FALSE;
    sc->copyTex.clear    = GX_FALSE;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx    mv;

    // Texture data is changed every frame
    GXInvalidateTexAll();

    // Set up camera
    SetCamera(&sc->cam);
    
    // Draw cube models
    DrawCubes(sc->cam.view);

    // Draw the floor
    GXLoadTexObj(&MyFloorTexture, GX_TEXMAP0);
    MTXCopy(sc->cam.view, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    DrawFloor();

    // Draw a screen panel which shows copied texture
    if ( sc->copyTex.data != NULL )
    {
        GXLoadTexObj(&sc->copyTex.tobj, GX_TEXMAP0);
    }
    else
    {
        GXLoadTexObj(&MyBlankTexture, GX_TEXMAP0);
    }
    MTXCopy(sc->cam.view, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    DrawScreenPanel();

    // Draw the frame which shows the area of copy texture
    SetScreenSpaceMode(sc);
    MTXIdentity(mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    DrawTexCopyFrame(&sc->copyTex);

    // Copy texture from the frame buffer
    CopyTextureFromFB(&sc->copyTex);

    // Caption
    DEMOInitCaption(DM_FT_OPQ, (s16)sc->screen_width, (s16)sc->screen_height);
    DEMOPrintf(32, (s16)(sc->screen_height - 40 ), 0,
               " COPY : %s ", TexFormats[sc->copyTex.format].cpyFmtStr);
    DEMOPrintf(32, (s16)(sc->screen_height - 32 ), 0,
               " USE  : %s ", TexFormats[sc->copyTex.format].useFmtStr);
    // Resume Z mode
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    u16  button, down;
    
    button = DEMOPadGetButton(0);
    down   = DEMOPadGetButtonDown(0);

    // Moves copy area
    sc->copyTex.width  += ( DEMOPadGetSubStickX(0) / 32 ) * 4;
    sc->copyTex.height -= ( DEMOPadGetSubStickY(0) / 24 ) * 2;

    // Moves copy area
    sc->copyTex.left += ( DEMOPadGetStickX(0) / 32 ) * 4;
    sc->copyTex.top  -= ( DEMOPadGetStickY(0) / 24 ) * 2;

    Clamp(sc->copyTex.width, 16, sc->copy_width_max);
    Clamp(sc->copyTex.height, 16, sc->copy_height_max);
    Clamp(sc->copyTex.left, 0, sc->screen_width - sc->copyTex.width);
    Clamp(sc->copyTex.top, 0, sc->screen_height - sc->copyTex.height);

    // Camera location control
    sc->cam.theta += (f32)DEMOPadGetTriggerR(0) / 100.0F
                   - (f32)DEMOPadGetTriggerL(0) / 100.0F;

    Clamp(sc->cam.theta, -30.0F, 30.0F);
    sc->cam.cfg.location.x = 800.0F * sinf(sc->cam.theta * PI / 180.0F);
    sc->cam.cfg.location.y = 0.0F;
    sc->cam.cfg.location.z = 800.0F * cosf(sc->cam.theta * PI / 180.0F);

    // Change texture format
    if ( down & PAD_BUTTON_A )
    {
        sc->copyTex.format = ( sc->copyTex.format + 1 ) % TEX_FORMATS;
        //OSReport("Format: %s\n",TexFormats[sc->copyTex.format].cpyFmtStr);
    }

    if ( down & PAD_BUTTON_B )
    {
        sc->copyTex.format = ( sc->copyTex.format + TEX_FORMATS - 1 ) % TEX_FORMATS;
        //OSReport("Format: %s\n",TexFormats[sc->copyTex.format].cpyFmtStr);
    }

    // Mipmap(box) filter option on/off
    if ( down & PAD_BUTTON_Y )
    {
        if ( sc->copyTex.mmFilter )
        {
            sc->copyTex.mmFilter = GX_FALSE;
            OSReport("Mipmap filter option OFF\n");
        }
        else
        {
            sc->copyTex.mmFilter = GX_TRUE;
            OSReport("Mipmap filter option ON\n");
        }
    }

    // Clear option on/off
    if ( down & PAD_BUTTON_X )
    {
        if ( sc->copyTex.clear )
        {
            sc->copyTex.clear = GX_FALSE;
            OSReport("Clear option OFF\n");
        }
        else
        {
            sc->copyTex.clear = GX_TRUE;
            OSReport("Clear option ON\n");
        }
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawFloor
    
    Description:    Draw the floor.
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawFloor( void )
{
    // set Tev operation
    GXSetNumTexGens( 1 );
	GXSetNumChans( 0 );
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // draw the floor
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        GXPosition3s16( -1200, -256, -400 );
        GXTexCoord2s16( 0x0000, 0x0000 );     // ( 0, 0 )
        GXPosition3s16(  1200, -256, -400 );
        GXTexCoord2s16( 0x0000, 0x0800 );     // ( 0, 8 )
        GXPosition3s16(  1200, -256,  400 );
        GXTexCoord2s16( 0x0C00, 0x0800 );     // ( 12, 8 )
        GXPosition3s16( -1200, -256,  400 );
        GXTexCoord2s16( 0x0C00, 0x0000 );     // ( 12, 0 )
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawScreenPanel
    
    Description:    Draw a textured screen panel
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawScreenPanel( void )
{
    // Frame

    // set Tev operation to use vertex color
    GXSetNumTexGens( 0 );
	GXSetNumChans( 1 );
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);

    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        GXPosition3s16( -256,  256, -1 );
        GXColor1x8((u8)NUM_CUBES);        // Gray
        GXPosition3s16(  256,  256, -1 );
        GXColor1x8((u8)NUM_CUBES);        // Gray
        GXPosition3s16(  256, -256, -1 );
        GXColor1x8((u8)NUM_CUBES);        // Gray
        GXPosition3s16( -256, -256, -1 );
        GXColor1x8((u8)NUM_CUBES);        // Gray
    GXEnd();


    // Screen

    // set Tev operation to use texture with decal mode
    GXSetNumTexGens( 1 );
	GXSetNumChans( 1 );
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOp(GX_TEVSTAGE0, GX_DECAL);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        GXPosition3s16( -240,  240, 0 );
        GXColor4u8( 0, 0, 0, 255 );
        GXTexCoord2s16( 0x0000, 0x0000 );
        GXPosition3s16(  240,  240, 0 );
        GXColor4u8( 128, 0, 0, 255 );
        GXTexCoord2s16( 0x0100, 0x0000 );
        GXPosition3s16(  240, -240, 0 );
        GXColor4u8( 128, 128, 0, 255 );
        GXTexCoord2s16( 0x0100, 0x0100 );
        GXPosition3s16( -240, -240, 0 );
        GXColor4u8( 0, 128, 0, 255 );
        GXTexCoord2s16( 0x0000, 0x0100 );
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawCubes
    
    Description:    Draw cube models
                    
    Arguments:      view : view matrix
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawCubes( Mtx view )
{
    static u32 deg = 0;
    u32 i;
    Mtx mt, mr, ms, mv, mvi;
    Vec pos;

    // enable lighting
    SetLight();
    
    // set Tev operation to use vertex color
    GXSetNumTexGens( 0 );
	GXSetNumChans( 1 );
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    for ( i = 0 ; i < NUM_CUBES ; ++i )
    {
        pos = CubePosArray[i];
        MTXTrans(mt, pos.x, pos.y, pos.z);
        MTXConcat(view, mt, mv);
        MTXScale(ms, 100.0F, 100.0F, 100.0F);
        MTXConcat(mv, ms, mv);
        MTXRotDeg(mr, 'x', CubeDegArray[i]);
        MTXConcat(mv, mr, mv);
        MTXRotDeg(mr, 'z', (f32)deg);
        MTXConcat(mv, mr, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
        MTXInverse(mv, mvi); 
        MTXTranspose(mvi, mv);
        GXLoadNrmMtxImm(mv, GX_PNMTX0);
        
        GXSetChanMatColor(GX_COLOR0A0, ColorArray[i]);
        
        GXDrawCube();
    }

    // disable lighting
    DisableLight();

    // animation parameter
    deg += 5;
    if ( deg > 360 )
    {
        deg -= 360;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawTexCopyFrame
    
    Description:    Draw the frame of the texture copy area
                    
    Arguments:      ct : a pointer to MyCopyTexObj structure
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTexCopyFrame( MyCopyTexObj* ct )
{
    s16  x0, x1, y0, y1;

    // set Tev operation
    GXSetNumTexGens( 0 );
	GXSetNumChans( 1 );
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    x0 = (s16)( ct->left - 1 );
    x1 = (s16)( ct->left + ct->width );
    y0 = (s16)( ct->top - 1 );
    y1 = (s16)( ct->top + ct->height );

    // draw the box
    GXBegin(GX_LINESTRIP, GX_VTXFMT1, 5);
        GXPosition3s16(x0, y0, 0);
        GXColor4u8(255, 255, 255, 255);
        GXPosition3s16(x1, y0, 0);
        GXColor4u8(255, 255, 255, 255);
        GXPosition3s16(x1, y1, 0);
        GXColor4u8(255, 255, 255, 255);
        GXPosition3s16(x0, y1, 0);
        GXColor4u8(255, 255, 255, 255);
        GXPosition3s16(x0, y0, 0);
        GXColor4u8(255, 255, 255, 255);
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           CopyTextureFromFB
    
    Description:    Copies texture from the frame buffer image
                    
    Arguments:      ct : a pointer to MyCopyTexObj structure
 
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CopyTextureFromFB( MyCopyTexObj* ct )
{
    u32      size;
    u16      twidth, theight;
    GXTexFmt cpyFmt, useFmt;

    cpyFmt  = TexFormats[ct->format].cpyFmt;
    useFmt  = TexFormats[ct->format].useFmt;
    twidth  = (u16)ct->width;
    theight = (u16)ct->height;

    // If the box(mipmap) filter is used, actual width/height become half.
    if ( ct->mmFilter )
    {
        twidth  /= 2;
        theight /= 2;
    }

    // Allocate memory for copied texture
    if ( ct->data != NULL )
    {
        OSFree(ct->data);
    }
    size = GXGetTexBufferSize(
               twidth,
               theight,
               useFmt,
               GX_FALSE,
               0 );
    ct->data = OSAlloc(size);
    
    // Copy image
    GXSetTexCopySrc(
        (u16)ct->left,
        (u16)ct->top,
        (u16)ct->width,
        (u16)ct->height );
    GXSetTexCopyDst(twidth, theight, cpyFmt, ct->mmFilter );
    GXCopyTex(ct->data, ct->clear);

    // Initialize texture object
    GXInitTexObj(
        &ct->tobj,
        ct->data,
        twidth,
        theight,
        useFmt,
        GX_CLAMP,
        GX_CLAMP,
        GX_FALSE );
    GXInitTexObjLOD(
        &ct->tobj,
        GX_LINEAR,
        GX_LINEAR,
        0.0F,
        0.0F,
        0.0F,
        GX_FALSE,
        GX_FALSE,
        GX_ANISO_1 );
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
    Name:           SetScreenSpaceMode
    
    Description:    set projection matrix up to screen coordinate system
                    
    Arguments:      none
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetScreenSpaceMode( MySceneCtrlObj* sc )
{
    Mtx44  proj;

#ifdef EMU
    GXSetViewport(
        0.0F, 0.0F, sc->screen_width, sc->screen_height,
        0.0F, 1.0F);
    MTXOrtho( proj, 0.0F, sc->screen_height - 1, 0.0f, sc->screen_width - 1, 0.0f, - MAX_Z );
#else // !EMU ( real hardware )
    GXSetViewport(
        1.0F, 1.0F, sc->screen_width, sc->screen_height,
        0.0F, 1.0F);
    MTXOrtho( proj, 0.0F, sc->screen_height, 0.0f, sc->screen_width, 0.0f, - MAX_Z );
#endif // EMU

    GXSetProjection( proj, GX_ORTHOGRAPHIC );
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
    GXInitLightPos(&lobj, 0.0F, 0.0F, 10000.0F); // almost parallel
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
    OSReport("frb-copy: texture copy from EFB test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("  Main stick   : move the capture frame\n");
    OSReport("  Sub stick    : change scale of the capture frame\n");
    OSReport("  A/B button   : change texture format\n");
    OSReport("  Y button     : mipmap filter option on/off\n");
    OSReport("  X button     : clear option on/off\n");
    OSReport("  L/R triggers : move the camera\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
