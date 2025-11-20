/*---------------------------------------------------------------------------*
  Project:  HIO test
  File:     test.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/hiodemo/src/print.c $
    
    2     12/20/00 9:07p Dante
    NCL modification
    
    1     11/27/00 3:47p Tian
    Initial checkin - based on code from Shiki
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This simple test constantly writes to the mailbox, using the low level HIO
  interfaces.  This is an EARLY demo of the low level APIs.  Higher level
  APIs are forthcoming.

  HIO module must be attached to HIO_CHAN
 *---------------------------------------------------------------------------*/
#include <dolphin.h>
#include <dolphin/hio.h>
#include <stdio.h>
#include <string.h>

#define HIO_CHAN    1
#define BUFSIZE     4096

char          Buffer[BUFSIZE * 2] ATTRIBUTE_ALIGN(32);
char*         Current = Buffer;
char*         Pointer = Buffer;
volatile BOOL Busy  = FALSE;
volatile BOOL Ready = TRUE;


/*---------------------------------------------------------------------------*
    Name:           TxCallback
    
    Description:    Called when our asynchronous write is complete.
                    
    Arguments:      None.
    
    Returns:        None.
 *---------------------------------------------------------------------------*/
static void TxCallback(void)
{
    HIOWriteMailbox(0);
    Busy = FALSE;
}


/*---------------------------------------------------------------------------*
    Name:           RxCallback
    
    Description:    Called when there is data pending
                    
    Arguments:      None.
    
    Returns:        None.
 *---------------------------------------------------------------------------*/
static void RxCallback(void)
{
    u32 mailbox;

    HIOReadMailbox(&mailbox);
    Ready = TRUE;
}


/*---------------------------------------------------------------------------*
    Name:           Flush
    
    Description:    sends BUFSIZE characters from the /Current/ buffer
                    
    Arguments:      None.
    
    Returns:        None.
 *---------------------------------------------------------------------------*/
static void Flush(void)
{
    BOOL enabled;

    while (!Ready || Busy)
    {
        ;
    }
    enabled = OSDisableInterrupts();
    if (Current < Pointer)
    {
        Busy = TRUE;
        Ready = FALSE;
        OSRestoreInterrupts(enabled);
        DCStoreRange(Current, BUFSIZE);
        HIOWriteAsync(0x4000, Current, BUFSIZE, TxCallback);
        Current = (Current == Buffer) ? Buffer + BUFSIZE : Buffer;
        Pointer = Current;
    }
    OSRestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
    Name:           Print
    
    Description:    Sends /len/ characters pointed to by /s/.  Buffers up to
                    BUFSIZE characters before sending it down the HIO via
                    Flush.
                    
    Arguments:      None.
    
    Returns:        None.
 *---------------------------------------------------------------------------*/
static void Print(char* s, u32 len)
{
    u32 xfer;

    while (0 < len)
    {
        xfer = ((Pointer - Current) + len < BUFSIZE) ? len : BUFSIZE - (Pointer - Current);
        memcpy(Pointer, s, xfer);
        Pointer += xfer;
        s += xfer;
        len -= xfer;
        if (Current + BUFSIZE <= Pointer)
        {
            Flush();
        }
    }
}

int main(void)
{
    u32 mailbox;
    int n;

    VIInit();

    OSReport("Initializing Host I/O...\n");
    while (!HIOInit(HIO_CHAN, RxCallback))
        ;

    HIOReadMailbox(&mailbox);  // Clear INT

    for (n = 0; ; n++)
    {
        char buf[80];

        sprintf(buf, "Hello, world(%4d)\n", n);
        Print(buf, strlen(buf));
    }

    return 0;
}
