/*---------------------------------------------------------------------------*
  Project:  Dolphin OS simple game pad API demo
  File:     motor.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/paddemo/src/motordemo.c $
    
    4     01/03/06 17:18 Shiki
    Modified InitCont() to support controller recalibration.

    3     01/03/05 11:00 Shiki
    Revised.

    2     12/01/00 6:45p Shiki
    Removed PADSetSpec().

    1     11/29/00 4:26p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <string.h>
#include <dolphin.h>
#include "cont.h"

u32 Motor[PAD_MAX_CONTROLLERS];
s32 Level[PAD_MAX_CONTROLLERS];

#define MAX     (60*60)
#define MIN     (10*10)

static void RambleCont(void)
{
    int  chan;
    BOOL controlMotor = FALSE;

    for (chan = 0; chan < PAD_MAX_CONTROLLERS; chan++)
    {
        Cont* cont;

        cont = &Conts[chan];
        if (cont->err != PAD_ERR_NONE)
        {
            continue;
        }

        if (cont->down & PAD_BUTTON_A)
        {
            Motor[chan] = (u32) ((Motor[chan] == PAD_MOTOR_RUMBLE) ?
                                 PAD_MOTOR_STOP : PAD_MOTOR_RUMBLE);
            controlMotor = TRUE;
        }
        else if ((cont->down & PAD_BUTTON_B) &&
                 Motor[chan] == PAD_MOTOR_RUMBLE)
        {
            Motor[chan] = PAD_MOTOR_STOP_HARD;
            controlMotor = TRUE;
        }
        else if (cont->button & PAD_BUTTON_X)
        {
            s32 level = (cont->stickX * cont->stickX + cont->stickY * cont->stickY);
            if (MAX < level && Motor[chan] != PAD_MOTOR_RUMBLE)
            {
                Motor[chan] = PAD_MOTOR_RUMBLE;
                controlMotor = TRUE;
            }
            else if (level < MIN && Motor[chan] != PAD_MOTOR_STOP)
            {
                Motor[chan] = PAD_MOTOR_STOP;
                controlMotor = TRUE;
            }
            else
            {
                // Adjust the frequency to start the motor.
                Level[chan] += level;
                if (MAX < Level[chan])
                {
                    Level[chan] = 0;
                    if (Motor[chan] != PAD_MOTOR_RUMBLE)
                    {
                        Motor[chan] = PAD_MOTOR_RUMBLE;
                        controlMotor = TRUE;
                    }
                }
                else if (Motor[chan] != PAD_MOTOR_STOP)
                {
                    Motor[chan] = PAD_MOTOR_STOP;
                    controlMotor = TRUE;
                }
            }
        }
    }

    if (controlMotor)
    {
        PADControlAllMotors(Motor);
    }
}

static void PrintIntro(void)
{
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("motordemo: control ramble motor\n");
    OSReport("************************************************\n");
    OSReport("\n");
    OSReport("Button A         : start/stop motor\n");
    OSReport("Button B         : hard stop motor\n");
    OSReport("Button X + stick : control motor strength\n");
}

void main(void)
{
    BOOL reset = FALSE;

    VIInit();
    PADInit();

    PrintIntro();

    InitCont(PAD_CHAN0_BIT | PAD_CHAN1_BIT |
             PAD_CHAN2_BIT | PAD_CHAN3_BIT, FALSE);

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

        RambleCont();
        if (OSGetResetSwitchState())
        {
            reset = TRUE;

            // InitCont() stops all motors
            memset(Motor, 0, sizeof Motor);
            memset(Level, 0, sizeof Level);
            InitCont(PAD_CHAN0_BIT | PAD_CHAN1_BIT |
                     PAD_CHAN2_BIT | PAD_CHAN3_BIT, TRUE);
        }
        VIWaitForRetrace();
    }
}
