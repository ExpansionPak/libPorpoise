/*---------------------------------------------------------------------------*
  Project:  Thread Demo -- No. 4
  File:     threaddemo4.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/osdemo/src/threaddemo4.c $
    
    2     6/07/00 6:12p Tian
    Updated OSCreateThread and idle function APIs
    
    1     2/16/00 2:23p Tian
    Cleaned up and moved to demos
    
    1     2/08/00 5:11p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <dolphin.h>


OSThread       Thread;
u8             ThreadStack[4096];

OSMutex        Mutex;

//
// Synchronous print
//
static void SyncPrint(char* msg)
{
    OSLockMutex(&Mutex);
    OSReport(msg);
    OSUnlockMutex(&Mutex);
}

// 
// Another thread competing for OSReport output
//
static void * ThreadFunc(void * arg)
{
    #pragma unused(arg)
    u32 i;
    
    for (i = 0; i < 16; i++)
    {
        SyncPrint("<Thread1 says Hi!>\n");
        OSYieldThread();
    }

    return 0;
}


void main(void)
{
    u32 i;

    OSInit();

    //
    // Creates a new thread. The thread is suspended by default.
    //
    OSCreateThread(
        &Thread,                            // ptr to the thread to init
        ThreadFunc,                         // ptr to the start routine
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
    // Initializes the mutex
    //
    OSInitMutex(&Mutex);

    //
    // Main loop
    //
    for (i = 0; i < 16; i++)
    {
        SyncPrint("<Main thread says Hi!>\n");
        OSYieldThread();
    }

    OSHalt("Demo Complete");
}
