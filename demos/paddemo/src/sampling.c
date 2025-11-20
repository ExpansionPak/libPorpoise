/*---------------------------------------------------------------------------*
  Project:  Sampling callback demo
  File:     sampling.c

  Copyright 1998-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/paddemo/src/sampling.c $
    
    5     11/27/01 11:56 Shiki
    Revised using SISetSamplingRate().

    4     11/21/01 14:32 Shiki
    Revised.

    3     9/08/01 14:59 Shiki
    Clean up.

    2     9/07/01 22:00 Shiki
    Fixed to make this program work even if no controller is attached
    initially.

    1     9/03/01 19:40 Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <demo.h>

volatile PADStatus Pads[SI_MAX_CHAN];
vu64     Count;

static void SamplingCallback(void)
{
    PADStatus pads[SI_MAX_CHAN];
    s32       chan;

    PADRead(pads);
    PADClamp(pads);
    for (chan = 0; chan < SI_MAX_CHAN; ++chan)
    {
        if (pads[chan].err != PAD_ERR_TRANSFER)
        {
            memcpy((PADStatus*) &Pads[chan], &pads[chan], sizeof(PADStatus));
        }
    }

    ++Count;
}

void main(int argc, char* argv[])
{
    BOOL             reset = FALSE;  // TRUE if reset is requested
    BOOL             enabled;
    PADStatus        pads[SI_MAX_CHAN];
    s32              chan;
    u32              chanBit;
    u32              resetBits;
    u32              msec;
    GXRenderModeObj* rmp;
    GXColor          blue = { 0, 0, 127, 0 };
    s16              x, y;

    DEMOInit(NULL);     // for VIInit() and PADInit()

    for (chan = 0; chan < SI_MAX_CHAN; ++chan)
    {
        Pads[chan].err = PAD_ERR_NO_CONTROLLER;
    }
    PADInit();
    PADSetSamplingCallback(SamplingCallback);

    msec = 0;
    if (argc == 2)
    {
        msec = (u32) atoi(argv[1]);
        if (11 < msec)
        {
            msec = 0;
        }
    }
    SISetSamplingRate(msec);

    // Clear EFB
    GXSetCopyClear(blue, 0x00ffffff);
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);

    for (;;)
    {
        DEMOBeforeRender();

        x = 25;
        y = 30;

        rmp = DEMOGetRenderModeObj();
        DEMOInitCaption(DM_FT_XLU, (s16) rmp->fbWidth * 3 / 4, (s16) rmp->efbHeight * 3 / 4);

        enabled = OSDisableInterrupts();
        memcpy(pads, (PADStatus*) Pads, sizeof Pads);
        OSRestoreInterrupts(enabled);

        // The user specified sampling callback function is usually beginning
        // to be called after the next vertical retrace interrupt. Please make
        // sure that your program does not call PADReset() more than once before
        // the sampling callback function is called, or the controller is
        // removed from the controller port.
        resetBits = 0;
        for (chan = 0; chan < SI_MAX_CHAN; ++chan)
        {
            chanBit = SI_CHAN_BIT(chan);
            switch (SIProbe(chan))
            {
              case SI_GC_CONTROLLER:
              case SI_GC_WAVEBIRD:
                if (pads[chan].err == PAD_ERR_NO_CONTROLLER)
                {
                    resetBits |= chanBit;
                    // Prevent the 2nd call to PADReset() before SamplingCallback() is called.
                    Pads[chan].err = PAD_ERR_NOT_READY;
                }
                break;
              default:
                Pads[chan].err = PAD_ERR_NO_CONTROLLER;
                break;
            }
        }
        if (resetBits)
        {
            PADReset(resetBits);
        }

        DEMOPrintf(x, y += 16, 0, "Port  AB XY M ZLR +Pad Left       Right      Trigger");
        //                         1[-2] AB XY M ZLR <>^v (123, 123) (123, 123) (123, 123)
        for (chan = 0; chan < SI_MAX_CHAN; chan++)
        {
            DEMOPrintf(x, y += 16, 0, "%d[%-2d] %c%c %c%c %c %c%c%c %c%c%c%c (%3d, %3d) (%3d, %3d) (%3d, %3d)",
                       chan,
                       pads[chan].err,
                       (pads[chan].button & PAD_BUTTON_A) ? 'O' : '_',
                       (pads[chan].button & PAD_BUTTON_B) ? 'O' : '_',
                       (pads[chan].button & PAD_BUTTON_X) ? 'O' : '_',
                       (pads[chan].button & PAD_BUTTON_Y) ? 'O' : '_',
                       (pads[chan].button & PAD_BUTTON_MENU) ? 'O' : '_',
                       (pads[chan].button & PAD_TRIGGER_Z) ? 'O' : '_',
                       (pads[chan].button & PAD_TRIGGER_L) ? 'O' : '_',
                       (pads[chan].button & PAD_TRIGGER_R) ? 'O' : '_',

                       (pads[chan].button & PAD_BUTTON_LEFT)  ? '<' : '_',
                       (pads[chan].button & PAD_BUTTON_RIGHT) ? '>' : '_',
                       (pads[chan].button & PAD_BUTTON_UP)    ? '^' : '_',
                       (pads[chan].button & PAD_BUTTON_DOWN)  ? 'v' : '_',

                       pads[chan].stickX,
                       pads[chan].stickY,
                       pads[chan].substickX,
                       pads[chan].substickY,
                       pads[chan].triggerLeft,
                       pads[chan].triggerRight);
        }
        DEMOPrintf(x, y += 16, 0, "Samples: %llu", Count);
        DEMOPrintf(x, y += 16, 0, "Retrace: %u",   VIGetRetraceCount());

        DEMODoneRender();

        // Handle reset button
        if (OSGetResetButtonState())
        {
            reset = TRUE;
        }
        else if (reset)
        {
            // Restart
            OSResetSystem(FALSE, 0x01, FALSE);
        }
    }
}
