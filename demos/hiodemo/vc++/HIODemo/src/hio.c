//
// USB2EXI DLL Sample program.
//
// build:  make.bat
//
//
/*---------------------------------------------------------------------------*
  Project:  HIO test - Host side
  File:     hio.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/hiodemo/vc++/HIODemo/src/hio.c $
    
    4     3/05/01 8:09p Hashida
    Fixed paths.
    
    3     3/05/01 11:46a Hashida
    Changed path of hio.h
    
    2     12/20/00 9:07p Dante
    NCL modification
    
    1     11/27/00 3:47p Tian
    Initial checkin - based on code from Shiki 
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#endif
#include <dolphin/hio.h>

char Buffer[4096];

s32 Chan = -1;

void WINAPI Callback(void)
{

}

BOOL WINAPI EnumCallback(s32 chan)
{
    Chan = chan;
    return (0 <= Chan && Chan < 128) ? FALSE : TRUE;
}

int main(int argc, char* argv[])
{
    u32     mailboxIn;
    u32     mailboxOut = 0;
    u32     status;

#ifdef _MSC_VER
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    //
    // Initialize USB2EXI
    //
    if (!HIOEnumDevices(EnumCallback))
    {
        fprintf(stderr, "HIOEnumDevices failed.\n");
        return 1;
    }
    if (Chan == -1 || !HIOInit(Chan, Callback))
    {
        fprintf(stderr, "No USB/IF device exists.\n");
        return 1;
    }

    fprintf(stderr, "Hit any key to stop.\n");
    while (!_kbhit())
    {
        //
        // Signal mail box
        //
        if (!HIOWriteMailbox(mailboxOut))
            break;

        //
        // Wait for mail box
        //
        do {
            if (_kbhit())
            {
                return 0;
            }
            HIOReadStatus(&status);
        } while (!(status & HIO_STATUS_RX));
        if (!HIOReadMailbox(&mailboxIn))
            break;

        //
        // Read SRAM
        //
        if (!HIORead(0x4000, Buffer, sizeof Buffer))
            break;

        //
        // Copy Out
        //
        fprintf(stdout, "%.*s", sizeof Buffer, Buffer);
    }

    return 0;
}
