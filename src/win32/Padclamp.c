/*---------------------------------------------------------------------------*
  Project:  Dolphin OS
  File:     Padclamp.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/libraries/win32/src/Padclamp.c $
    
    1     8/18/00 1:35p Dante
    A Copy of the original ../../pad/padclamp.c for PC only
    
    3     6/13/00 7:28p Shiki
    Added analog trigger clamping.

    2     2/24/00 11:32p Shiki
    Reimplemented for dolphin controller.

    6     9/23/99 4:57p Shiki
    Renamed 'errno' of PADStatus to 'err'.

    4     5/11/99 4:39p Shiki
    Refreshed include tree.

    3     5/07/99 9:15p Shiki
    Fixed dead-zone clamping.

    2     5/06/99 8:18p Shiki
    Renamed PADNormalize() to PADClamp()

    1     5/06/99 7:04p Shiki

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin/os.h>
#include <dolphin/pad.h>

// Parameters for dolphin controller

#ifndef MAC
#define PLAY        15          // the play of the stick
#else
#define PLAY        35          // the play of the stick
#endif
#define XMAX        74
#define YMAX        74
#define XMIDDLE     52
#define YMIDDLE     52

#define TRIGGER_MIN 40          // the play of the trigger
#define TRIGGER_MAX 210

/*---------------------------------------------------------------------------*
  Name:         ClampStick

  Description:  Adjusts stick movement data within the octagon, or clamps
                the data to the origin if stick is close to the origin as
                the play.

  Arguments:    px          pointer to movement data in terms of x-axis
                py          pointer to movement data in terms of y-axis

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClampStick(s8* px, s8* py)
{
    int x = *px;
    int y = *py;
    int signX;
    int signY;
    int d;

    if (0 <= x)
    {
        signX = 1;
    }
    else
    {
        signX = -1;
        x = -x;
    }

    if (0 <= y)
    {
        signY = 1;
    }
    else
    {
        signY = -1;
        y = -y;
    }

    // Clamp dead zone
    if (x <= PLAY)
    {
        x = 0;
    }
    else
    {
        x -= PLAY;
    }
    if (y <= PLAY)
    {
        y = 0;
    }
    else
    {
        y -= PLAY;
    }

    if (x == 0 && y == 0)
    {
        *px = *py = 0;
        return;
        // NOT REACHED HERE
    }

    // Clamp outer octagon
    if (XMIDDLE * y <= YMIDDLE * x)
    {
        d = YMIDDLE * x + (XMAX - XMIDDLE) * y;
        if (YMIDDLE * XMAX < d)
        {
            x = (s8) (YMIDDLE * XMAX * x / d);
            y = (s8) (YMIDDLE * XMAX * y / d);
        }
    }
    else
    {
        d = XMIDDLE * y + (YMAX - YMIDDLE) * x;
        if (XMIDDLE * YMAX < d)
        {
            x = (s8) (XMIDDLE * YMAX * x / d);
            y = (s8) (XMIDDLE * YMAX * y / d);
        }
    }

    *px = (s8) (signX * x);
    *py = (s8) (signY * y);
}

/*---------------------------------------------------------------------------*
  Name:         ClampTrigger

  Description:  Adjusts trigger movement data

  Arguments:    px          pointer to movement data in terms of x-axis
                py          pointer to movement data in terms of y-axis

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClampTrigger(u8* trigger)
{
    if (*trigger <= TRIGGER_MIN)
    {
        *trigger = 0;
    }
    else
    {
        if (TRIGGER_MAX < *trigger)
        {
            *trigger = TRIGGER_MAX;
        }
        *trigger -= TRIGGER_MIN;
    }
}

/*---------------------------------------------------------------------------*
  Name:         PADClamp

  Description:  Clamps game pad status.

  Arguments:    status      array[PAD_MAX_CONTROLLERS] of PADStatus to be
                            clamped.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PADClamp(PADStatus* status)
{
    int i;

    for (i = 0; i < PAD_MAX_CONTROLLERS; i++, status++)
    {
        if (status->err != PAD_ERR_NONE)
            continue;
        ClampStick(&status->stickX,    &status->stickY);
        ClampStick(&status->substickX, &status->substickY);
        ClampTrigger(&status->triggerLeft);
        ClampTrigger(&status->triggerRight);
    }
}
