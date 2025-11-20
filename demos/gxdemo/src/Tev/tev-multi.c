/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tev-multi.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Tev/tev-multi.c $
    
    15    6/12/01 8:37p Hirose
    got rid of tev-emu stuff because it is no longer necessary.
    
    14    5/13/01 2:48p Hirose
    removed obsolete flags
    
    13    10/30/00 4:18p Hirose
    flag fix MAC -> EMU
    
    12    7/07/00 5:57p Dante
    PC Compatibility
    
    11    6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    10    5/17/00 8:46p Hirose
    changed parameter for Z mode
    
    9     3/26/00 7:32p Hirose
    centered display object more considering H/V overscan again
    
    8     3/24/00 6:00p Carl
    Adjusted screen height for overscan.
    
    7     3/24/00 4:22p Hirose
    deleted unnecessary macro definitions
    
    6     3/24/00 3:45p Hirose
    changed to use DEMOPad library
    
    5     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    4     3/20/00 7:50p Hirose
    changed texture filter
    
    3     3/17/00 2:16p Hirose
    
    2     3/15/00 5:24p Hirose
    small changes
    
    1     3/07/00 7:14p Hirose
    initial version
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tev-multi
        multiple TEV stages and ordering test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <math.h>
#include <demo.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define TEX_WIDTH       64
#define TEX_HEIGHT      64

#define NUM_TEVSTAGES   8
#define NUM_TEXCOORDS   8
#define NUM_TEXMAPS     8
#define NUM_CHANNELS    2
#define NUM_TEVMODES    5
#define NUM_COLORS      9
#define NUM_CTRLS       4

#define CTRL_TM         0  // TEXMAP
#define CTRL_TC         1  // TEXCOORD
#define CTRL_CH         2  // CHANNEL
#define CTRL_MD         3  // TEVMODE

// for display location
#define ALI_ROW(y)      (y * 96 + 64)
#define ALI_COL(x)      (x * 68 + 48) 

#define PI              3.14159265F

/*---------------------------------------------------------------------------*
   Structure definitions
 *---------------------------------------------------------------------------*/
// for tev ordering and operation control
typedef u8 MyTevCtrlObj[NUM_CTRLS][NUM_TEVSTAGES];

// for entire scene control
typedef struct
{
    u32           curX;
    u32           curY;
    GXTexObj      texture[NUM_TEXMAPS];
    Mtx           texgen[NUM_TEXCOORDS];
    u8            color[NUM_CHANNELS];
    MyTevCtrlObj  tev;
    u32           counter;
    u16           screenWd;
    u16           screenHt;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main               ( void );
static void DrawInit           ( MySceneCtrlObj* sc );
static void DrawTick           ( MySceneCtrlObj* sc );
static void AnimTick           ( MySceneCtrlObj* sc );
static void DrawQuad           ( void );
static void DrawLines          ( void );
static void DrawTexMapPanels   ( MyTevCtrlObj tevc );
static void DrawTexCoordPanels ( MyTevCtrlObj tevc );
static void DrawColorChanPanels( MyTevCtrlObj tevc );
static void DrawResultPanels   ( MyTevCtrlObj tevc );
static void DrawCaption        ( MySceneCtrlObj* sc );
static void AnimTexMtx         ( Mtx* tg, u32 cnt );
static void PrintIntro         ( void );

/*---------------------------------------------------------------------------*
   Model Data
 *---------------------------------------------------------------------------*/
static s16 ModelVertices[8] ATTRIBUTE_ALIGN(32) = 
{
    // Used for drawing quads
     0,  0, 64,  0,
    64, 64,  0, 64,
};

static s8 QuadTexCoords[2*4] ATTRIBUTE_ALIGN(32) =
{
      0,   0,
      1,   0,
      1,   1,
      0,   1
};

/*---------------------------------------------------------------------------*
   Constant color data for each channel
 *---------------------------------------------------------------------------*/
static GXColor ColorTbl[NUM_COLORS] =
{
    { 255, 255,   0, 255 }, // Yellow
    {   0, 255, 255, 255 }, // Cyan
    { 255,   0, 255, 255 }, // Magenta
    { 255,   0,   0, 255 }, // Red
    {   0, 255,   0, 255 }, // Green
    {   0,   0, 255, 255 }, // Blue
    {   0,   0,   0, 255 }, // Black
    { 128, 128, 128, 255 }, // Gray
    { 255, 255, 255, 255 }  // White
};

/*---------------------------------------------------------------------------*
   Other data tables
 *---------------------------------------------------------------------------*/
static GXTevStageID StageIDTbl[NUM_TEVSTAGES] =
{
    GX_TEVSTAGE0, GX_TEVSTAGE1, GX_TEVSTAGE2, GX_TEVSTAGE3,
    GX_TEVSTAGE4, GX_TEVSTAGE5, GX_TEVSTAGE6, GX_TEVSTAGE7
};

static GXTexCoordID CoordIDTbl[NUM_TEXCOORDS] =
{
    GX_TEXCOORD0, GX_TEXCOORD1, GX_TEXCOORD2, GX_TEXCOORD3,
    GX_TEXCOORD4, GX_TEXCOORD5, GX_TEXCOORD6, GX_TEXCOORD7
};

static GXTexMtx MtxIDTbl[NUM_TEXCOORDS] =
{
    GX_TEXMTX0, GX_TEXMTX1, GX_TEXMTX2, GX_TEXMTX3,
    GX_TEXMTX4, GX_TEXMTX5, GX_TEXMTX6, GX_TEXMTX7
};

static GXTexMapID MapIDTbl[NUM_TEXMAPS] =
{
    GX_TEXMAP0, GX_TEXMAP1, GX_TEXMAP2, GX_TEXMAP3,
    GX_TEXMAP4, GX_TEXMAP5, GX_TEXMAP6, GX_TEXMAP7
};

static GXChannelID ChannelIDTbl[NUM_CHANNELS] =
{
    GX_COLOR0A0, GX_COLOR1A1
};

static GXTevMode TevModeTbl[NUM_TEVMODES] =
{
    GX_PASSCLR, GX_REPLACE, GX_DECAL, GX_MODULATE, GX_BLEND
};

// Max numbers of each control parameter
static u8 MaxTbl[NUM_CTRLS] =
{
    NUM_TEXMAPS, NUM_TEXCOORDS, NUM_CHANNELS, NUM_TEVMODES
};

/*---------------------------------------------------------------------------*
   Strings data for captions
 *---------------------------------------------------------------------------*/
static char* TevModeStr[NUM_TEVMODES] =
{
    "PASSCLR", "REPLACE", "DECAL", "MODUL.", "BLEND"
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;        // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;  // texture palette

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
    u32               i, n;
    
    // Vertex Attribute (VTXFMT0 is used by DEMOPuts library)
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XY, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_S8, 0);

    // Array pointers
    GXSetArray(GX_VA_POS,  ModelVertices, 2 * sizeof(s16));
    GXSetArray(GX_VA_TEX0, QuadTexCoords, 2 * sizeof(s8));

    // Get Screen Information defined in DEMOInit()
    // This test is supposing height=480 (or near.)
    rmode = DEMOGetRenderModeObj();
    sc->screenWd = rmode->fbWidth;   // Screen Width
    sc->screenHt = rmode->efbHeight; // Screen Height

    // Load TPL file and initialize texture objects
    TEXGetPalette(&MyTplObj, "gxTests/tev-02.tpl");
    n = MyTplObj->numDescriptors;
    for ( i = 0 ; i < NUM_TEXMAPS ; ++i )
    {
        tdp = TEXGet(MyTplObj, i % n);
        
        GXInitTexObj(
            &sc->texture[i],
            tdp->textureHeader->data,
            tdp->textureHeader->width,
            tdp->textureHeader->height,
            (GXTexFmt)tdp->textureHeader->format,
            tdp->textureHeader->wrapS, // s
            tdp->textureHeader->wrapT, // t
            GX_FALSE ); // Mipmap
        
        GXInitTexObjLOD(
            &sc->texture[i],
            GX_LINEAR,
            GX_LINEAR,
            tdp->textureHeader->minLOD,
            tdp->textureHeader->maxLOD,
            tdp->textureHeader->LODBias,
            GX_FALSE,
            tdp->textureHeader->edgeLODEnable,
            GX_ANISO_1 );
    }
    // Color channel
    for ( i = 0 ; i < NUM_CHANNELS ; ++i )
    {
         GXSetChanCtrl(
            ChannelIDTbl[i],
            GX_DISABLE,    // Lighting off
            GX_SRC_REG,    // Ambient source (N/A)
            GX_SRC_REG,    // Material (Vertex color) source
            GX_LIGHT_NULL, // Light mask (N/A)
            GX_DF_NONE,    // Diffuse function (N/A)
            GX_AF_NONE );  // Attenuation (N/A)
    }

    
    // Default scene control parameter settings

    // initialize all texgen matrices
    for ( i = 0 ; i < NUM_TEXCOORDS ; ++i )
    {
        MTXIdentity(sc->texgen[i]);
    }

    // color number for each channel
    for ( i = 0 ; i < NUM_CHANNELS ; ++i )
    {
        sc->color[i] = (u8)i;
    } 

    // cursor
    sc->curX = sc->curY = 0;
    
    // counter for animation
    sc->counter = 0;

    // initialize all ordering and operation settings
    for ( i = 0 ; i < NUM_TEVSTAGES ; ++i )
    {
        sc->tev[CTRL_TM][i] = (u8)i;                  // TEXMAP
        sc->tev[CTRL_TC][i] = (u8)i;                  // TEXCOORD
        sc->tev[CTRL_CH][i] = (u8)(i % NUM_CHANNELS); // CHANNEL
        sc->tev[CTRL_MD][i] = 2;                      // MODE(OP) = DECAL
    }
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
    GXSetZMode(GX_ENABLE, GX_ALWAYS, GX_ENABLE);

    // draw lines
    DrawLines();

    // set up texcoord generators
    for ( i = 0 ; i < NUM_TEXCOORDS ; ++i )
    {
        GXLoadTexMtxImm(sc->texgen[i], MtxIDTbl[i], GX_MTX2x4);
        GXSetTexCoordGen(CoordIDTbl[i], GX_TG_MTX2x4, GX_TG_TEX0, MtxIDTbl[i]);
    }

    // set up texmaps
    for ( i = 0 ; i < NUM_TEXMAPS ; ++i )
    {
        GXLoadTexObj(&sc->texture[i], MapIDTbl[i]);
    }

    // set up color channels
    for ( i = 0 ; i < NUM_CHANNELS ; ++i )
    {
        GXSetChanMatColor(ChannelIDTbl[i], ColorTbl[sc->color[i]]);
    }

    // draw first row (texmap panels)
    DrawTexMapPanels(sc->tev);
    
    // draw second row (texcoord panels)
    DrawTexCoordPanels(sc->tev);

    // draw third row (color channel panels)
    DrawColorChanPanels(sc->tev);

    // draw final row (TEV combined result)
    DrawResultPanels(sc->tev);
    
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
    u16  button, down, dirs;

    // Texgen matrices animation
    AnimTexMtx(sc->texgen, sc->counter);

    // PAD
    down   = DEMOPadGetButtonDown(0);
    button = DEMOPadGetButton(0);
    dirs   = DEMOPadGetDirsNew(0);

    // move cursor
    if ( dirs & DEMO_STICK_RIGHT )
    {
        sc->curX += 1;
    }
    if ( dirs & DEMO_STICK_LEFT )
    {
        sc->curX += NUM_TEVSTAGES - 1;
    }
    if ( dirs & DEMO_STICK_DOWN )
    {
        sc->curY += 1;
    }
    if ( dirs & DEMO_STICK_UP )
    {
        sc->curY += 3;
    }
    sc->curX %= NUM_TEVSTAGES;
    sc->curY %= 4;

    // change parameter
    if ( down & PAD_BUTTON_X )
    {
        sc->tev[sc->curY][sc->curX] += 1;
        sc->tev[sc->curY][sc->curX] %= MaxTbl[sc->curY];
    }
    if ( down & PAD_BUTTON_Y )
    {
        sc->tev[sc->curY][sc->curX] += MaxTbl[sc->curY] - 1;
        sc->tev[sc->curY][sc->curX] %= MaxTbl[sc->curY];
    }

    // select constant color for a channel
    if ( ( down & PAD_BUTTON_B ) && sc->curY == CTRL_CH )
    {
        u8  ch = sc->tev[CTRL_CH][sc->curX];
        
        sc->color[ch] += 1;
        sc->color[ch] %= NUM_COLORS;
    }

    // freeze animation
    if (!(button & PAD_BUTTON_A))
    {
        ++sc->counter;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawQuad
    
    Description:    Draw a textured quad.
    
    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawQuad( void )
{
    u8   i;

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);

    // draw the quad
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        for ( i = 0 ; i < 4 ; ++i )
        {
            GXPosition1x8(i);
            GXTexCoord1x8(i);
        }
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawLines
    
    Description:    Draw lines
    
    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawLines( void )
{
    GXColor lineColor = { 192, 192, 192, 0 };
    u8   i;
    Mtx  mv;

    MTXIdentity(mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    // TEV settings
    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    GXSetNumChans(1);
    GXSetChanMatColor(GX_COLOR0A0, lineColor);

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    // draw lines
    GXBegin(GX_LINES, GX_VTXFMT1, NUM_TEVSTAGES * 6);
        for ( i = 0 ; i < NUM_TEVSTAGES ; ++i )
        {
            GXPosition2s16((s16)(ALI_COL(i)+32), ALI_ROW(0));
            GXPosition2s16((s16)(ALI_COL(i)+32), ALI_ROW(3));
            
            GXPosition2s16((s16)(ALI_COL(i)+64), (s16)(ALI_ROW(3)+30));
            GXPosition2s16((s16)(ALI_COL(i)+67), (s16)(ALI_ROW(3)+32));
            
            GXPosition2s16((s16)(ALI_COL(i)+64), (s16)(ALI_ROW(3)+34));
            GXPosition2s16((s16)(ALI_COL(i)+67), (s16)(ALI_ROW(3)+32));
        }
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawTexMapPanels
    
    Description:    Draw quads for the first row (TEXMAP)
    
    Arguments:      tevc : pointer for tev control status object

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTexMapPanels( MyTevCtrlObj tevc )
{
    Mtx  mt;
    u32  x;

    GXSetNumTevStages(1);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);

    for ( x = 0 ; x < NUM_TEVSTAGES ; ++x )
    {
        // Code for actual HW
        GXSetTevOrder(
            GX_TEVSTAGE0,
            GX_TEXCOORD0,    // always identity
            MapIDTbl[tevc[CTRL_TM][x]],
            GX_COLOR_NULL );
        
        MTXTrans(mt, (f32)ALI_COL(x), (f32)ALI_ROW(0), 0);
        GXLoadPosMtxImm(mt, GX_PNMTX0);
        DrawQuad();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawTexCoordPanels
    
    Description:    Draw quads for the second row (TEXCOORD)
    
    Arguments:      tevc : pointer for tev control status object

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTexCoordPanels( MyTevCtrlObj tevc )
{
    Mtx  mt;
    u32  x;

    GXSetNumTevStages(1);
    GXSetNumChans(0);
    GXSetNumTexGens(NUM_TEXCOORDS);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);

    for ( x = 0 ; x < NUM_TEVSTAGES ; ++x )
    {
        GXSetTevOrder(
            GX_TEVSTAGE0,
            CoordIDTbl[tevc[CTRL_TC][x]],
            MapIDTbl[tevc[CTRL_TM][x]],
            GX_COLOR_NULL );
        
        MTXTrans(mt, (f32)ALI_COL(x), (f32)ALI_ROW(1), 0);
        GXLoadPosMtxImm(mt, GX_PNMTX0);
        DrawQuad();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawColorChanPanels
    
    Description:    Draw quads for the third row (COLOR CHANNEL)
    
    Arguments:      tevc : pointer for tev control status object

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawColorChanPanels( MyTevCtrlObj tevc )
{
    Mtx  mt;
    u32  x;

    GXSetNumTevStages(1);
    GXSetNumChans(NUM_CHANNELS);
    GXSetNumTexGens(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    for ( x = 0 ; x < NUM_TEVSTAGES ; ++x )
    {
        GXSetTevOrder(
            GX_TEVSTAGE0,
            GX_TEXCOORD_NULL,
            GX_TEXMAP_NULL,
            ChannelIDTbl[tevc[CTRL_CH][x]] );
        
        MTXTrans(mt, (f32)ALI_COL(x), (f32)ALI_ROW(2), 0);
        GXLoadPosMtxImm(mt, GX_PNMTX0);
        DrawQuad();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawResultPanels
    
    Description:    Draw quads for the final row (TEV combined result)
    
    Arguments:      tevc : pointer for tev control status object

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawResultPanels( MyTevCtrlObj tevc )
{
    Mtx  mt;
    u32  x;

    GXSetNumChans(NUM_CHANNELS);
    GXSetNumTexGens(NUM_TEXCOORDS);

    for ( x = 0 ; x < NUM_TEVSTAGES ; ++x )
    {
        GXSetNumTevStages((u8)(x+1));
        GXSetTevOrder(
            StageIDTbl[x],
            CoordIDTbl[tevc[CTRL_TC][x]],
            MapIDTbl[tevc[CTRL_TM][x]],
            ChannelIDTbl[tevc[CTRL_CH][x]] );
        GXSetTevOp(StageIDTbl[x], TevModeTbl[tevc[CTRL_MD][x]]);
        
        MTXTrans(mt, (f32)ALI_COL(x), (f32)ALI_ROW(3), 0);
        GXLoadPosMtxImm(mt, GX_PNMTX0);
        DrawQuad();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawCaption
    
    Description:    Draw captions
    
    Arguments:      sc : pointer to the structure of scene control parameters

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawCaption( MySceneCtrlObj* sc )
{
    u32  i;
    s16  x;

    DEMOInitCaption(DM_FT_OPQ, sc->screenWd, sc->screenHt);

    for ( i = 0 ; i < NUM_TEVSTAGES ; ++i )
    {
        x = (s16)ALI_COL(i);
        DEMOPrintf(x, (s16)(ALI_ROW(0)-16), 0, "[STAGE%d]", i);
        x += 8;
        DEMOPrintf(x, (s16)(ALI_ROW(0)+72), 0, "TEXMAP%d", sc->tev[CTRL_TM][i]);
        DEMOPrintf(x, (s16)(ALI_ROW(1)+72), 0, "TCOORD%d", sc->tev[CTRL_TC][i]);
        DEMOPrintf(x, (s16)(ALI_ROW(2)+72), 0, "COLOR%d", sc->tev[CTRL_CH][i]);
        DEMOPuts(x, (s16)(ALI_ROW(3)+72), 0, TevModeStr[sc->tev[CTRL_MD][i]]);
    }
    
    // Cursor
    DEMOPrintf(
        (s16)ALI_COL(sc->curX),
        (s16)(ALI_ROW(sc->curY) + 72),
        0,
        "%c",
        0x7F );
}

/*---------------------------------------------------------------------------*
    Name:           AnimTexMtx
    
    Description:    Generates animating texcoord generator matrices.
                    
    Arguments:      tg  : pointer to texgen matrices array
                    cnt : counter for animation
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTexMtx( Mtx* tg, u32 cnt )
{
    Mtx  m0, m1, m2;
    f32  st, ss;
    
    // Matrix 0 is always identity.
    MTXIdentity(tg[0]);
    
    // Matrix 1
    st = (f32)( cnt % 200 ) / 200.0F;
    MTXTrans(tg[1], st, st, 0);

    // Matrix 2
    st = (f32)( cnt % 90 ) * 4.0F;
    MTXTrans(m0, -0.5F, -0.5F, 0.0F);
    MTXRotDeg(m1, 'z', st);
    MTXConcat(m1, m0, m2);
    MTXTrans(m0, 0.5F, 0.5F, 0.0F);
    MTXConcat(m0, m2, tg[2]);

    // Matrix 3
    st = (f32)( cnt % 200 ) / 200.0F;
    MTXTrans(m0, st, 0.0F, 0.0F);
    MTXScale(m1, -1.0F, 2.0F, 1.0F);
    MTXConcat(m0, m1, tg[3]);

    // Matrix 4
    st = (f32)( cnt % 360 ) * PI / 180.0F;
    ss = ( sinf(st) + 1.0F ) * 1.5F + 1.0F;
    MTXScale(tg[4], ss, ss, 1.0F);

    // Matrix 5
    st = (f32)( cnt % 180 ) * PI / 90.0F;
    ss = ( sinf(st) + 1.0F ) * 2.0F + 0.5F;
    MTXRotDeg(m1, 'z', 45);
    MTXScale(m2, 1.0F, ss, 1.0F);
    MTXConcat(m2, m1, m0);
    MTXConcat(m1, m0, m2);
    MTXTrans(m0, 0.5F, 0.5F, 0.0F);
    MTXConcat(m0, m2, tg[5]);
    
    // Matrix 6
    st = (f32)( cnt % 360 );
    MTXTrans(m0, -1.0F, -1.0F, 0.0F);
    MTXScale(m1, 2.0F, 2.0F, 0.0F);
    MTXConcat(m1, m0, m2);
    MTXRotDeg(m1, 'z', -st);
    MTXConcat(m1, m2, m0);
    MTXTrans(m2, 1.0F, 1.0F, 0.0F);
    MTXConcat(m2, m0, tg[6]);

    // Matrix 7
    st = (f32)( cnt % 360 );
    MTXRotDeg(m1, 'z', st);
    st = st * PI / 180.0F;
    ss = ( sinf(st) + 1.5F ) * 0.5F;
    MTXScale(m0, ss, 1.0F, 1.0F);
    MTXConcat(m1, m0, m2);
    MTXTrans(m0, ss, -ss, 0.0F);
    MTXConcat(m0, m2, tg[7]);
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
    OSReport("tev-multi: multiple TEV stages test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick  : move the cursor\n");
    OSReport("X/Y Buttons : change parameter\n");
    OSReport("A Button    : freeze animation\n"); 
    OSReport("B Button    : change channel color (if selected)\n"); 
    OSReport("************************************************\n\n");
}

/*============================================================================*/
