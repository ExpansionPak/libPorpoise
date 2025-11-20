/*---------------------------------------------------------------------------*
  Project:  Dolphin OS Overview - One heap demo
  File:     allocdemo2-oneheap.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /dolphin/build/demos/osdemo/src/allocdemo2-oneheap.c $
    
    3     6/11/99 10:07a Tianli01
    Beautified
    
    2     6/09/99 10:35a Hashida
    Inserted OSRound*32B macros in OSCreateHeap() so that the local arenaHi
    is consistant with the system corresponding variable after the second
    OSSetArenaLo().
    
    1     6/04/99 3:04p Tianli01
    Initial Checkin
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This program shows how to set up and use one heap
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

OSHeapHandle                TheHeap;

#define INT_ARRAY_ENTRIES   1024

void main ()
{
    void*   arenaLo;
    void*   arenaHi;
    u32*    intArray;

    OSInit();

    OSReport("\n-----------------------------------");
    OSReport("\n  Hit Command+Q to quit this demo");
    OSReport("\n-----------------------------------\n\n");

    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();

    // OSInitAlloc should only ever be invoked once.
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 1); // 1 heap
    OSSetArenaLo(arenaLo);

    // The boundaries given to OSCreateHeap should be 32B aligned
    TheHeap = OSCreateHeap((void*)OSRoundUp32B(arenaLo),
                           (void*)OSRoundDown32B(arenaHi));
    OSSetCurrentHeap(TheHeap);
    // From here on out, OSAlloc and OSFree behave like malloc and free
    // respectively

    OSSetArenaLo(arenaLo = arenaHi);

    intArray = (u32*)OSAlloc(sizeof(u32) * INT_ARRAY_ENTRIES);

    // some interesting code using intArray would go here

    OSFree(intArray);

    OSHalt("End of demo");
}
