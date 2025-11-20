/*---------------------------------------------------------------------------*
  Project:  Game controller utilities
  File:     cont.c

  Copyright 2000-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/paddemo/src/cont.c $
    
    8     8/06/01 18:29 Shiki
    Revised ReadCont().

    7     01/03/27 13:16 Shiki
    Fixed ReadCont().

    6     01/03/23 15:06 Shiki
    Fixed resetBits handling.

    5     01/03/22 21:07 Shiki
    Revised.

    4     01/03/06 17:18 Shiki
    Modified InitCont() to support controller recalibration.

    3     01/03/06 15:51 Shiki
    Fixed ReadCont() so it can work even if no controller is connected
    initially.

    2     11/14/00 3:02p Shiki
    Fixed MAC build error.

    1     11/13/00 6:02p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <dolphin.h>
#include "cont.h"

Cont Conts[PAD_MAX_CONTROLLERS+1];
static int RepeatDelay;
static int RepeatRate;
static u32 ConnectedBits;

/*---------------------------------------------------------------------------*
  Name:         InitCont

  Description:  Reinitializes controllers

  Arguments:    connectBits     Set PAD_CHANn_BIT(s) to try to connect to the
                                specified controller ports at every frame no
                                matter what.
                                Set zero to use controllers that initially
                                connected to the controller ports to minimize
                                extra processor overhead.
                recalibrate     Set TRUE to recalibrate controllers. (Should
                                be set TRUE only when the reset button on the
                                console is pushed.)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitCont(u32 connectBits, BOOL recalibrate)
{
    u32 resetBits;

    if (VIGetTvFormat() == VI_PAL)
    {
        // 50Hz
        RepeatDelay = 25;
        RepeatRate  = 5;
    }
    else    // VI_NTSC, VI_MPAL
    {
        // 60Hz
        RepeatDelay = 30;
        RepeatRate  = 6;
    }

    ConnectedBits = connectBits;
    resetBits = PAD_CHAN0_BIT | PAD_CHAN1_BIT |
                PAD_CHAN2_BIT | PAD_CHAN3_BIT;
    if (ConnectedBits)
    {
        resetBits &= ConnectedBits;
    }
    if (recalibrate)
    {
        PADRecalibrate(resetBits);
    }
    else
    {
        PADReset(resetBits);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ReadCont

  Description:  Read controllers

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ReadCont(void)
{
    PADStatus  pads[PAD_MAX_CONTROLLERS];
    PADStatus* pad;
    u32        padBit;
    int        chan;
    Cont*      cont;
    Cont*      contAll;
    u32        resetBits;

    PADRead(pads);
    PADClamp(pads);

    contAll = &Conts[PAD_MAX_CONTROLLERS];
    contAll->err = PAD_ERR_NO_CONTROLLER;

    resetBits = 0;
    for (chan = 0; chan < PAD_MAX_CONTROLLERS; ++chan)
    {
        padBit = PAD_CHAN0_BIT >> chan;
        switch (pads[chan].err)
        {
          case PAD_ERR_NONE:
            ConnectedBits |= padBit;
            contAll->err = PAD_ERR_NONE;
            break;
          case PAD_ERR_TRANSFER:
            ConnectedBits |= padBit;
            if (contAll->err == PAD_ERR_NO_CONTROLLER)
            {
                contAll->err = PAD_ERR_TRANSFER;
            }
            break;
          case PAD_ERR_NO_CONTROLLER:
            resetBits |= padBit;
            break;
          case PAD_ERR_NOT_READY:
            if (contAll->err == PAD_ERR_NO_CONTROLLER)
            {
                contAll->err = PAD_ERR_NOT_READY;
            }
          default:
            break;
        }
    }
    if (ConnectedBits)
    {
        resetBits &= ConnectedBits;
    }
    if (resetBits)
    {
        PADReset(resetBits);
    }

    contAll->buttonLast = contAll->button;
    contAll->button = contAll->down = contAll->up = contAll->repeat = 0;
    contAll->stickX = contAll->stickY = contAll->substickX = contAll->substickY = 0;
    contAll->triggerLeft = contAll->triggerRight = 0;
    contAll->analogA = contAll->analogB = 0;

    for (chan = 0; chan < PAD_MAX_CONTROLLERS; ++chan)
    {
        cont = &Conts[chan];
        pad = &pads[chan];
        cont->err = pad->err;

        cont->buttonLast = cont->button;    // not to generate up/down twice
        if (cont->err != PAD_ERR_TRANSFER)
        {
            cont->button = pad->button;
            cont->stickX = pad->stickX;
            cont->stickY = pad->stickY;
            cont->substickX = pad->substickX;
            cont->substickY = pad->substickY;
            cont->triggerLeft  = pad->triggerLeft;
            cont->triggerRight = pad->triggerRight;
            cont->analogA = pad->analogA;
            cont->analogB = pad->analogB;
        }

        if (cont->stickX < 0)
        {
            cont->button |= PAD_BUTTON_LEFT;
        }
        else if (0 < cont->stickX)
        {
            cont->button |= PAD_BUTTON_RIGHT;
        }
        if (cont->stickY < 0)
        {
            cont->button |= PAD_BUTTON_DOWN;
        }
        else if (0 < cont->stickY)
        {
            cont->button |= PAD_BUTTON_UP;
        }

        cont->down = PADButtonDown(cont->buttonLast, cont->button);
        cont->up   = PADButtonUp(cont->buttonLast, cont->button);

        cont->repeat = (u16) ((cont->button & cont->buttonLast) &
                       (PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_X | PAD_BUTTON_Y |
                        PAD_TRIGGER_Z | PAD_TRIGGER_R | PAD_TRIGGER_L |
                        PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT |
                        PAD_BUTTON_DOWN | PAD_BUTTON_UP |
                        PAD_BUTTON_START));
        if (cont->repeat)
        {
            ++cont->count;
            if (cont->count < RepeatDelay)
            {
                cont->repeat = 0;
            }
            else if (cont->count % RepeatRate)
            {
                cont->repeat = 0;
            }
        }
        else
        {
            cont->count = 0;
        }
        cont->repeat |= cont->down;

        contAll->down   |= cont->down;
        contAll->up     |= cont->up;
        contAll->button |= cont->button;
        contAll->repeat |= cont->repeat;

        if (abs(contAll->stickX) < abs(cont->stickX))
        {
            contAll->stickX = cont->stickX;
        }
        if (abs(contAll->stickY) < abs(cont->stickY))
        {
            contAll->stickY = cont->stickY;
        }
        if (abs(contAll->substickX) < abs(cont->substickX))
        {
            contAll->substickX = cont->substickX;
        }
        if (abs(contAll->substickY) < abs(cont->substickY))
        {
            contAll->substickY = cont->substickY;
        }
        if (contAll->triggerLeft < cont->triggerLeft)
        {
            contAll->triggerLeft = cont->triggerLeft;
        }
        if (contAll->triggerRight < cont->triggerRight)
        {
            contAll->triggerRight = cont->triggerRight;
        }
        if (contAll->analogA < cont->analogA)
        {
            contAll->analogA = cont->analogA;
        }
        if (contAll->analogB < cont->analogB)
        {
            contAll->analogB = cont->analogB;
        }
    }
}
