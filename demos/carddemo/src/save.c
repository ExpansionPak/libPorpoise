/*---------------------------------------------------------------------------*
  Project:  CARD API save demo
  File:     save.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/src/save.c $
    
    2     9/03/01 20:23 Shiki
    Modified to use slot B.

    1     6/14/01 11:36a Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

#define OFFSET(n, a)    (((u32) (n)) & ((a) - 1))
#define TRUNC(n, a)     (((u32) (n)) & ~((a) - 1))
#define ROUND(n, a)     (((u32) (n) + (a) - 1) & ~((a) - 1))

#define CARD_CHAN   1

u8 CardWorkArea[CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);

static void* LoadFile(char* name, s32* length)
{
    DVDFileInfo fileInfo;
    BOOL        result;
    void*       data;

    result = DVDOpen(name, &fileInfo);
    if (!result)
    {
        return NULL;
    }
    *length = (s32) OSRoundUp32B(DVDGetLength(&fileInfo));
    data = OSAllocFromArenaLo((u32) *length, 32);
    result = DVDRead(&fileInfo, data, *length, 0);
    if (!result)
    {
        return NULL;
    }
    DVDClose(&fileInfo);

    return data;
}

int main(int argc, char* argv[])
{
    s32          result;
    CARDFileInfo fileInfo;
    CARDStat     stat;
    u32          sectorSize;
    s32          length;
    void*        data;

    OSInit();
    DVDInit();
    CARDInit();

    if (argc != 2)
    {
        OSHalt("usage: save filename");
    }

    data = LoadFile(argv[1], &length);
    if (data == NULL)
    {
        OSHalt("error: could not load file from disk");
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

    // Adjust length
    length = (s32) ROUND(length, sectorSize);

    // Delete
    OSReport("CARDDelete ");
    result = CARDDelete(CARD_CHAN, argv[1]);
    OSReport("done. (%d)\n", result);

    // Create
    OSReport("CARDCreate ");
    result = CARDCreateAsync(CARD_CHAN, argv[1], (u32) length, &fileInfo, 0);
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

    OSReport("CARDWrite ");
    result = CARDWriteAsync(&fileInfo, data, length, 0, 0);
    if (result < 0)
    {
        OSReport("failed. (%d)\n", result);
        return 1;
    }
    while ((result = CARDGetResultCode(CARD_CHAN)) == CARD_RESULT_BUSY)
    {
        ;
    }
    if (result < 0)
    {
        OSReport("failed. (%d)\n", result);
        return 1;
    }
    OSReport("done. (%d)\n", result);

    // Close
    CARDClose(&fileInfo);

    // Unmount
    CARDUnmount(CARD_CHAN);

    OSReport("Done.\n");

    return 0;
}
