/*---------------------------------------------------------------------------*
  Project:  Dolphin GD demo
  File:     gd-tev-host.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-tev-host.c $
    
    2     10/09/01 11:27a Hirose
    Moved WinMain to gd-win32-ui.c. Integrated tiny message output
    function.
    
    1     10/04/01 2:48p Hirose
    Initial check in.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   gd-tev
     Displaylist demo with multitexture shader commands
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

#include "gd-tev.h"

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
char* AppName = "gd-tev-host";

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static void(*CreateShdDLFunc[NUM_SHADERDLS])() =
{
    CreateShader0DL, CreateShader1DL, CreateShader2DL, CreateShader3DL
};

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
// Function WinMain() is defined in gd-win32-ui.c
void main ( void )
{
    GDGList dlists[NUM_DLS];    // keeps track of all display lists
    GDGList plists[NUM_PLS];    // keeps track of all patch lists
	u32     size;
	void*   buffer;
	u8*     ptr;
	u32     i;
	u32     index;

    // Allocate entire work memory first
    size   = (SDL_SIZE_MAX + PL_SIZE_MAX * sizeof(u32)) * NUM_SHADERDLS
           + MDL_SIZE_MAX
           + 32;
    buffer = malloc(size);
	assert(buffer);
	
	index  = 0;

    // Make sure 32Bytes alignment
    ptr = (u8*)RoundUp32B(buffer);


    // Create multitexturing shader display lists
    for ( i = 0 ; i < NUM_SHADERDLS ; ++i )
    {
       	dlists[index].ptr = (void*)ptr;
       	ptr += SDL_SIZE_MAX;
       	plists[index].ptr = (u32*)ptr;
      	ptr += PL_SIZE_MAX * sizeof(u32);
        
        (*CreateShdDLFunc[i])(dlists[index].ptr, &dlists[index].byteLength,
                              plists[index].ptr, &plists[index].byteLength);
        
        index++;
    }

    // Create model display list
	dlists[index].ptr = (void*)ptr;
	ptr += MDL_SIZE_MAX;
 
    CreateModelDL(dlists[index].ptr, &dlists[index].byteLength);

    
    GDWriteDLFile("gdTev.gdl", NUM_DLS, NUM_PLS, dlists, plists);

	free(buffer);

	GDReport("Created file \"gdTev.gdl\" successfully.");
}

/*============================================================================*/
