/*---------------------------------------------------------------------------*
  Project:  Dolphin OS Overview - Getting memory demo
  File:     allocdemo1-gettingmemory.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /dolphin/build/demos/osoverview/src/allocdemo1-gettingmemory.c $
    
    1     6/04/99 3:04p Tianli01
    Initial Checkin
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This program shows how to get memory from the arena
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

#define MY_FIRST_MEMORY_AREA_SIZE       1024
#define MY_SECOND_MEMORY_AREA_SIZE      2048

void * MyFirstMemoryArea;
void * MySecondMemoryArea;

void main ()
{
    u8* arenaLo;
    u8* arenaHi;

    OSInit();

    OSReport("\n-----------------------------------");
    OSReport("\n  Hit Command+Q to quit this demo");
    OSReport("\n-----------------------------------\n\n");

    arenaLo             = OSGetArenaLo();
    arenaHi             = OSGetArenaHi();

    MyFirstMemoryArea 	= arenaLo;
    arenaLo += MY_FIRST_MEMORY_AREA_SIZE;
    OSSetArenaLo(arenaLo);

    MySecondMemoryArea 	= arenaLo;
    arenaLo += MY_SECOND_MEMORY_AREA_SIZE;
    OSSetArenaLo(arenaLo);

    OSReport("First memory area is at  0x%x\n", MyFirstMemoryArea);
    OSReport("Second memory area is at 0x%x\n", MySecondMemoryArea);    
    OSReport("New arena Lo is at       0x%x\n", OSGetArenaLo());    

    OSHalt("Getting memory demo complete");
}
