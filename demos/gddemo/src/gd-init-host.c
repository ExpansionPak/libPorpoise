/*---------------------------------------------------------------------------*
  Project:  Dolphin GD demo
  File:     gd-init-host.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-init-host.c $
    
    3     10/09/01 11:27a Hirose
    Moved WinMain to gd-win32-ui.c. Integrated tiny message output
    function.
    
    2     10/04/01 4:59p Carl
    Fixed patch list length parameter.
    
    1     9/25/01 6:23p Carl
    Sources for GD init demo.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <windows.h>

#include <dolphin/gd.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/

#define ASSERT           assert
#define OSRoundUp32B(x)  (((u32)(x) + 31) & ~31)
#define OSAlloc(x)       ((void*)OSRoundUp32B(malloc((x)+31)))
#define OSFree(x)        free(x)

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void main      ( void );

// Externs:

extern void CreateDLs ( void );

extern void GDReport (char* msg, ...);

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
char* AppName = "gd-init-host";

// Display lists *************************************************************

// This DL is used with the "Draw" display list.
// It initializes state that will be used by the Draw DL.

GDLObj InitDLO;

// This DL draws a cube.

GDLObj DrawDLO;

// This array indicates the offsets to patch memory addresses for the
// primitive data arrays (positions, colors) referred to in the Init DL.

u32 *setArrayOffsets;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
// Function WinMain() is defined in gd-win32-ui.c
void main ( void )
{
    GDGList dlists[2]; // keeps track of all display lists
    GDGList plists[2]; // keeps track of all patch lists
    u32 numDLs = 2;
    u32 numPLs = 1;

    CreateDLs();

    dlists[0].ptr = GDGetGDLObjStart(&InitDLO);
    dlists[0].byteLength = GDGetGDLObjOffset(&InitDLO);
    
    dlists[1].ptr = GDGetGDLObjStart(&DrawDLO);
    dlists[1].byteLength = GDGetGDLObjOffset(&DrawDLO);
    
    plists[0].ptr = setArrayOffsets;
    plists[0].byteLength = 2 * sizeof(u32);
    
    GDWriteDLFile("gdInit.gdl", numDLs, numPLs, dlists, plists);

	GDReport("Created file \"gdInit.gdl\" successfully.");
}

