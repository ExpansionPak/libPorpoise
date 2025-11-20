/*---------------------------------------------------------------------------*
  Project:  Thread Demo -- No. 1
  File:     thread01.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/osdemo/src/threaddemo2.c $
    
    2     6/07/00 6:12p Tian
    Updated OSCreateThread and idle function APIs
    
    1     2/16/00 2:23p Tian
    Cleaned up and moved to demos
    
    1     2/08/00 5:11p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

OSThread      Thread;
u8            ThreadStack[4096];
volatile u64  Sum;

static void* Func(void* param)
{
    #pragma unused( param )
    u64 n;

    //
    // Do background job
    //
    for (n = 1; n <= 1000000; n++)
    {
        Sum += n;
    }

    //
    // Exits
    //
    return 0;
}

void main(void)
{

    OSInit();
    VIInit();

    OSReport("Thread Demo 2: Using OSJoinThread()\n");

    //
    // Creates a new thread. The thread is suspended by default.
    //
    OSCreateThread(
        &Thread,                            // ptr to the thread to init
        Func,                               // pter to the start routine
        0,                                  // param passed to start routine
        ThreadStack + sizeof ThreadStack,   // initial stack address
        sizeof ThreadStack,                 // stack size
        31,                                 // scheduling priority
        0);                                 // joinable by default

    //
    // Resumes the thread
    //
    OSResumeThread(&Thread);

    //
    // Loop until the thread exits
    //
    do
    {
        OSReport("Sum of 1 to 1000000 >= %llu after %u V-syncs\n",
                 Sum,
                 VIGetRetraceCount());
        VIWaitForRetrace();                 // Sleep till next V-sync
    } while (!OSIsThreadTerminated(&Thread));

    //
    // Release the thread from OS control
    //
    OSJoinThread(&Thread, NULL);

    OSReport("Sum of 1 to 1000000 == %llu after %u V-syncs\n",
             Sum,
             VIGetRetraceCount());

    OSHalt("Demo complete\n");
}
