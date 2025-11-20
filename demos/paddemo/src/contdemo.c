/*---------------------------------------------------------------------------*
  Project:  Controller utility demo
  File:     contdemo.c

  Copyright 2000-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/paddemo/src/contdemo.c $
    
    10    5/21/01 10:25a Shiki
    Removed analog A/B output.

    9     5/18/01 8:43p Shiki
    Fixed the default analog mode.

    8     01/04/11 12:46 Shiki
    Revised using escape sequences for smoother screen update.

    7     01/03/22 21:09 Shiki
    Revised.

    6     01/03/06 17:18 Shiki
    Modified InitCont() to support controller recalibration.

    5     01/03/05 11:00 Shiki
    Revised.

    4     12/01/00 6:45p Shiki
    Removed PADSetSpec().

    3     11/21/00 9:52p Shiki
    #ifdef EMU to deal with emulators.

    2     11/20/00 6:10p Shiki
    Added PADSetSpec() line for DS4.

    1     11/13/00 6:02p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>
#include "cont.h"

static void PrintCont(void)
{
    int chan;

    OSReport("\033[H");     // Home
    OSReport("Port  AB XY S ZLR +Pad Left         Right        Trigger\n");
    //        1[-2] AB XY S ZLR <>^v (1234, 1234) (1234, 1234) (123, 123)

    for (chan = 0; chan < PAD_MAX_CONTROLLERS + 1; chan++)
    {
        OSReport("%d[%-2d] %c%c %c%c %c %c%c%c %c%c%c%c (%4d, %4d) (%4d, %4d) (%3d, %3d)\n",
            chan,
            Conts[chan].err,
            (Conts[chan].button & PAD_BUTTON_A) ? 'O' : '_',
            (Conts[chan].button & PAD_BUTTON_B) ? 'O' : '_',
            (Conts[chan].button & PAD_BUTTON_X) ? 'O' : '_',
            (Conts[chan].button & PAD_BUTTON_Y) ? 'O' : '_',
            (Conts[chan].button & PAD_BUTTON_START) ? 'O' : '_',
            (Conts[chan].button & PAD_TRIGGER_Z) ? 'O' : '_',
            (Conts[chan].button & PAD_TRIGGER_L) ? 'O' : '_',
            (Conts[chan].button & PAD_TRIGGER_R) ? 'O' : '_',

            (Conts[chan].button & PAD_BUTTON_LEFT)  ? '<' : '_',
            (Conts[chan].button & PAD_BUTTON_RIGHT) ? '>' : '_',
            (Conts[chan].button & PAD_BUTTON_UP)    ? '^' : '_',
            (Conts[chan].button & PAD_BUTTON_DOWN)  ? 'v' : '_',

            Conts[chan].stickX,
            Conts[chan].stickY,
            Conts[chan].substickX,
            Conts[chan].substickY,
            Conts[chan].triggerLeft,
            Conts[chan].triggerRight);

    }
}

void main(void)
{
    BOOL reset = FALSE;

    VIInit();
    PADInit();
    InitCont(0, FALSE);
    OSReport("\033c");   // Resets the terminal

    for (;;)
    {
        ReadCont();
        if (reset)
        {
            if (!OSGetResetSwitchState())
            {
                reset = FALSE;
            }
            continue;
        }

        PrintCont();

        if (OSGetResetSwitchState())
        {
            reset = TRUE;
            InitCont(0, TRUE);
            OSReport("\033c");   // Resets the terminal
        }
        VIWaitForRetrace();
    }
}
