/*---------------------------------------------------------------------------*
  Project:  Basic pad demo
  File:     basic.c

  Copyright 1998-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/paddemo/src/basic.c $
    
    7     5/21/01 10:53a Shiki
    Removed analog A/B outputs.

    6     01/04/11 12:46 Shiki
    Revised using escape sequences for smoother screen update.

    5     01/04/11 11:36 Shiki
    Fixed report message.

    4     01/03/27 13:19 Shiki
    Bug fix.

    3     01/03/23 15:01 Shiki
    Fixed resetBits handling code.

    2     01/03/22 21:09 Shiki
    Revised.

    1     3/01/00 11:48p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

PADStatus Pads[PAD_MAX_CONTROLLERS];

static void PrintPads(void)
{
    int chan;

    OSReport("Port  AB XY S ZLR +Pad Left         Right        Trigger\n");
    //        1[-2] AB XY S ZLR <>^v (1234, 1234) (1234, 1234) (123, 123)
    for (chan = 0; chan < PAD_MAX_CONTROLLERS; ++chan)
    {
        OSReport("%d[%-2d] %c%c %c%c %c %c%c%c %c%c%c%c (%4d, %4d) (%4d, %4d) (%3d, %3d)\n",
            chan,
            Pads[chan].err,
            (Pads[chan].button & PAD_BUTTON_A) ? 'O' : '_',
            (Pads[chan].button & PAD_BUTTON_B) ? 'O' : '_',
            (Pads[chan].button & PAD_BUTTON_X) ? 'O' : '_',
            (Pads[chan].button & PAD_BUTTON_Y) ? 'O' : '_',
            (Pads[chan].button & PAD_BUTTON_START) ? 'O' : '_',
            (Pads[chan].button & PAD_TRIGGER_Z) ? 'O' : '_',
            (Pads[chan].button & PAD_TRIGGER_L) ? 'O' : '_',
            (Pads[chan].button & PAD_TRIGGER_R) ? 'O' : '_',

            (Pads[chan].button & PAD_BUTTON_LEFT)  ? '<' : '_',
            (Pads[chan].button & PAD_BUTTON_RIGHT) ? '>' : '_',
            (Pads[chan].button & PAD_BUTTON_UP)    ? '^' : '_',
            (Pads[chan].button & PAD_BUTTON_DOWN)  ? 'v' : '_',

            Pads[chan].stickX,
            Pads[chan].stickY,
            Pads[chan].substickX,
            Pads[chan].substickY,
            Pads[chan].triggerLeft,
            Pads[chan].triggerRight);
    }
}

void main(void)
{
    u32 padBit;
    u32 resetBits;
    u32 connectedBits;
    int chan;

    VIInit();
    PADInit();
    OSReport("\033c");   // Resets the terminal

    connectedBits = 0x0;

    for (;;)
    {
        PADRead(Pads);

        resetBits = 0x0;
        for (chan = 0; chan < PAD_MAX_CONTROLLERS; ++chan)
        {
            padBit = PAD_CHAN0_BIT >> chan;
            switch (Pads[chan].err)
            {
              case PAD_ERR_NONE:
              case PAD_ERR_TRANSFER:
                connectedBits |= padBit;
                break;
              case PAD_ERR_NO_CONTROLLER:
                resetBits |= padBit;
                break;
              case PAD_ERR_NOT_READY:
              default:
                break;
            }
        }
        if (connectedBits)
        {
            resetBits &= connectedBits;
        }
        if (resetBits)
        {
            PADReset(resetBits);
        }

        OSReport("\033[H");   // Home
        if (connectedBits)
        {
            OSReport("Attached Controllers: 0x%1x.\n", connectedBits);
            PrintPads();
            PADClamp(Pads);
            OSReport("\nClamped\n");
            PrintPads();
        }
        else
        {
            OSReport("Please connect controllers\n");
        }

        VIWaitForRetrace();
    }
}
