/*---------------------------------------------------------------------------*
  Project:  Dolphin GD demo
  File:     gd-matrix-host.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-matrix-host.c $
    
    2     10/09/01 11:27a Hirose
    Moved WinMain to gd-win32-ui.c. Integrated tiny message output
    function.
    
    1     9/19/01 5:49p Carl
    Source files for GD matrix demo.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <windows.h>

#include <dolphin/gd.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/

#define ASSERT           assert
#define OSRoundUp32B(x)  (((u32)(x) + 31) & ~31)
#define OSAlloc(x)       ((void*)OSRoundUp32B(malloc((x)+31)))
#define OSFree(x)        free(x)

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main      ( void );

// Externs:

extern void CreateDLs ( void );

extern void GDReport (char* msg, ...);

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
// Application name
char* AppName = "gd-matrix-host";

// Display lists *************************************************************

// This set of display lists will each load an indexed position matrix
// and an indexed normal matrix, then draw one face of the cube.

GDLObj DrawDLOs[6];

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
// Function WinMain() is defined in gd-win32-ui.c
void main ( void )
{
    GDGList dlists[6]; // keeps track of all display lists
    u32 numDLs = 6;
    u32 i;

    CreateDLs();

    for(i=0; i < numDLs; i++)
    {
        dlists[i].ptr = GDGetGDLObjStart(&DrawDLOs[i]);
        dlists[i].byteLength = GDGetGDLObjOffset(&DrawDLOs[i]);
    }
    
    GDWriteDLFile("gdMatrix.gdl", numDLs, 0 /*PLs*/, dlists, NULL /*PLs*/);

	GDReport("Created file \"gdMatrix.gdl\" successfully.");
}


