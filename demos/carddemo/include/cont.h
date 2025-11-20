/*---------------------------------------------------------------------------*
  Project:  Game controller utilities
  File:     cont.h

  Copyright 2000-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/paddemo/include/cont.h $
    
    3     01/04/11 11:30 Shiki
    Updated the comments.

    2     01/03/06 17:17 Shiki
    Modified InitCont() to support controller recalibration.

    1     11/13/00 6:02p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef _CONT_H_
#define _CONT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Cont
{
    // same as PADStatus
    u16 button;                 // Or-ed PAD_BUTTON_* bits
    s8  stickX;                 // -128 <= stickX       <= 127
    s8  stickY;                 // -128 <= stickY       <= 127
    s8  substickX;              // -128 <= substickX    <= 127
    s8  substickY;              // -128 <= substickY    <= 127
    u8  triggerLeft;            //    0 <= triggerLeft  <= 255
    u8  triggerRight;           //    0 <= triggerRight <= 255
    u8  analogA;                //    0 <= analogA      <= 255
    u8  analogB;                //    0 <= analogB      <= 255
    s8  err;                    // one of PAD_ERR_* number

    u16 buttonLast;             // Or-ed PAD_BUTTON_* bits
    u16 down;                   // Or-ed PAD_BUTTON_* bits
    u16 up;                     // Or-ed PAD_BUTTON_* bits
    u16 repeat;                 // Or-ed PAD_BUTTON_* bits

    u32 count;                  // repeat count (private)
} Cont;

void InitCont(u32 connectBits, BOOL recalibrate);
void ReadCont(void);

extern Cont Conts[];

#ifdef __cplusplus
}
#endif

#endif // _CONT_H_
