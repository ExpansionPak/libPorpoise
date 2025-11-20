/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     ind-tile-data.c

  Copyright 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Indirect/ind-tile-data.c $
    
    4     9/11/00 6:28p Carl
    Changed to use different blank tile position.
    
    3     6/23/00 7:29p Carl
    Fixed comments.
    Changed texture mode from clamp to repeat.
    
    2     5/25/00 11:37p Carl
    Added necessary cache flush.
    
    1     5/25/00 4:33a Carl
    Indirect tiled texture test.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/
#define WIDTH_LYR1        128
#define HALF_WIDTH_LYR1    64
#define HMASK_LYR1        127
#define HSHIFT_LYR1         7
#define HEIGHT_LYR1        64
#define HALF_HEIGHT_LYR1   32
#define VMASK_LYR1         63
#define VSHIFT_LYR1         6

#define TILE_WIDTH_LYR1    32
#define TILE_HSHIFT_LYR1    5
#define TILE_HEIGHT_LYR1   32
#define TILE_VSHIFT_LYR1    5

#define WORLD_WIDTH      (WIDTH_LYR1 * TILE_WIDTH_LYR1)
#define WORLD_HEIGHT     (HEIGHT_LYR1 * TILE_HEIGHT_LYR1)

#define NUM_TILES_LYR1         95

/*---------------------------------------------------------------------------*
   Global Variables and Data
 *---------------------------------------------------------------------------*/

typedef struct
{
    f32 rPosX;  // Position vector X component (measured in tile coordinates)
    f32 rPosY;  // Position vector Y component (measured in tile coordinates)
    f32 rOriX;  // Normalized orientation vector X component
    f32 rOriY;  // Normalized orientation vector Y component
} G2DPosOri;    // 2D Position and Orientation

extern G2DPosOri poShip;

GXTexObj toTilesRGB_1;

GXTexObj toTileMap_1;

static u8 tmap1[WIDTH_LYR1 * HEIGHT_LYR1] ATTRIBUTE_ALIGN(32);

static u8 map1[WIDTH_LYR1 * HEIGHT_LYR1] =
{
    "6aT..............46a..............P.13jTPEIOL0LT..1222222222DC222222222223j......P.4C2222222222222223j1222222222222222DC2222222D"
    "6aT..............13j..............P.dfiTPK0KHJGSV.decccccccco6bcccccccccchi......P.46bcccccccccccccchidecccccccccccccco6bcccccco"
    "6aT..............dfi..............XNNNNWPHIG0EIOT...........46a............../0/.P.46a...79n.....................79n..46aMNNNO.4"
    "6aT.....................................XOEJFK0KT...........46a..79n......../0/0/P.46a...4Bp888;<=++>?()++,-:8888A6m..13jT...P.4"
    "6aT......................................PL0LHIGT...........46a.7ABqn.......0/0/0P.46a...4C2222yz{uu|}@tuuvwx2222D6a..dfiT...P.4"
    "6aT......................................PHJGMNNW...........46a.1DC3l......./0/0/P.46a...13gcccccccccc_`cccccccccr3jMNNNNW...P.4"
    "6aT......................................XNNNW..............46a.dr3si......../0/.P.46a...dfiMNNNNNNNNNNNNNNNNNNO.dfiT........P.4"
    "6aT.........................EF..............................46a..dfi.URRRRRRRRRRRQ.46aMNNNNNW..................XNNNNW........P.4"
    "6aT.......................EIGK..............................46a......P.............46aT......................................P.4"
    "6aT.......................HJIG...........URRRRRV............13j......P.788888888888A6aT.............................URRRRV...P.4"
    "6aT......................................P.....T............dfi......P.4C222222222223jT.............................P....T...P.4"
    "6aT......................................P.789nT..........URRRRRRRRRRQ.46bcccccccccchiT....................URV......P.79nT...P.4"
    "6aT......................................P.456mT..........P............46aMNNNNNNNNNNNW...................UQ0SV.....P.46mT...P.4"
    "6aT......................................P.456aT..........P.78888888888A6aT.....................URV.......P0/0T.....P.46aT...P.4"
    "6aT.................................URRRRQ.456aSRRRRV.....P.4C22222222223jT....................UQ0SV......XO0MW.....P.46aT...P.4"
    "6aT.................................P......456a.....T.....P.46bccccccccchiT....................P0/0T..URV..XNW......P.46aT...P.4"
    "6aT.................................P.78888A5Bp8889nT.....P.46aMNNNNNNNNNNW....................XO0MW.UQ0SV..........P.46aT...P.4"
    "6aT...................0.0.0.0.0.....P.4555555555556mT.....P.13jT................................XNW..P0/0T..........P.46aT...P.4"
    "6aT.................................P.12222D5C22223jT.....P.dfiT.....................................XO0MW..........P.46aT...P.4"
    "6aT...................0.0.0.0.0.....P.deccco56bccchiT.....P....T......................................XNW...........P.46aT...P.4"
    "6aT.................................XNNNNO.456aMNNNNW.....P.79nT....................................................P.46aT...P.4"
    "6aT...................0.0.0.0.0..........P.456aT..........P.46mT.......................................URV..........P.46aT...P.4"
    "6aT......................................P.456aT..........P.46aT......................................UQ0SV.........P.46aT...P.4"
    "6aT...................0.0.0.0.0..........P.123jT..........P.46aT......................................P0/0T.........P.46aT...P.4"
    "6aT......................................P.dehiT..........P.46aT......................................XO0MW.........P.46aT...P.4"
    "6aT...................0.0.0.0.0..........XNNNNNW..........P.46aT.......................................XNW..........P.46aT...P.4"
    "6aSRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRQ.46aT....................................................P.46aT...P.4"
    "6a..........................................................46aT.URRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRV..........P.46aT...P.4"
    "Bp888888888888888888888;<=++>?!n.... ()++,-:8888888888888888A6aT.P0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0T..........P.46aT...P.4"
    "C2222222222222222222222yz{uu|}$%....#@tuuvwx222222222222222223jT.P/...................................../T..........P.46aT...P.4"
    "6bcccccccccccccccccccccccccccc_'....d&`cccccccccccccccccccccchiT.P0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0T........URQ.46aT...P.4"
    "6aMNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNW.XNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNW........P...46aT...P.4"
    "3jT...............................................................................................................P.78A6aT...P.1"
    "fiT...............................................................................................................P.1223jT...P.d"
    "NNW..............................................................................URRRRV...........................P.dechiT...XNN"
    ".................................................................................P....T...........................XNNNNNNW......"
    ".................................................................................P.79nT........................................."
    ".................................................................................P.46mT........................................."
    ".................................................................................P.46aT...........URRRV........................."
    "...............................URRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRV................P.46aT........URRQEJFT...........URRRRRRV......"
    "RRV............................P................................SRRRRRRRRRRRRRRRRQ.46aT........PEIOL0LT...........P......T...URR"
    "..T............................P.788888888888888888888888888889n...................46aT........PK0KHJGSV..........P.7889nT...P.."
    "9nT............................P.1222222222222222222222222222DBp888;<=+>?()+,-:8888A6aT........PHIG0EIOT..........P.12D6mT...P.7"
    "6mT..............79n...........P.deccccccccccccccccccccccccccoC2222yz{u|}@tuvwx222223jT........XOEJFK0KT..........P.deo6aT...P.4"
    "6aT...........79n13l79n........XNNNNNNNNNNNNNNNNNNNNNNNNNNNO.46bccccccccc_`ccccccccchiT.........PL0LHIGT..........XNO.46aT...P.4"
    "6aT........79n13ldfi13l79n.................................P.46aMNNNNNNNNNNNNNNNNNNNNNW.........PHJGMNNW............P.13jT...P.4"
    "6aT........13ldfi !ndfi13l.................................P.46aT...............................XNNNW...............P.dfiT...P.4"
    "6aT........dfi !n#$% !ndfi.................................P.46aT...................................................XNNNNW...P.4"
    "6aT........ !n#$%d&'#$% !n.................................P.46aT............................................................P.4"
    "6aT.....79n#$%d&'...d&'#$%79n..............................P.46aT............................................................P.4"
    "6aT.....46md&'...*.....d&'46m..............................P.46aT............................................................P.4"
    "6aT.....13j !n......... !n13j..............................P.46aT............................................................P.4"
    "6aT.....dfi#$% !n... !n#$%dfi..............................P.46aT............................................................P.4"
    "6aT........d&'#$% !n#$%d&'.................................P.13jT.......79n...........EFEFEF.................................P.4"
    "6aT........79nd&'#$%d&'79n.................................P.dfiT.......46m...........KHGHGK.................................P.4"
    "6aT........13l79nd&'79n13l..............0.0.0.0.0..........XNNNNW.......46a...........HFEFEG.................................P.4"
    "6aT........dfi13l79n13ldfi..............................................46a...........0HGHG0.................................P.4"
    "6aT...........dfi46mdfi.................0...0...0.79n...................46a......................URRRRRRRRRRRRRRV............P.4"
    "6aSRRRRRRRRRRRRV.46aURRRRRRRRRRRRRRRRRRV..........46m...................46a......................P..............T............P.4"
    "6a.............T.46aP..................T0.0.0.0.0.46a........ !n........46a......................P.78888889n.79nT............P.4"
    "Bp88888;<=+>?!nT.46aP. ()+,-:888888889nT..........46a........#$%........46a......................P.4C2222D6m.46mT............P.4"
    "C222222yz{u|}$%T.46aP.#@tuvwx2222222D6mT..........46a........d&'........46a......URRRRRRRRRRRRRRRQ.46bcccr3j.46aSRRRRRRRRRRRRQ.4"
    "6bccccccccccc_'T.46aP.d&`ccccccccccco6aT...URRRV..46a...................46a......P.................46a79ndfi.46a...............4"
    "6aMNNNNNNNNNNNNW.46aXNNNNNNNNNNNNNO.46aTURRQEJFT..4Bp8888888888888888888A6a......P.7888888888888888A6a4Bp8888ABp888888888888888A"
};

typedef struct
{
    u8  nMaterial;  // Tile type / texture-map selection
    u8  nS;         // S texture coordinate
    u8  nT;         // T texture cordinate
    u8  nCI;        // Color Index
    u8  aUser[4];   // User data
}
G2DTileDesc;

static G2DTileDesc tileDescLyr1[NUM_TILES_LYR1] =
{
//  M   S   T   CI    User     // M=Material  (S,T) = Tex Coords  -or-
//  M   R   G   B     Data     // CI = Color Index  -or-  (R,G,B) = RGB8 Color
    1,  8,  8,  0,   0,1,0,0,  // ' ': Solo Tower
    1,  9,  8,  0,   3,1,0,0,  // '!': Solo Tower
    0,  0,  0,  0,   0,0,0,0,  // '"': 
    1,  8,  9,  0,   1,1,0,0,  // '#': Solo Tower
    1,  9,  9,  0,   2,1,0,0,  // '$': Solo Tower
    1, 10,  9,  0,   0,0,0,0,  // '%': Solo Tower
    1,  9, 10,  0,   0,0,0,0,  // '&': Solo Tower
    1, 10, 10,  0,   0,0,0,0,  // ''': Solo Tower
    1,  1, 11,  0,   0,2,0,0,  // '(': Solo Tower
    1,  2, 11,  0,   0,2,0,0,  // ')': Raised
    1,  7,  7,  0,   0,0,0,0,  // '*': Starting position
    1,  3, 11,  0,   0,2,0,0,  // '+': Raised
    1,  4, 11,  0,   0,2,0,0,  // ',': Raised
    1,  5, 11,  0,   0,2,0,0,  // '-': Raised
    1,  7,  7,  1,   0,0,0,0,  // '.': Basic tile (colored)
    1,  4,  9,  0,   0,0,0,0,  // '/': Inverse Bump
    1,  1,  9,  0,   0,0,0,0,  // '0': Bump
    1,  0,  5,  0,   1,1,0,0,  // '1': Bottom Left
    1,  1,  5,  0,   2,2,0,0,  // '2': Bottom
    1,  5,  5,  0,   2,1,0,0,  // '3': Bottom Right
    1,  0,  4,  0,   1,2,0,0,  // '4': Left
    1,  4,  4,  2,   0,3,0,0,  // '5': Mid
    1,  5,  4,  0,   3,2,0,0,  // '6': Right
    1,  0,  0,  0,   0,1,0,0,  // '7': Top Left
    1,  1,  0,  0,   0,2,0,0,  // '8': Top
    1,  5,  0,  0,   3,1,0,0,  // '9': Top Right
    1,  6, 11,  0,   0,2,0,0,  // ':': Raised
    1,  7, 11,  0,   0,2,0,0,  // ';': Raised
    1,  8, 11,  0,   0,2,0,0,  // '<': Raised
    1,  9, 11,  0,   0,2,0,0,  // '=': Raised
    1, 10, 11,  0,   0,2,0,0,  // '>': Raised
    1, 11, 11,  0,   0,2,0,0,  // '?': Raised
    1,  1, 12,  0,   2,2,0,0,  // '@': Raised
    1,  3,  4,  0,   0,3,0,0,  // 'A': Inner TL
    1,  1,  4,  0,   3,3,0,0,  // 'B': Inner TR
    1,  1,  1,  0,   2,3,0,0,  // 'C': Inner BR
    1,  4,  1,  0,   1,3,0,0,  // 'D': Inner BL

    1,  0,  8,  1,   0,0,0,0,  // 'E': Floor Marking
    1,  2,  8,  0,   0,0,0,0,  // 'F': Floor Marking
    1,  2, 10,  0,   0,0,0,0,  // 'G': Floor Marking
    1,  0, 10,  0,   0,0,0,0,  // 'H': Floor Marking
    1,  1,  8,  0,   0,0,0,0,  // 'I': Floor Marking
    1,  1, 10,  0,   0,0,0,0,  // 'J': Floor Marking
    1,  2,  9,  0,   0,0,0,0,  // 'K': Floor Marking
    1,  0,  9,  0,   0,0,0,0,  // 'L': Floor Marking
    1,  3,  8,  0,   0,0,0,0,  // 'M': Floor Marking
    1,  4,  8,  0,   0,0,0,0,  // 'N': Floor Marking
    1,  5,  8,  0,   0,0,0,0,  // 'O': Floor Marking
    1,  5,  9,  0,   0,0,0,0,  // 'P': Floor Marking
    1,  5, 10,  0,   0,0,0,0,  // 'Q': Floor Marking
    1,  4, 10,  0,   0,0,0,0,  // 'R': Floor Marking
    1,  3, 10,  0,   0,0,0,0,  // 'S': Floor Marking
    1,  3,  9,  0,   0,0,0,0,  // 'T': Floor Marking
    1,  6,  8,  0,   0,0,0,0,  // 'U': Floor Marking
    1,  7,  8,  0,   0,0,0,0,  // 'V': Floor Marking
    1,  7,  9,  0,   0,0,0,0,  // 'W': Floor Marking
    1,  6,  9,  0,   0,0,0,0,  // 'X': Floor Marking
    0,  0,  0,  0,   0,0,0,0,  // 'Y': 
    0,  0,  0,  0,   0,0,0,0,  // 'Z': 
    0,  0,  0,  0,   0,0,0,0,  // '[': 
    0,  0,  0,  0,   0,0,0,0,  // '\': 
    0,  0,  0,  0,   0,0,0,0,  // ']': 
    0,  0,  0,  0,   0,0,0,0,  // '^': 
    1, 12, 13,  0,   0,0,0,0,  // '_': Shadow
    1,  2, 13,  0,   0,0,0,0,  // '`': Shadow

    1,  2,  3,  0,   0,0,0,0,  // 'a': Shadow
    1,  2,  2,  0,   0,0,0,0,  // 'b': Shadow
    1,  3,  2,  0,   0,0,0,0,  // 'c': Shadow
    1,  0,  6,  0,   0,0,0,0,  // 'd': Shadow
    1,  1,  6,  0,   0,0,0,0,  // 'e': Shadow
    1,  3,  7,  0,   0,0,0,0,  // 'f': Shadow
    1,  4,  6,  0,   0,0,0,0,  // 'g': Shadow
    1,  5,  6,  0,   0,0,0,0,  // 'h': Shadow
    1,  6,  6,  0,   0,0,0,0,  // 'i': Shadow
    1,  6,  5,  0,   0,0,0,0,  // 'j': Shadow
    1,  6,  4,  0,   0,0,0,0,  // 'k': Shadow
    1,  7,  3,  0,   0,0,0,0,  // 'l': Shadow
    1,  6,  1,  0,   0,0,0,0,  // 'm': Shadow
    1,  6,  0,  0,   0,0,0,0,  // 'n': Shadow
    1,  4,  2,  0,   1,2,0,0,  // 'o': Left Shadow
    1,  2,  4,  0,   0,2,0,0,  // 'p': Top Shadow
    1,  6,  2,  0,   3,1,0,0,  // 'q': TR Shadow
    1,  2,  6,  0,   1,1,0,0,  // 'r': BL Shadow
    1, 12,  9,  0,   0,0,0,0,  // 's': Shadow

    1,  2, 12,  0,   2,2,0,0,  // 't': Raised
    1,  3, 12,  0,   2,2,0,0,  // 'u': Raised
    1,  4, 12,  0,   2,2,0,0,  // 'v': Raised
    1,  5, 12,  0,   2,2,0,0,  // 'w': Raised
    1,  6, 12,  0,   2,2,0,0,  // 'x': Raised
    1,  7, 12,  0,   2,2,0,0,  // 'y': Raised
    1,  8, 12,  0,   2,2,0,0,  // 'z': Raised
    1,  9, 12,  0,   2,2,0,0,  // '{': Raised
    1, 10, 12,  0,   2,2,0,0,  // '|': Raised
    1, 11, 12,  0,   2,2,0,0,  // '}': Raised
    0,  0,  0,  0,   0,0,0,0,  // '~': Raised
};

void MapInit( TEXPalettePtr *tpl );

/*---------------------------------------------------------------------------*
    Name:		MapInit

    Description:	Initializes the tile map indirect texture.

    Arguments:		TEXPalettePtr to necessary tpl file
    Returns:		none
 *---------------------------------------------------------------------------*/
void MapInit( TEXPalettePtr *tpl )
{
    s32 nI, nJ, ndx;
    u8  tile;
    s32 off;
    u8  nS, nT;
    
    for (nJ=0; nJ<HEIGHT_LYR1; nJ++)
    {
        for (nI=0; nI<WIDTH_LYR1; nI++)
        {
            ndx = (nJ<<HSHIFT_LYR1)+nI;
            if (map1[ndx] == '*')
            {
                map1[ndx] = '.';
                // Initial ship world position (in pixels)
                poShip.rPosX = (f32) nI/WIDTH_LYR1;
                poShip.rPosY = (f32) nJ/HEIGHT_LYR1;
            }
            tile = (u8) (map1[ndx] - ' ');

            nS = tileDescLyr1[tile].nS;
            nT = tileDescLyr1[tile].nT;

            // This offset calculation works for map widths that are
            // powers of 2
            off = (nI & 7) | ((nI >> 3) << 5) | ((nJ & 3) << 3) |
                  ((nJ >> 2) << (5+(HSHIFT_LYR1-3)));
            
            tmap1[off] = (u8) ((nS<<4) | (nT));
        }
    }
    DCFlushRange((void *) tmap1, 64*128);
    
    // Set up tile texture for bilinear interpolation

    // Tiles
    GXInitTexObjLOD( &toTilesRGB_1,
        GX_LINEAR, GX_LINEAR, 0.0F, 0.0F, 0.0F,
        GX_FALSE, GX_FALSE, GX_ANISO_1 );
    TEXGetGXTexObjFromPalette(*tpl, &toTilesRGB_1, 0);

    // Tile Map
    GXInitTexObj(&toTileMap_1,          // texture object
                 (void*) &tmap1,        // data
                 128,                   // width
                 64,                    // height
                 GX_TF_IA4,             // format
                 GX_REPEAT,             // wrap s
                 GX_REPEAT,             // wrap t
                 GX_FALSE);             // mipmap

    GXInitTexObjLOD( &toTileMap_1,
        GX_NEAR, GX_NEAR, 0.0F, 0.0F, 0.0F,
        GX_FALSE, GX_FALSE, GX_ANISO_1 );
}
