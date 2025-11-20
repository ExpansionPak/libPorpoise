/*---------------------------------------------------------------------------*
  Project:  CARD API create test
  File:     create.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/src/create.c $
    
    4     01/04/23 14:40 Shiki
    Removed CARDStat.gameVersion.

    3     01/02/22 13:00 Shiki
    Added support for multiple sector sizes.

    2     12/08/00 7:01p Shiki
    Fixed to call CARDCheck(), etc.

    1     11/28/00 2:13p Tian
    Moved from card tests directories.

    3     9/08/00 6:34p Shiki
    Fixed to check mount result code more strictly.

    2     9/06/00 8:23p Shiki
    Revised to use EXIProbe() first.

    1     7/14/00 4:16p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

#define CARD_CHAN   1

u8 CardWorkArea[CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);

int main(int argc, char* argv[])
{
    s32          result;
    CARDFileInfo fileInfo;
    CARDStat     stat;
    u32          sectorSize;

    OSInit();
    CARDInit();

    if (argc != 2)
    {
        OSReport("Usage: create filename\n");
        return 1;
    }

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

    // Sector size
    result = CARDGetSectorSize(CARD_CHAN, &sectorSize);
    if (result < 0)
    {
        OSReport("CARDGetSectorSize() failed. (%d)\n", result);
        return 1;
    }
    OSReport("Sector size %d bytes.\n", sectorSize);

    // Create
    OSReport("CARDCreate ");
    result = CARDCreateAsync(CARD_CHAN, argv[1], sectorSize, &fileInfo, 0);
    if (result < 0)
    {
        OSReport("failed. (%d)\n", result);
        return 1;
    }
    while ((result = CARDGetResultCode(CARD_CHAN)) == CARD_RESULT_BUSY)
    {
        ;
    }
    OSReport("done. (%d:%d:%d)\n", result, fileInfo.chan, fileInfo.fileNo);

    // Stat
    OSReport("CARDGetStatus ");
    result = CARDGetStatus(CARD_CHAN, fileInfo.fileNo, &stat);
    if (result < 0)
    {
        OSReport("failed. (%d)\n", result);
        return 1;
    }
    OSReport("gameName %.4s\n",  stat.gameName);
    OSReport("company %.2s\n",   stat.company);
    OSReport("length %d\n",      stat.length);
    OSReport("fileName %.32s\n", stat.fileName);
    OSReport("time %d\n",        stat.time);

    // Close
    CARDClose(&fileInfo);

    // Unmount
    CARDUnmount(CARD_CHAN);

    return 0;
}
