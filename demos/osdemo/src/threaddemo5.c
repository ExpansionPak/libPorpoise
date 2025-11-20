/*---------------------------------------------------------------------------*
  Project:  Thread Demo -- No. 5
  File:     threaddemo5.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/osdemo/src/threaddemo5.c $
    
    2     6/07/00 6:12p Tian
    Updated OSCreateThread and idle function APIs
    
    1     2/16/00 2:23p Tian
    Cleaned up and moved to demos
    
    1     2/08/00 5:11p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <string.h>
#include <dolphin.h>

#define  BUFFER_SIZE  4

//
// Bounded buffer data structure
//
OSMutex  Mutex;
OSCond   CondNotFull;
OSCond   CondNotEmpty;
s32      Buffer[BUFFER_SIZE];
s32      Count;

OSThread Thread;
u8       ThreadStack[4096];

static s32 Get(void)
{
    s32 item;

    OSLockMutex(&Mutex);
    while (Count == 0)
    {
        OSWaitCond(&CondNotEmpty, &Mutex);
    }
    item = Buffer[0];
    --Count;
    memmove(&Buffer[0], &Buffer[1], sizeof(s32) * Count);
    OSUnlockMutex(&Mutex);
    OSSignalCond(&CondNotFull);
    return item;
}

static void Put(s32 item)
{
    OSLockMutex(&Mutex);
    while (BUFFER_SIZE <= Count)
    {
        OSWaitCond(&CondNotFull, &Mutex);
    }
    Buffer[Count] = item;
    ++Count;
    OSUnlockMutex(&Mutex);
    OSSignalCond(&CondNotEmpty);
}

static void* Func(void* param)
{
    #pragma unused (param)
    s32 item;

    for (item = 0; item < 16; item++)
    {
        Put(item);
    }

    return 0;   
}

void main(void)
{
    s32 i;

    OSInit();
    VIInit();

    //
    // Initializes mutex and condition variables
    //
    OSInitMutex(&Mutex);
    OSInitCond(&CondNotFull);
    OSInitCond(&CondNotEmpty);

    //
    // Creates a new thread. The thread is suspended by default.
    //
    OSCreateThread(
        &Thread,                            // ptr to the thread to init
        Func,                               // ptr to the start routine
        0,                                  // param passed to start routine
        ThreadStack + sizeof ThreadStack,   // initial stack address
        sizeof ThreadStack,                 // stack size
        16,                                 // scheduling priority
        OS_THREAD_ATTR_DETACH);             // detached


    //
    // Starts the thread
    //
    OSResumeThread(&Thread);

    //
    // Main loop
    //
    for (i = 0; i < 16; i++)
    {
        s32 item;

        item = Get();
        OSReport("%d\n", item);
        VIWaitForRetrace();                 // Sleep till next V-sync
    }

    OSHalt("Demo complete\n");
}
