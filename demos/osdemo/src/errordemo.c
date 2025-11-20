/*---------------------------------------------------------------------------*
  Project:  Dolphin OS Overview - Error handler demo
  File:     errordemo.c

  Copyright 1998-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/osdemo/src/errordemo.c $
    
    3     11/28/01 13:36 Shiki
    Minor fix.

    2     8/01/01 13:04 Shiki
    Improved the error message.

    1     3/08/00 11:42p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This program shows how to set up and use the error handler
  Does not function in emulator.
 *---------------------------------------------------------------------------*/
#include <dolphin.h>

static void ErrorHandler(OSError    error,
                         OSContext* context,
                         u32        dsisr,
                         u32        dar)
{
    #pragma unused (error)
    u32  i;
    u32* p;

    OSReport("------------------------- Context 0x%08x -------------------------\n",
             context);

    for (i = 0; i < 16; i++)
    {
        OSReport("r%-2d  = 0x%08x (%14d)  r%-2d  = 0x%08x (%14d)\n",
                 i,      context->gpr[i],      context->gpr[i],
                 i + 16, context->gpr[i + 16], context->gpr[i + 16]);
    }

    OSReport("LR   = 0x%08x                   CR   = 0x%08x\n",
             context->lr,   context->cr);
    OSReport("SRR0 = 0x%08x                   SRR1 = 0x%08x\n",
             context->srr0, context->srr1);
    OSReport("DSISR= 0x%08x                   DAR  = 0x%08x\n",
             dsisr, dar);

    // Dump stack crawl (at most 16 levels)
    OSReport("\nAddress:      Back Chain    LR Save\n");
    for (i = 0, p = (u32*) context->gpr[1]; // get current sp
         p && (u32) p != 0xffffffff && i++ < 16;
         p = (u32*) *p)                     // get caller sp
    {
        OSReport("0x%08x:   0x%08x    0x%08x\n", p, p[0], p[1]);
    }

    OSReport("\nInstruction at 0x%x (read from SRR0) attempted to access "
             "invalid address 0x%x (read from DAR)\n",
             context->srr0, dar);

    OSHalt("Done.");
}

int main(void)
{
    OSInit();
    OSSetErrorHandler(OS_ERROR_DSI, (OSErrorHandler) ErrorHandler);

    // Touch address zero: DSI exception will be raised.
    *(int*) 0 = 0;

    return 0;
}
