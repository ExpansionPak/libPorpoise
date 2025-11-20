/*---------------------------------------------------------------------------*
  Project:  CARD API stat test
  File:     list.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/src/list.c $
    
    4     01/04/23 14:56 Shiki
    Fixed OSReport().

    3     01/04/23 14:40 Shiki
    Removed CARDStat.gameVersion.

    2     01/02/22 13:01 Shiki
    Added support for multiple sector sizes.

    1     11/28/00 2:13p Tian
    Moved from card tests directories.

    7     10/31/00 4:49p Shiki
    Modified to call CARDCheck().

    6     9/14/00 8:17p Shiki
    Revised to use localtime().

    5     9/08/00 6:30p Shiki
    Fixed to check mount result code more strictly.

    4     9/06/00 8:23p Shiki
    Revised to use EXIProbe() first.

    3     8/10/00 4:39p Shiki
    Modified CARDStat.length from u8 to u32.

    2     8/08/00 5:23p Shiki
    Improved the output format.

    1     7/14/00 4:16p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

#define CARD_CHAN   1

u8 CardWorkArea[CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);

int main(void)
{
    s32            result;
    CARDStat       stat;
    u16            fileNo;
    s32            byteNotUsed;
    s32            filesNotUsed;
    u32            sectorSize;
    OSCalendarTime ct;

    OSInit();
    CARDInit();

    // Probe
    OSReport("Insert card in slot %c.\n", "AB"[CARD_CHAN]);
    while (!CARDProbe(CARD_CHAN))
    {
        ;
    }

    // Mount
    OSReport("CARDMount ");
    result = CARDMountAsync(CARD_CHAN, CardWorkArea, 0, 0);
    if (result < 0)
    {
        OSReport("failed. (%d)\n", result);
        return 1;
    }
    while ((result = CARDGetResultCode(CARD_CHAN)) == CARD_RESULT_BUSY)
    {
        ;
    }
    switch (result)
    {
      case CARD_RESULT_READY:
      case CARD_RESULT_BROKEN:
        OSReport("done. (%d)\n", result);
        OSReport("CARDCheck ");
        result = CARDCheckAsync(CARD_CHAN, 0);
        if (result < 0)
        {
            CARDUnmount(CARD_CHAN);
            OSReport("failed. (%d)\n", result);
            return 1;
        }
        while ((result = CARDGetResultCode(CARD_CHAN)) == CARD_RESULT_BUSY)
        {
            ;
        }
        break;
    }
    switch (result)
    {
      case CARD_RESULT_READY:
        OSReport("done. (%d)\n", result);
        break;
      case CARD_RESULT_BROKEN:
      case CARD_RESULT_ENCODING:
        CARDUnmount(CARD_CHAN);
        OSReport("failed. (%d)\n", result);
        return 1;
        break;
      default:
        OSReport("failed. (%d)\n", result);
        return 1;
        break;
    }

    // List
    OSReport("no. game com. length   fileName                         date     time\n");
    for (fileNo = 0; fileNo < CARD_MAX_FILE; fileNo++)
    {
        result = CARDGetStatus(CARD_CHAN, fileNo, &stat);
        if (result < 0)
        {
            continue;
        }
        OSReport("%3d ",       fileNo);
        OSReport("%-4.4s ",    stat.gameName);
        OSReport("%-2.2s   ",  stat.company);
        OSReport("%-8d ",      stat.length);
        OSReport("%-32.32s ",  stat.fileName);

        OSTicksToCalendarTime(OSSecondsToTicks((OSTime) stat.time), &ct);
        OSReport("%02.2d/%02.2d/%02.2d %02.2d:%02.2d:%02.2d\n",
                 ct.mon + 1, ct.mday, ct.year % 100,
                 ct.hour, ct.min, ct.sec);
    }

    // Sector size
    result = CARDGetSectorSize(CARD_CHAN, &sectorSize);
    if (result < 0)
    {
        OSReport("CARDGetSectorSize() failed. (%d)\n", result);
        CARDUnmount(CARD_CHAN);
        return 1;
    }
    OSReport("Sector size %u bytes.\n", sectorSize);

    // FreeBlocks
    OSReport("CARDFreeBlocks ");
    result = CARDFreeBlocks(CARD_CHAN, &byteNotUsed, &filesNotUsed);
    if (result < 0)
    {
        OSReport("failed. (%d)\n", result);
        CARDUnmount(CARD_CHAN);
        return 1;
    }
    OSReport("done. (%d)\n", result);
    OSReport("%d bytes (%d blocks) free.\n%d files free.\n", byteNotUsed, byteNotUsed / sectorSize, filesNotUsed);

    // Unmount
    CARDUnmount(CARD_CHAN);

    return 0;
}
