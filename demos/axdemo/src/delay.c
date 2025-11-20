/*---------------------------------------------------------------------------*
  Project:  Simple single instance delay for AX
  File:     delay.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/axdemo/src/delay.c $
    
    2     8/03/01 4:32p Billyjack
    added OSEnableInterrupts() and OSRestoreInterrupts() to AX aufdio frame
    callback per change in AX lib.
    
    1     5/09/01 6:09p Billyjack
    created
   
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <dolphin.h>
#include "delay.h"
 
/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
#define DELAY_SAMPLES                   32000   // 1 second worth of samples
#define DELAY_SAMPLES_PER_AUDIO_FRAME   160     // 5ms frame

static s32 L[DELAY_SAMPLES];
static s32 R[DELAY_SAMPLES];
static s32 S[DELAY_SAMPLES];

static vs32 *inputL, *inputR, *inputS;
static vs32 *outputL, *outputR, *outputS;

static vu32 inputFrame;
static vu32 outputFrame;
static vu32 effectOn;


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
void DelayInit(void)
{
    int i;
    s32 *pL, *pR, *pS;

    effectOn = 0;

    // nitialize pointer and frame counter used to wrap buffers
    inputL  = &L[0]; 
    inputR  = &R[0]; 
    inputS  = &S[0]; 
    
    inputFrame  = 0;

    outputL = &L[DELAY_SAMPLES_PER_AUDIO_FRAME]; 
    outputR = &R[DELAY_SAMPLES_PER_AUDIO_FRAME]; 
    outputS = &S[DELAY_SAMPLES_PER_AUDIO_FRAME]; 
    
    outputFrame = 1;

    // zero the buffers
    pL = &L[0];
    pR = &R[0];
    pS = &S[0];
    
    for (i = 0; i < DELAY_SAMPLES; i++)
    {
        *pL = 0; pL++;
        *pR = 0; pR++;
        *pS = 0; pS++;
    }

    effectOn = 1;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
void DelayShutdown(void)
{
    effectOn = 0;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
void DelayCallback(AXFX_BUFFERUPDATE *update, void *user)
{
#pragma unused(user)    // this is only single instance effect

    int old = OSEnableInterrupts();

    if (effectOn)
    {
        int i;
        s32 *pL, *pR, *pS;

        // first we eat input from caller specified location then put output
        // on the same location
        pL = update->left;
        pR = update->right;
        pS = update->surround;

        // don't worry about invalidating or flushing the data cache.. AX does
        // that for you

        for (i = 0; i < DELAY_SAMPLES_PER_AUDIO_FRAME; i++)
        {
            *inputL = *pL; inputL++;
            *pL = *outputL; outputL++;
            pL++;

            *inputR = *pR; inputR++;
            *pR = *outputR; outputR++;
            pR++;

            *inputS = *pS; inputS++;
            *pS = *outputS; outputS++;
            pS++;
        }

        // take care of buffer wrapping
        inputFrame++;
        inputFrame %= 200;

        if (inputFrame == 0)
        {
            inputL  = &L[0]; 
            inputR  = &R[0]; 
            inputS  = &S[0]; 
        }

        outputFrame++;
        outputFrame %= 200;

        if (outputFrame == 0)
        {
            outputL = &L[0]; 
            outputR = &R[0]; 
            outputS = &S[0]; 
        }
    }

    OSRestoreInterrupts(old);
}