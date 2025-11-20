/*---------------------------------------------------------------------------*
  Project:  Dolphin OS Overview - Multiple heap demo
  File:     allocdemo3-multipleheaps.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /dolphin/build/demos/osdemo/src/allocdemo3-multipleheaps.c $
    
    2     6/10/99 11:34p Hashida
    Changed not to do OSRoundDown32B() to arenaHi so that the local arenaHi
    will be consistent with system arenaHi.
    
    1     6/04/99 3:04p Tianli01
    Initial Checkin
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This program shows how to set up and use multiple heaps
 *---------------------------------------------------------------------------*/


#include <dolphin.h>

// Heap sizes MUST be multiples of 32
#define HEAP1_SIZE      65536
#define HEAP2_SIZE      4096

#define OBJ1SIZE        1024
#define OBJ2SIZE        2048

OSHeapHandle            Heap1, Heap2;

void main ()
{
    u8*     arenaLo;
    u8*     arenaHi;
    void*   fromHeap1;
    void*   fromHeap2;

    OSInit();

    OSReport("\n-----------------------------------");
    OSReport("\n  Hit Command+Q to quit this demo");
    OSReport("\n-----------------------------------\n\n");

    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 2); // 2 heaps
    OSSetArenaLo(arenaLo);

    // Ensure boundary is 32B aligned
    arenaLo = (void*)OSRoundUp32B(arenaLo);
    
    Heap1 = OSCreateHeap(arenaLo, arenaLo + HEAP1_SIZE);
    arenaLo += HEAP1_SIZE; 
    Heap2 = OSCreateHeap(arenaLo, arenaLo + HEAP2_SIZE);
    arenaLo += HEAP2_SIZE;
   
    OSSetArenaLo(arenaLo);

    OSSetCurrentHeap(Heap1);
    fromHeap1 = OSAlloc(OBJ1SIZE);

    // Some code allocating from heap1 goes here
    // OSFree will free to heap1 as well

    OSSetCurrentHeap(Heap2);
    fromHeap2 = OSAlloc(OBJ2SIZE);

    // Some code allocating from heap2 goes here
    // OSFree will free to heap2

    OSFreeToHeap(Heap1, fromHeap1);
    OSFreeToHeap(Heap2, fromHeap2);

    // example of allocation overriding the current heap
    fromHeap1 = OSAllocFromHeap(Heap1, OBJ2SIZE);

    OSHalt("Demo complete");
}

