/*---------------------------------------------------------------------------*
  Project:  Dolphin OS Overview - Timer demo
  File:     timerdemo.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/osdemo/src/timerdemo.c $
    
    4     9/03/01 20:04 Shiki
    Updated.

    3     3/16/00 7:26p Shiki
    Revised to use Alarm API instead of Timer API.

    2     3/16/00 7:03p Shiki
    Revised to include <private/OSTimer.h>

    2     9/10/99 4:35p Tian
    added function prototype

    1     6/07/99 1:53p Tianli01
    Iniital checkin
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This program shows how to set up and use the alarm
  Does not function in emulator.
 *---------------------------------------------------------------------------*/
#include <dolphin.h>

#define PERIOD  5   // sec

OSAlarm Alarm;

static void AlarmHandler(OSAlarm* alarm, OSContext* context)
{
    #pragma unused( alarm, context )
    OSTime t;

    t = OSGetTime();
    OSReport("Alarm at %lld.%03lld [sec]\n",
             OSTicksToSeconds(t),
             OSTicksToMilliseconds(t) % 1000);
}

void main(void)
{
    OSTime now;

    now = OSGetTime();
    OSReport("The time now is %lld.%03lld [sec]\n",
             OSTicksToSeconds(now),
             OSTicksToMilliseconds(now) % 1000);
    OSReport("Initializing period to %d [sec]\n", PERIOD);

    OSCreateAlarm(&Alarm);
    OSSetPeriodicAlarm(
        &Alarm,                     // pointer to alarm
        now,                        // start counting immediately
        OSSecondsToTicks(PERIOD),   // set 5 sec period
        AlarmHandler);              // alarm handler to be called at every
                                    // PERIOD sec

    for (;;)
    {

    }
}
