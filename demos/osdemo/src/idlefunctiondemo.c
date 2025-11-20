/*---------------------------------------------------------------------------*
  Project:  Idle function demo
  File:     idlefunctiondemo.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/osdemo/src/idlefunctiondemo.c $
    
    2     6/07/00 6:12p Tian
    Updated OSCreateThread and idle function APIs
    
    1     2/16/00 2:23p Tian
    Cleaned up and moved to demos
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

u8              Stack[4096];
volatile u64    Sum;

static void Func(void* param)
{
    #pragma unused( param )
    u64 n;

    // Do background job
    for (n = 1; n <= 1000000; n++)
    {
        Sum += n;
    }
}

void main(void)
{

    OSInit();
    VIInit();
    OSReport("Idle Function Demo\n");
     
    OSSetIdleFunction(
        Func,                   // start function
        0,                      // initial parameter
        Stack + sizeof Stack,   // initial stack address
        sizeof Stack);          // stack size

    // Loop until the idle function completes. OSGetIdleFunction()
    // returns NULL after the idle function completes.
    do
    {
        OSReport("Sum of 1 to 1000000 >= %llu after %u V-syncs\n",
                 Sum,
                 VIGetRetraceCount());
        VIWaitForRetrace();                 // Sleep till next V-sync
    } while (OSGetIdleFunction());

    OSReport("Sum of 1 to 1000000 == %llu after %u V-syncs\n",
             Sum,
             VIGetRetraceCount());

    OSHalt("Demo complete");
}
