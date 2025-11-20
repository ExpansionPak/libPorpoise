/*---------------------------------------------------------------------------*
  Project:  Dolphin OS Overview - Stopwatch demo
  File:     stopwatchdemo.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /dolphin/build/demos/osoverview/src/stopwatchdemo.c $
    
    1     6/04/99 3:04p Tianli01
    Initial Checkin
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This program times how long a matrix concat takes
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

#define OUTER_ITERATIONS  50
#define INNER_ITERATIONS  100

OSStopwatch         MySW;

void main (void)
{
    u32             i, j;
    Mtx             a, b, ab;
    	
    OSInit();

    OSReport("\n-----------------------------------");
    OSReport("\n  Hit Command+Q to quit this demo");
    OSReport("\n-----------------------------------\n\n");

    OSInitStopwatch(&MySW, "100 concat stopwatch");
    OSReport("Stopwatch demo program\nTimes %d matrix concatenations\n",
             OUTER_ITERATIONS*INNER_ITERATIONS);

    MTXIdentity(a);
    MTXIdentity(b);
    MTXIdentity(ab);    

    for (i = 0; i < OUTER_ITERATIONS; i++)
    {
        OSStartStopwatch(&MySW);    
        for (j = 0; j < INNER_ITERATIONS; j++)
        {
            MTXConcat(a, b, ab);
        }
        OSStopStopwatch(&MySW);        
    }
    OSReport("\nEach hit is 100 matrix concats:\n");
    OSDumpStopwatch(&MySW);
    OSHalt("Stopwatch Demo complete");
}
