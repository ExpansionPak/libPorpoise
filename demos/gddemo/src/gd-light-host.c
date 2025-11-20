/*---------------------------------------------------------------------------*
  Project:  Dolphin GD demo
  File:     gd-light-host.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-light-host.c $
    
    2     10/09/01 11:27a Hirose
    Moved WinMain to gd-win32-ui.c. Integrated tiny message output
    function.
    
    1     9/21/01 4:06p Hirose
    Initial check in.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   gd-light
     Displaylist demo with lighting commands
     [Host program for creating off-line display list file]
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header file includes
 *---------------------------------------------------------------------------*/
#include <windows.h>

#include <dolphin/gd.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "gd-light.h"

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define RoundUp32B(x)  (((u32)(x) + 31) & ~31)

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
// defined in gd-win32-ui.c
extern void GDReport (char* msg, ...);

void        main     ( void );

/*---------------------------------------------------------------------------*
   Application name
 *---------------------------------------------------------------------------*/
char* AppName = "gd-light-host";

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static void* DisplayListBuffer;
static u32*  PatchListBuffer;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
// Function WinMain() is defined in gd-win32-ui.c
void main ( void )
{
    GDGList dlists[1]; // keeps track of all display lists
    GDGList plists[1]; // keeps track of all patch lists
    u32     numDLs = 1;
    u32     numPLs = 1;
	u32     size;
	void*   buffer;

    size = MODELDL_SIZE_MAX + MODELDL_NUM_PATCHES * sizeof(u32) + 32;
    buffer = malloc(size);
	assert(buffer);

	DisplayListBuffer = (void*)RoundUp32B(buffer);
	PatchListBuffer   = (u32*)((u32)DisplayListBuffer + MODELDL_SIZE_MAX);

    size = CreateModelDL(DisplayListBuffer, PatchListBuffer);

    dlists[0].ptr        = DisplayListBuffer;
    dlists[0].byteLength = size;
    plists[0].ptr        = PatchListBuffer;
    plists[0].byteLength = MODELDL_NUM_PATCHES * sizeof(u32);
    
    GDWriteDLFile("gdLight.gdl", numDLs, numPLs, dlists, plists);

	free(buffer);

	GDReport("Created file \"gdLight.gdl\" successfully.");
}

/*============================================================================*/
