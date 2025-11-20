/*---------------------------------------------------------------------------*
  Project:  Dolphin OS simple game pad API demo
  File:     main.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/paddemo/src/simple.c $
    
    3     01/04/11 11:41 Shiki
    Revised.

    2     01/03/22 21:08 Shiki
    Revised.

    1     3/01/00 11:48p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

PADStatus Pads[PAD_MAX_CONTROLLERS];

void main(void)
{
    u16 button = 0; // Previous button status
    u16 down;       // Buttons just pressed down
    u16 up;         // Buttons just released

    VIInit();       // VI must be initialized before PAD
    PADInit();

    do {
        PADRead(Pads);

        if (Pads[0].err != PAD_ERR_NONE)
            continue;

        down = PADButtonDown(button, Pads[0].button);
        up   = PADButtonUp  (button, Pads[0].button);
        button = Pads[0].button;

        PADClamp(Pads);

        OSReport("Buttons: %c%c%c%c Stick: (%3d, %3d) Sub: (%3d, %3d) LR: (%3d, %3d) Down: %c%c Up: %c%c\n",
            (Pads[0].button & PAD_BUTTON_A) ? 'A' : '_',
            (Pads[0].button & PAD_BUTTON_B) ? 'B' : '_',
            (Pads[0].button & PAD_BUTTON_X) ? 'X' : '_',
            (Pads[0].button & PAD_BUTTON_Y) ? 'Y' : '_',
            Pads[0].stickX,
            Pads[0].stickY,
            Pads[0].substickX,
            Pads[0].substickY,
            Pads[0].triggerLeft,
            Pads[0].triggerRight,
            (down & PAD_BUTTON_A) ? 'A' : '_',
            (down & PAD_BUTTON_B) ? 'B' : '_',
            (up & PAD_BUTTON_A) ? 'A' : '_',
            (up & PAD_BUTTON_B) ? 'B' : '_');
    } while (!(button & PAD_BUTTON_START));
}
