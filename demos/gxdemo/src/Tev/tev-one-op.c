/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tev-one-op.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Tev/tev-one-op.c $
    
    14    6/12/01 8:38p Hirose
    got rid of tev-emu stuff because it is no longer necessary.
    fixed control interface etc.
    
    13    5/13/01 2:48p Hirose
    removed obsolete flags
    
    12    10/30/00 1:33p Carl
    Removed call to set clamp mode.
    
    11    10/04/00 4:54p Hirose
    A cache coherency fix. + replaced the use of "EPPC" by "EMU"
    
    10    7/07/00 5:57p Dante
    PC Compatibility
    
    9     5/20/00 11:26p Hirose
    
    8     5/17/00 8:46p Hirose
    changed parameter for Z mode
    
    7     5/02/00 4:04p Hirose
    updated to call DEMOGetCurrentBuffer instead of using direct
    access to CurrentBuffer defined in DEMOInit.c
    
    6     3/26/00 7:32p Hirose
    centered display object more considering H/V overscan again
    
    5     3/24/00 6:00p Carl
    Adjusted screen height for overscan.
    
    4     3/24/00 3:45p Hirose
    changed to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:12p Alligator
    move from tests/gx and rename
    
    12    2/24/00 8:12p Hirose
    updated pad control functions to match actual gamepad
    
    11    2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    10    2/15/00 3:53p Hirose
    small fix
    
    9     2/14/00 6:40p Hirose
    added dummy GXCopyDisp to clear framebuffer
    
    8     2/12/00 5:16p Alligator
    Integrate ArtX source tree changes
    
    7     2/09/00 7:21p Hirose
    added singleframe tests
    
    6     2/08/00 7:34p Hirose
    fixed some parts
    
    5     2/08/00 3:48p Carl
    Fixed spelling error; updated copyright message.
    
    4     2/07/00 7:50p Hirose
    made psuedo emulation code
    
    3     2/03/00 11:19p Hirose
    beta version (still under construction)
    
    2     2/02/00 11:31p Hirose
    changed to call tev-emu utility
    
    1     2/01/00 10:02p Hirose
    initial prototype
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tev-one-op
        one stage test with various color inputs and operations
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define MAX_Z           0x00FFFFFF

#define TEX_WIDTH       64
#define TEX_HEIGHT      64

#define PI              3.14159265F

#define NUM_TEVCLRARGS  16
#define NUM_TEVOPS      2
#define NUM_TEVBIASES   3
#define NUM_TEVSCALES   4
#define NUM_TEVCLRINS   4
#define NUM_TEVREGS     4
#define NUM_PARAMETERS  (NUM_TEVCLRINS+3)

#define NUM_PANELTEX    (NUM_TEVOPS+NUM_TEVBIASES+NUM_TEVSCALES+1)
#define NUM_SAMPLETEX   3
#define NUM_TEXTURES    (NUM_PANELTEX+NUM_SAMPLETEX)

#define TEX_FORMULA     0
#define TEX_OPS         1
#define TEX_BIASES      (TEX_OPS+NUM_TEVOPS)
#define TEX_SCALES      (TEX_BIASES+NUM_TEVBIASES)

#define NUM_LINES       22

#define DuplicateAlpha(dst, src) \
    ((dst).r = (dst).g = (dst).b = (dst).a = (src).a)

// for display location
#define ALI_COL(x)      (x * 80 + 48) 

/*---------------------------------------------------------------------------*
   Structure definitions
 *---------------------------------------------------------------------------*/
// for input arg display mode
typedef struct
{
    GXTevColorArg  arg;
    GXTevMode      mode;
    u8             ciOffset;
    u8             ciStep;
} MyColorArgDispObj;

// for tev state storage
typedef struct
{
    GXTexObj       panelTex[NUM_PANELTEX];
    u32            param[NUM_PARAMETERS];
    GXColor        regColor[NUM_TEVREGS];
} MyTevStateObj;

// for entire scene control
typedef struct
{
    u32            cur;
    GXTexObj       sampleTex[NUM_SAMPLETEX];
    u32            currentTex;
    GXColor        vtxColor[4];
    MyTevStateObj  tevState;
    u32            count;
    u16            screenWd;
    u16            screenHt;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main               ( void );
static void DrawInit           ( MySceneCtrlObj* sc );
static void DrawTick           ( MySceneCtrlObj* sc );
static void AnimTick           ( MySceneCtrlObj* sc );
static void DrawColorInputs    ( MyTevStateObj* ts );
static void DrawMisc           ( MyTevStateObj* ts );
static void DrawCaption        ( MySceneCtrlObj* sc );
static void DrawLines          ( void );
static void DrawQuad           ( u8 c_offset, u8 c_step );
static void DrawResult         ( GXTexObj* tx );
static void SetTev             ( MyTevStateObj* ts );
static void PrintIntro         ( void );

/*---------------------------------------------------------------------------*
   Model Data
 *---------------------------------------------------------------------------*/
static s16 ModelVertices[8+NUM_LINES*4] ATTRIBUTE_ALIGN(32) = 
{
    // Used for drawing quads
     0,  0, 64,  0,
    64, 64,  0, 64,
    
    // Used for drawing lines
     80, 112,  80, 170,
     80, 170, 120, 170,
    120, 170, 120, 176,
    
    160, 112, 160, 170,
    160, 170, 288, 170,
    184, 170, 184, 176,
    288, 170, 288, 176,
    
    240, 112, 240, 160,
    240, 160, 200, 160,
    200, 160, 200, 176,
    
    320, 112, 320, 176,
    
    400, 112, 400, 140,
    400, 140, 150, 140,
    150, 140, 150, 176,
    
    480, 112, 480, 146,
    480, 146, 380, 146,
    380, 146, 380, 176,
    
    560, 112, 560, 152,
    560, 152, 448, 152,
    448, 152, 448, 176,
    
    480, 208, 528, 208,
    528, 208, 520, 200
};

static s8 QuadTexCoords[2*4] ATTRIBUTE_ALIGN(32) =
{
      0,   0,
      1,   0,
      1,   1,
      0,   1
};

/*---------------------------------------------------------------------------*
   Color table
 *---------------------------------------------------------------------------*/
#define COLOR_VTX        0
#define COLOR_VTXA       4
#define COLOR_REG        8
#define COLOR_REGA       12
#define COLOR_ONE        16
#define COLOR_HALF       17
#define COLOR_QUARTER    18
#define COLOR_ZERO       19
#define COLOR_RED        20
#define COLOR_GREEN      21
#define COLOR_BLUE       22
#define COLOR_BG         23

static GXColor ColorArray[] ATTRIBUTE_ALIGN(32) =
{
    { 0x80, 0xFF, 0x80, 0x00 }, // Vertex colors (modified dinamically)
    { 0xFF, 0x80, 0x00, 0x80 },
    { 0x80, 0x00, 0x80, 0xFF },
    { 0x00, 0x80, 0xFF, 0x80 },
    { 0x00, 0x00, 0x00, 0x00 }, // Vertex alphas (modified dinamically)
    { 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00 },
    { 0xFF, 0x00, 0x00, 0x20 }, // Register colors (modified dinamically)
    { 0x00, 0xFF, 0x00, 0x60 },
    { 0x00, 0x00, 0xFF, 0xC0 },
    { 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00 }, // Register alphas (modified dinamically)
    { 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0xFF },
    { 0xFF, 0xFF, 0xFF, 0xFF }, // One
    { 0x80, 0x80, 0x80, 0x80 }, // Half
    { 0x40, 0x40, 0x40, 0x40 }, // Quarter
    { 0x00, 0x00, 0x00, 0x00 }, // Zero
    { 0xFF, 0x00, 0x00, 0x00 }, // Red
    { 0x00, 0xFF, 0x00, 0x00 }, // Green
    { 0x00, 0x00, 0xFF, 0x00 }, // Blue
    { 0x00, 0x00, 0x40, 0x00 }  // BG
};

/*---------------------------------------------------------------------------*
   Other data tables
 *---------------------------------------------------------------------------*/
// The table used to display each component
static MyColorArgDispObj ColorArgDispTbl[NUM_TEVCLRARGS] =
{
    // Argument,    Display mode, ci offset, ci step
    { GX_CC_C0,      GX_PASSCLR,  COLOR_REG,     0 },
    { GX_CC_C1,      GX_PASSCLR,  COLOR_REG+1,   0 },
    { GX_CC_C2,      GX_PASSCLR,  COLOR_REG+2,   0 },
    { GX_CC_CPREV,   GX_PASSCLR,  COLOR_REG+3,   0 },
    { GX_CC_A0,      GX_PASSCLR,  COLOR_REGA,    0 },
    { GX_CC_A1,      GX_PASSCLR,  COLOR_REGA+1,  0 },
    { GX_CC_A2,      GX_PASSCLR,  COLOR_REGA+2,  0 },
    { GX_CC_APREV,   GX_PASSCLR,  COLOR_REGA+3,  0 },
    { GX_CC_TEXC,    GX_REPLACE,  COLOR_ZERO,    0 },
    { GX_CC_TEXA,    GX_DECAL,    COLOR_ZERO,    0 },
    { GX_CC_RASC,    GX_PASSCLR,  COLOR_VTX,     1 },
    { GX_CC_RASA,    GX_PASSCLR,  COLOR_VTXA,    1 },
    { GX_CC_ONE,     GX_PASSCLR,  COLOR_ONE,     0 },
    { GX_CC_HALF,    GX_PASSCLR,  COLOR_HALF,    0 },
    { GX_CC_QUARTER, GX_PASSCLR,  COLOR_QUARTER, 0 },
    { GX_CC_ZERO,    GX_PASSCLR,  COLOR_ZERO,    0 }
};

static GXTevOp OpArgTbl[NUM_TEVOPS] =
{
    GX_TEV_ADD, GX_TEV_SUB
};

static GXTevBias BiasArgTbl[NUM_TEVBIASES] =
{
    GX_TB_ZERO, GX_TB_ADDHALF, GX_TB_SUBHALF
};

static GXTevScale ScaleArgTbl[NUM_TEVSCALES] =
{
    GX_CS_SCALE_1, GX_CS_SCALE_2, GX_CS_SCALE_4, GX_CS_DIVIDE_2
};

static u32 ParameterMax[NUM_PARAMETERS] =
{
    NUM_TEVCLRARGS, // Arg.D
    NUM_TEVCLRARGS, // Arg.C
    NUM_TEVCLRARGS, // Arg.B
    NUM_TEVCLRARGS, // Arg.A
    NUM_TEVOPS,     // Operation
    NUM_TEVBIASES,  // Bias
    NUM_TEVSCALES   // Scale
};

/*---------------------------------------------------------------------------*
   String data for messages
 *---------------------------------------------------------------------------*/
static char* ArgNameStr[NUM_PARAMETERS+1] =
{
    "Arg.D",  "Arg.C",  "Arg.B",  "Arg.A",
    "Op.",    "Bias",   "Scale",  "Result"
};

static char* ColorArgStr[NUM_TEVCLRARGS] =
{
    "C0",     "C1",     "C2",      "CPREV",
    "A0",     "A1",     "A2",      "APREV",
    "TEXC",   "TEXA",   "RASC",    "RASA",
    "ONE",    "HALF",   "QUARTER", "ZERO"
};

static char* OpArgStr[NUM_TEVOPS] =
{
    "ADD",  "SUB"
};

static char* BiasArgStr[NUM_TEVBIASES] =
{
    "ZERO", "ADDHALF", "SUBHALF"
};

static char* ScaleArgStr[NUM_TEVSCALES] =
{
    "SCALE_1", "SCALE_2", "SCALE_4", "DIVIDE_2"
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;          // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;    // texture palette

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

    OSHalt("End of demo");
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
    TEXDescriptorPtr  tdp;
    GXRenderModeObj*  rmode;
    u32               i;
    GXTexObj*         tobj;
    
    // Vertex Attribute (VTXFMT0 is used by DEMOPuts library)
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XY, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_S8, 0);

    // Array pointers
    GXSetArray(GX_VA_POS,  ModelVertices, 2 * sizeof(s16));
    GXSetArray(GX_VA_TEX0, QuadTexCoords, 2 * sizeof(s8));
    GXSetArray(GX_VA_CLR0, ColorArray, sizeof(GXColor));

    // Get Screen Information defined in DEMOInit()
    // This test is supposing height=480 (or near.)
    rmode = DEMOGetRenderModeObj();
    sc->screenWd = rmode->fbWidth;   // Screen Width
    sc->screenHt = rmode->efbHeight; // Screen Height

    // Background color
    GXSetCopyClear(ColorArray[COLOR_BG], MAX_Z);

    // Load TPL file and initialize texture objects
    TEXGetPalette(&MyTplObj, "gxTests/tev-00.tpl");
    for ( i = 0 ; i < NUM_TEXTURES ; ++i )
    {
        tdp = TEXGet(MyTplObj, i);
        
        if ( i < NUM_PANELTEX )
        {
            // Used as Tev status panel
            tobj = &sc->tevState.panelTex[i];
        }
        else
        {
            // Used as sample input texture
            tobj = &sc->sampleTex[i - NUM_PANELTEX];
        }
    
        GXInitTexObj(
            tobj,
            tdp->textureHeader->data,
            tdp->textureHeader->width,
            tdp->textureHeader->height,
            (GXTexFmt)tdp->textureHeader->format,
            tdp->textureHeader->wrapS, // s
            tdp->textureHeader->wrapT, // t
            GX_FALSE ); // Mipmap
        
        GXInitTexObjLOD(
            tobj,
            tdp->textureHeader->minFilter,
            tdp->textureHeader->magFilter,
            tdp->textureHeader->minLOD,
            tdp->textureHeader->maxLOD,
            tdp->textureHeader->LODBias,
            GX_FALSE,
            tdp->textureHeader->edgeLODEnable,
            GX_ANISO_1 );
    }
    

    // Default scene control parameter settings

    // tev status
    for ( i = 0 ; i < NUM_TEVCLRINS ; ++i )
    {
        sc->tevState.param[i] = 0;
    }
    
    // initial tev register color
    for ( i = 0 ; i < NUM_TEVREGS ; ++i )
    {
        sc->tevState.regColor[i] = ColorArray[COLOR_REG+i];
    }

    // initial vertex color (used for singleframe)
    for ( i = 0 ; i < 4 ; ++i )
    {
        sc->vtxColor[i] = ColorArray[COLOR_VTX+i];
    }
    
    // current texture number
    sc->currentTex = 0;
    
    // a counter for vertex color animation
    sc->count = 0;

    // cursor
    sc->cur   = 0;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    u32  i;

    // set projection to match screen space coordinate system
    DEMOSetupScrnSpc(sc->screenWd, sc->screenHt, 100.0F);
    
    // set up Z mode (which actually doesn't matter)
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
    
    // reflect the newest color data
    GXInvalidateVtxCache();
    for ( i = 0 ; i < NUM_TEVREGS ; ++i )
    {
        ColorArray[COLOR_REG+i] = sc->tevState.regColor[i];
        DuplicateAlpha(ColorArray[COLOR_REGA+i], sc->tevState.regColor[i]);
    }
    for ( i = 0 ; i < 4 ; ++i )
    {
        ColorArray[COLOR_VTX+i] = sc->vtxColor[i];
        DuplicateAlpha(ColorArray[COLOR_VTXA+i], sc->vtxColor[i]);
    }
   
    // set up Tev rendering mode
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXSetNumTexGens(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    // Draw quads that show color-inputs
    GXLoadTexObj(&sc->sampleTex[sc->currentTex], GX_TEXMAP0);
    DrawColorInputs(&sc->tevState);
    
    // Draw a result quad
    SetTev(&sc->tevState);
    DrawResult(&sc->sampleTex[sc->currentTex]);
    
    // Draw miscellaneous status
    DrawMisc(&sc->tevState);
    
    // Connection lines
    DrawLines();

    // Captions
    DrawCaption(sc);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    u32  i, max;
    f32  theta;
    
    u16  down  = DEMOPadGetButtonDown(0);
    u16  stick = DEMOPadGetDirsNew(0);

    // vertex color animation
    sc->count = ( sc->count + 4 ) % 360;
    for ( i = 0 ; i < 4 ; ++i )
    {
        theta = ( sc->count + i * 90 ) * PI / 180.0F;
        sc->vtxColor[i].r = (u8)( sinf(theta) * 127.0F + 128.0F);
        sc->vtxColor[i].g = (u8)( cosf(theta) * 127.0F + 128.0F);
        sc->vtxColor[i].b = (u8)(-sinf(theta) * 127.0F + 128.0F);
        sc->vtxColor[i].a = (u8)(-cosf(theta) * 127.0F + 128.0F);
    }

    // move cursor
    if ( ( down & PAD_BUTTON_RIGHT ) || ( stick & DEMO_STICK_RIGHT ) )
    {
        sc->cur += 1;
    }
    if ( ( down & PAD_BUTTON_LEFT ) || ( stick & DEMO_STICK_LEFT ) )
    {
        sc->cur += NUM_PARAMETERS - 1;
    }
    sc->cur %= NUM_PARAMETERS;

    // change parameter
    max = ParameterMax[sc->cur];
    if ( down & PAD_BUTTON_X )
    {
        sc->tevState.param[sc->cur] += 1;
    }
    if ( down & PAD_BUTTON_Y )
    {
        sc->tevState.param[sc->cur] += max - 1;
    }
    sc->tevState.param[sc->cur] %= max;


    // change texture/register color
    if ( down & PAD_TRIGGER_R )
    {
        i = sc->tevState.param[sc->cur];
        
        // change texture
        if ( i == 8 || i == 9 || i >= 16 )
        {
            ++(sc->currentTex);
            sc->currentTex %= NUM_SAMPLETEX;
        }
        // change register value (color)
        if ( i >= 0 && i <= 3 )
        {
            sc->tevState.regColor[i].b ^= 0xFF;
            if ( sc->tevState.regColor[i].b == 0 )
            {
                sc->tevState.regColor[i].g ^= 0xFF;
                if ( sc->tevState.regColor[i].g == 0 )
                {
                    sc->tevState.regColor[i].r ^= 0xFF;
                }
            }
        }
        // change register value (alpha)
        if ( i >= 4 && i <= 7 )
        {
            sc->tevState.regColor[i%4].a += 0x20;
        }
    }

    // make sure cache-main memory coherency
    DCStoreRange(&ColorArray[0], 16 * sizeof(GXColor));
}

/*---------------------------------------------------------------------------*
    Name:           DrawColorInputs
    
    Description:    Draw quads that show color input source arguments
    
    Arguments:      ts : pointer to a tev state structure

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawColorInputs( MyTevStateObj* ts )
{
    Mtx  mt;
    u32  i, j;
    u8   offset, step;

    for ( i = 0 ; i < NUM_TEVCLRINS ; ++i )
    {
        // modelview matrix
        MTXTrans(mt, (f32)ALI_COL(i), 48.0F, 0.0F);
        GXLoadPosMtxImm(mt, GX_PNMTX0);
        
        j      = ts->param[i];
        offset = ColorArgDispTbl[j].ciOffset;
        step   = ColorArgDispTbl[j].ciStep;
        GXSetTevOp(GX_TEVSTAGE0, ColorArgDispTbl[j].mode);
       
        DrawQuad(offset, step);
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawMisc
    
    Description:    Draw miscellaneous status panels
    
    Arguments:      ts : pointer to a tev state structure

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawMisc( MyTevStateObj* ts )
{
    Mtx  mt, ms;

    // Use only texture
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);

    // Tev formula panel
    MTXScale(ms, 6.0F, 1.0F, 1.0F);
    MTXTrans(mt, ALI_COL(0)+48, 176, 0.0F);
    MTXConcat(mt, ms, mt); 
    GXLoadPosMtxImm(mt, GX_PNMTX0);
    GXLoadTexObj(&ts->panelTex[TEX_FORMULA], GX_TEXMAP0);
    DrawQuad(COLOR_ZERO, 0);

    // Operation panel
    MTXTrans(mt, ALI_COL(4), 48.0F, 0.0F);
    GXLoadPosMtxImm(mt, GX_PNMTX0);
    GXLoadTexObj(&ts->panelTex[TEX_OPS + ts->param[4]], GX_TEXMAP0);
    DrawQuad(COLOR_ZERO, 0);
    
    // Bias panel
    MTXTrans(mt, ALI_COL(5), 48.0F, 0.0F);
    GXLoadPosMtxImm(mt, GX_PNMTX0);
    GXLoadTexObj(&ts->panelTex[TEX_BIASES + ts->param[5]], GX_TEXMAP0);
    DrawQuad(COLOR_ZERO, 0);
    
    // Scale panel
    MTXTrans(mt, ALI_COL(6), 48.0F, 0.0F);
    GXLoadPosMtxImm(mt, GX_PNMTX0);
    GXLoadTexObj(&ts->panelTex[TEX_SCALES + ts->param[6]], GX_TEXMAP0);
    DrawQuad(COLOR_ZERO, 0);

}

/*---------------------------------------------------------------------------*
    Name:           DrawCaption
    
    Description:    Draw captions
    
    Arguments:      sc : pointer to the structure of scene control parameters

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawCaption( MySceneCtrlObj* sc )
{
    u32  i, j;
    char* str;

    DEMOInitCaption(DM_FT_OPQ, sc->screenWd, sc->screenHt);

    // Argument names
    for ( i = 0 ; i < NUM_PARAMETERS ; ++i )
    {
        DEMOPuts((s16)ALI_COL(i), 32, 0, ArgNameStr[i]);
    }
    DEMOPuts(ALI_COL(6), 160, 0, ArgNameStr[NUM_PARAMETERS]); // Result

    // Tev stage color input arguments
    for ( i = 0 ; i < NUM_PARAMETERS ; ++i )
    {
        j = sc->tevState.param[i];
        
        if ( i < NUM_TEVCLRINS )
            str = ColorArgStr[j];
        else if ( i == NUM_TEVCLRINS )
            str = OpArgStr[j];
        else if ( i == NUM_TEVCLRINS + 1 )
            str = BiasArgStr[j];
        else if ( i == NUM_TEVCLRINS + 2 )
            str = ScaleArgStr[j];

        DEMOPuts((s16)(i*80+48), 120, 0, str);
   }
    
    // Cursor
    DEMOPrintf((s16)(ALI_COL(sc->cur)-8), 32, 0, "%c", 0x7F);
}

/*---------------------------------------------------------------------------*
    Name:           DrawLines
    
    Description:    Draw connection lines
    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawLines( void )
{
    u32  i;
    Mtx  mt;

    // use only vertex color
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    // reset modelview matrix
    MTXIdentity(mt);
    GXLoadPosMtxImm(mt, GX_PNMTX0);

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);

    // draw the quad
    GXBegin(GX_LINES, GX_VTXFMT1, NUM_LINES*2);
        for ( i = 0 ; i < NUM_LINES*2 ; ++i )
        {
            GXPosition1x8((u8)(i+4));
            GXColor1x8(COLOR_ONE);
        }
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawQuad
    
    Description:    Draw a textured & colored Quad. Color index lookup
                    values for each vertex can be specified by arguments.
    
    Arguments:      c_offset : offset value added to color index
                    c_step   : step value added to color index per vertex

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawQuad( u8 c_offset, u8 c_step )
{
    u8   i, ci;
    
    ci = c_offset;

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);

    // draw the quad
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        for ( i = 0 ; i < 4 ; ++i )
        {
            GXPosition1x8(i);
            GXColor1x8(ci);
            GXTexCoord1x8(i);
            
            ci += c_step;
        }
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawResult
    
    Description:    Draw a quad which shows color combination result
    
    Arguments:      tx : pointer to a texture object to be used

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawResult( GXTexObj* tx )
{
    Mtx  mt;

    // Modelview matrix
    MTXTrans(mt, ALI_COL(6), 176.0F, 0.0F);
    GXLoadPosMtxImm(mt, GX_PNMTX0);

    // Draw a quad by using actual vertex color
    GXLoadTexObj(tx, GX_TEXMAP0);
    DrawQuad(COLOR_VTX, 1);
}

/*---------------------------------------------------------------------------*
    Name:           SetTev
    
    Description:    Set up TEV color operation.
    
    Arguments:      ts : pointer to a tev state structure

    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetTev( MyTevStateObj* ts )
{
    // Detailed Tev settings
    GXSetNumTevStages(1);
    GXSetTevColor(GX_TEVREG0, ts->regColor[0]);
    GXSetTevColor(GX_TEVREG1, ts->regColor[1]);
    GXSetTevColor(GX_TEVREG2, ts->regColor[2]);
    GXSetTevColor(GX_TEVPREV, ts->regColor[3]);
    
    GXSetTevColorIn(
        GX_TEVSTAGE0,
        ColorArgDispTbl[ts->param[3]].arg,    // a
        ColorArgDispTbl[ts->param[2]].arg,    // b
        ColorArgDispTbl[ts->param[1]].arg,    // c
        ColorArgDispTbl[ts->param[0]].arg );  // d
    
    GXSetTevColorOp(
        GX_TEVSTAGE0,
        OpArgTbl[ts->param[4]],    // Op (Add/Sub)
        BiasArgTbl[ts->param[5]],  // Bias
        ScaleArgTbl[ts->param[6]], // Scale
        GX_TRUE,                   // Clamp
        GX_TEVPREV );              // Output
    
    // GXSetTevAlphaIn();
    // GXSetTevAlphaOp();
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
    OSReport("tev-one-op: one stage test with various inputs\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Stick/D-PAD  : move the cursor\n");
    OSReport("X/Y buttons  : change parameters\n");
    OSReport("R trigger    : change texture (if selected)\n");
    OSReport("             : change reg. color (if selected)\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
