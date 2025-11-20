/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     G2D-test00.h (Test of 2D API by Paul Donnelly, Nov. 1999)

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/include/G2D-test.h $
    
    4     10/26/00 10:34a Hirose
    fixed flags
    
    3     3/24/00 5:41p Carl
    Added #defines for screen size.
    
    2     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    1     3/06/00 12:03p Alligator
    move from gx/tests to demos/gxdemos and rename
    
    6     99/12/15 8:41p Paul
    
    5     99/12/15 2:13p Paul
    Disabled Map-editor except in MAC emulator mode
    
    4     99/12/14 7:57p Paul
    
    3     99/12/12 10:42p Paul
    
    2     99/12/12 10:30p Paul
    
    1     99/12/09 12:29p Paul

 *---------------------------------------------------------------------------*/

#ifndef __G2DTEST_H__
#define __G2DTEST_H__

#ifdef __cplusplus
    extern "C" {
#endif

/*---------------------------------------------------------------------------*
   Includes
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <G2D.h>

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/

#define MY_SCREEN_WIDTH  640
#define MY_SCREEN_HEIGHT 448

#define USED_TILES 86

#ifdef EMU
    #define _EDITOR
#endif

/*---------------------------------------------------------------------------*
  Global Variables
 *---------------------------------------------------------------------------*/

// Joy
extern u16 nButtons;
extern u16 nOldButtons;
extern s8 stickX;
extern s8 stickY;

// Phy
extern G2DPosOri poCam;
extern G2DPosOri poShip;
extern f32 rAng;

extern s32 nStartIdx;
extern u32 nMode;
extern u8 *map;
extern s32 nMapWidth, nMapHeight;
extern f32 rWorldWidth, rWorldHeight;

// Level1
extern const char sUsedTiles[];
extern G2DLayer lyrBack;
extern G2DLayer lyrFront;
extern G2DTileDesc tileDescLyr1[];
extern G2DTileDesc tileDescLyr2[];

#ifdef _EDITOR

    // Editor
    extern u32 nMode;
    extern s32 nMapIdx;
    extern s32 nMapX, nMapY;
    extern s32 nStampLeft, nStampWidth;
    extern s32 nStampTop, nStampHeight;
    extern s32 nStampStartX, nStampStartY;
    extern s32 nStampOffsetX, nStampOffsetY;
    extern u8 aEditStamp[];
    extern u8 aEditBack[];

#endif // ifdef _EDITOR

/*---------------------------------------------------------------------------*
  Function Declarations
 *---------------------------------------------------------------------------*/

// Joy
void JoyInit        ( void );
void JoyTick        ( void );
u16  JoyReadButtons ( void );
s8 	 JoyReadStickX 	( void );
s8 	 JoyReadStickY 	( void );

// Phy
extern void AnimInit( void );
extern void AnimTick( void );

// Level1
void InitLevel1( TEXPalettePtr *tpl );

// Editor
void MapEditor( G2DLayer *layer );
void SaveMap( G2DLayer *layer );
void RenderEditorMode( s8 *aSortBuffer );

/*
#ifdef _WIN32
    #define OSReport printf
#endif
*/

#ifdef __cplusplus
	}
#endif

/* Prevent re-inclusion */
#endif // __G2DTEST_H__

/*===========================================================================*/

