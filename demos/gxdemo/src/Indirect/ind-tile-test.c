/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     ind-tile-test.c

  Copyright 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Indirect/ind-tile-test.c $
    
    1     11/20/01 5:58p Matthew
    
    11    5/13/01 1:45a Hirose
    removed obsolete flags
    
    10    4/09/01 7:40p Hirose
    fixed matrix type for projection API (should be Mtx44)
    
    9     11/28/00 9:39p Hirose
    Update due to DEMOStat library enhancement
    
    8     10/24/00 9:41a Hirose
    added explicit NULL initialization of TEXPalletePtr
    
    7     9/11/00 6:30p Carl
    Had to change texture preload configuration due
    to the change in tile texture format (doubled in size).
    
    6     8/23/00 2:53p Alligator
    allow perf counter api to work with draw sync callback
    
    5     6/26/00 7:31p Carl
    Added texture preload option, stat option, print intro.
    
    4     6/26/00 1:10p Carl
    Removed debug OSReport.
    
    3     6/23/00 7:30p Carl
    Fixed controls.
    Now draws 9 copies of the board, to give that "infinite scrolling"
    effect.
    
    2     5/25/00 11:37p Carl
    Fixed aspect ratio, added movement.
    
    1     5/25/00 4:33a Carl
    Indirect tiled texture test.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>
#define PI 3.1415926535f

// If these two are defined simultaneously, the image will not appear correct.
//#define SHOW_STATS
#define USE_PRELOADED_TEXTURE

// Level 1

void MapInit( TEXPalettePtr *tpl );

extern GXTexObj toTilesRGB_1;

extern GXTexObj toTileMap_1;

/*---------------------------------------------------------------------------*
  Defines
 *---------------------------------------------------------------------------*/

typedef struct
{
    f32 x, y, z;
    f32 s, t;
} Coord;

typedef struct
{
    f32 rPosX;  // Position vector X component (measured in tile coordinates)
    f32 rPosY;  // Position vector Y component (measured in tile coordinates)
    f32 rOriX;  // Normalized orientation vector X component
    f32 rOriY;  // Normalized orientation vector Y component
} G2DPosOri;    // 2D Position and Orientation

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main                ( void );

static void ParameterInit       ( void );

static void TexInit             ( void );
static void DrawInit            ( void );
static void DrawTick            ( void );
static void AnimTick            ( void );
static void sendVertex          (Coord *d);
static void PrintIntro          ( void );

 /*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

G2DPosOri poCam;
G2DPosOri poShip;
f32 rAng = 0.0f;
f32 scale = 1.5f;

static TEXPalettePtr tpl = NULL;

static GXTexRegion texRegTiles, texRegMap;

static DEMOStatObj MyStats[] = {
    {  "pf1 texels........",  DEMO_STAT_GP1, GX_PERF1_TEXELS,      0 },
    {  "pf1 tx memstall...",  DEMO_STAT_GP1, GX_PERF1_TX_MEMSTALL, 0 },
    {  "pf1 tc check 1-2..",  DEMO_STAT_GP1, GX_PERF1_TC_CHECK1_2, 0 },
    {  "pf1 tc check 3-4..",  DEMO_STAT_GP1, GX_PERF1_TC_CHECK3_4, 0 },
    {  "pf1 tc check 5-6..",  DEMO_STAT_GP1, GX_PERF1_TC_CHECK5_6, 0 },
    {  "pf1 tc check 7-8..",  DEMO_STAT_GP1, GX_PERF1_TC_CHECK7_8, 0 },
    {  "pf1 tc miss.......",  DEMO_STAT_GP1, GX_PERF1_TC_MISS,     0 },
    {  "mem tc req........",  DEMO_STAT_MEM, DEMO_STAT_MEM_TC,     0 },
    {  "clocks............",  DEMO_STAT_GP0, GX_PERF0_CLOCKS,      0 },
};
#define STAT_SIZE   (sizeof(MyStats) / sizeof(DEMOStatObj))

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
#ifdef SHOW_STATS
    DEMOSetStats(MyStats, STAT_SIZE, DEMO_STAT_TL);
#endif
    TexInit();

    PrintIntro();    // Print demo directions

    ParameterInit();

    DEMOPadRead();      // Read the joystick for this frame

    // While the quit button is not pressed
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU)) 
    {
        DrawInit();

        DEMOPadRead();  // Read the joystick for this frame

        AnimTick();     // Do animation based on input

        DEMOBeforeRender();

        DrawTick();     // Draw the model.
#ifdef SHOW_STATS
        // Reset various state for stats to appear correctly
        GXSetNumIndStages(0);
        GXSetTevDirect( GX_TEVSTAGE0 );
        GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
#endif
        DEMODoneRender();
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Name:           ParameterInit

    Description:    Initialize parameters for single frame display

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void ParameterInit( void )
{
    rAng = 0.0F;
    poShip.rPosX = 0.0F;
    poShip.rPosY = 0.0F;
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Calls the correct initialization function for the current
                    model.

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void TexInit( void )
{
#ifdef USE_PRELOADED_TEXTURE
    u32 size;
#endif
    // Get/Create textures

    TEXGetPalette(&tpl, "gxTests/G2D-00.tpl");

    MapInit( &tpl );
#ifdef USE_PRELOADED_TEXTURE
    // Preload the textures

    // Note that we don't set up a region allocator callback.
    // This is okay if these are the only textures we use.
    // See the comment in tex-preload if this is not clear.

    size = GXGetTexBufferSize(
        GXGetTexObjWidth(&toTilesRGB_1),
        GXGetTexObjHeight(&toTilesRGB_1),
        GXGetTexObjFmt(&toTilesRGB_1),
        GX_FALSE, 0);

    GXInitTexPreLoadRegion(
        &texRegTiles,
        0x00000,         // tmem_even
        size/2,          // size_even
        0x80000,         // tmem_odd
        size/2 );        // size_odd

    GXPreLoadEntireTexture(&toTilesRGB_1, &texRegTiles);

    // Note that the above texture occupies the entire
    // texture cache.  However, there are some unused tiles
    // towards the end.  The texture below is loaded over
    // those unused tiles.

    size = GXGetTexBufferSize(
        GXGetTexObjWidth(&toTileMap_1),
        GXGetTexObjHeight(&toTileMap_1),
        GXGetTexObjFmt(&toTileMap_1),
        GX_FALSE, 0);

    GXInitTexPreLoadRegion(
        &texRegMap,
        0x100000 - size, // tmem_even
        size,            // size_even
        0x00000,         // tmem_odd (not used)
        0 );             // size_odd (not used)

    GXPreLoadEntireTexture(&toTileMap_1, &texRegMap);
#endif
}

static void DrawInit( void )
{
    Mtx   idtMtx;
    Mtx44 prjMtx;

    // Misc setup
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

    // Set up textures
#ifdef USE_PRELOADED_TEXTURE
    GXLoadTexObjPreLoaded(&toTilesRGB_1, &texRegTiles, GX_TEXMAP0);
    GXLoadTexObjPreLoaded(&toTileMap_1, &texRegMap, GX_TEXMAP1);
#else
    GXLoadTexObj(&toTilesRGB_1, GX_TEXMAP0);
    GXLoadTexObj(&toTileMap_1, GX_TEXMAP1);
#endif
    // Set up texgen
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, 128*32, 64*32);

    // Set up TEV and such...
    GXSetNumTevStages(1);
    GXSetNumIndStages(1);

    // One texture coordinate, no colors.
    GXSetNumTexGens(1);
    GXSetNumChans(0);

    // Indirect Stage 0 -- Lookup tile map
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_32, GX_ITS_32);

    // Stage 0 -- Output texture color
    //
    // TEVPREV = TEXC/TEXA

    // This function also takes care of setting the indirect matrix
    GXSetTevIndTile(GX_TEVSTAGE0,       // tev stage
                    GX_INDTEXSTAGE0,    // indirect stage
                    32,                 // tile size S
                    32,                 // tile size T
                    32,                 // tile spacing S
                    32,                 // tile spacing T
                    GX_ITF_4,           // ind tex format
                    GX_ITM_0,           // ind matrix select
                    GX_ITB_NONE,        // bias select
                    GX_ITBA_OFF);       // bump alpha select

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    MTXIdentity(idtMtx);
    GXLoadPosMtxImm(idtMtx, GX_PNMTX0);
    GXLoadTexMtxImm(idtMtx, GX_TEXMTX0, GX_MTX2x4);

#define ASP (10.0f/7.0f)
    MTXOrtho(prjMtx, -1.5f, 1.5f, -1.5f*ASP, 1.5f*ASP, 1.0f, 15.0f);
    GXSetProjection(prjMtx, GX_ORTHOGRAPHIC);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick

    Description:    Draw the current model once.  

    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( void )
{
    Coord c;

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
#if 0
    // Draw one copy of the board
    c.x = -1.0f;
    c.y = -0.5f;
    c.z = -5.0f;
    c.s =  0.0f;
    c.t =  0.0f;
    sendVertex(&c);
    c.x =  1.0f;
    c.s =  1.0f;
    sendVertex(&c);
    c.y =  0.5f;
    c.t =  1.0f;
    sendVertex(&c);
    c.x = -1.0f;
    c.s =  0.0f;
    sendVertex(&c);
#else
    // Draw nine copies of the board
    c.x = -3.0f;
    c.y = -1.5f;
    c.z = -5.0f;
    c.s = -1.0f;
    c.t = -1.0f;
    sendVertex(&c);
    c.x =  3.0f;
    c.s =  2.0f;
    sendVertex(&c);
    c.y =  1.5f;
    c.t =  2.0f;
    sendVertex(&c);
    c.x = -3.0f;
    c.s = -1.0f;
    sendVertex(&c);
#endif
    GXEnd();
}

void sendVertex(Coord *d)
{
    GXPosition3f32(d->x, d->y, d->z);
    GXTexCoord2f32(d->s, d->t);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick

    Description:    Animate the scene.  

    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( void )
{
    Mtx tmpMtx;
    Mtx posMtx;
    f32 dx, dy;
    u16 stick = DEMOPadGetDirs(0);
    u16 press = DEMOPadGetButton(0);
    u16 down = DEMOPadGetButtonDown(0);

    if (press & PAD_TRIGGER_L)
    rAng += 1.5f;
        
    if (press & PAD_TRIGGER_R)
    rAng -= 1.5f;

    if (down & PAD_BUTTON_A)
    scale *= 1.25f;
    
    if (down & PAD_BUTTON_B)
    scale *= 0.8f;
    
    dx = sinf(rAng / 180.0f * PI) * 0.01f;
    dy = cosf(rAng / 180.0f * PI) * 0.01f;

    if (stick & DEMO_STICK_RIGHT)
    {
        poShip.rPosX -= dy;
        poShip.rPosY += dx;
    }
    
    if (stick & DEMO_STICK_LEFT)
    {
        poShip.rPosX += dy;
        poShip.rPosY -= dx;
    }

    if (stick & DEMO_STICK_DOWN)
    {
        poShip.rPosX -= dx;
        poShip.rPosY -= dy;
    }
    
    if (stick & DEMO_STICK_UP)
    {
        poShip.rPosX += dx;
        poShip.rPosY += dy;
    }

    if (poShip.rPosX >  1.0f) poShip.rPosX -= 2.0f;
    if (poShip.rPosX < -1.0f) poShip.rPosX += 2.0f;
    if (poShip.rPosY >  0.5f) poShip.rPosY -= 1.0f;
    if (poShip.rPosY < -0.5f) poShip.rPosY += 1.0f;

    MTXRotDeg(posMtx, 'z', rAng);
    MTXTrans(tmpMtx, poShip.rPosX, poShip.rPosY, 0.0f);
    MTXConcat(posMtx, tmpMtx, posMtx);
    MTXScale(tmpMtx, scale, scale, 1.0f);
    MTXConcat(tmpMtx, posMtx, posMtx);

    GXLoadPosMtxImm(posMtx, GX_PNMTX0);
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
    OSReport("ind-tile-test: tiled texture map using indirect textures\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("main stick   : moves position\n");
    OSReport("l/r triggers : rotates position\n");
    OSReport("\n");
    OSReport("************************************************\n\n");
}
