/*---------------------------------------------------------------------------*
  Project:  CARD API attach test
  File:     attach.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/src/format.c $
    
    2     01/02/22 13:02 Shiki
    Added support for multiple sector sizes.

    1     11/28/00 2:13p Tian
    Moved from card tests directories.

    2     9/06/00 8:23p Shiki
    Revised to use EXIProbe() first.

    1     7/14/00 4:16p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

#define CARD_CHAN   1

u8 CardWorkArea[CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);

int main(void)
{
    s32 result;
    u16 encode;
    u16 size;
    u32 sectorSize;

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
        OSReport("done. (%d)\n", result);
        break;
      case CARD_RESULT_BROKEN:
      case CARD_RESULT_ENCODING:
        OSReport("failed. (%d)\n", result);
        break;
      default:
        OSReport("failed. (%d)\n", result);
        return 1;
        break;
    }

    // Format
    OSReport("CARDFormat ");
    result = CARDFormatAsync(CARD_CHAN, 0);
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

    result = CARDGetEncoding(CARD_CHAN, &encode);
    if (0 <= result)
    {
        OSReport("Character encoding: %s(%d)\n",
                 encode == CARD_ENCODE_SJIS ? "SJIS" : "ANSI", encode);
    }

    // Memory size
    result = CARDGetMemSize(CARD_CHAN, &size);
    if (result < 0)
    {
        OSReport("CARDGetMemSize() failed. (%d)\n", result);
        return 1;
    }
    OSReport("%u Mbits memory card.\n", size);

    // Sector size
    result = CARDGetSectorSize(CARD_CHAN, &sectorSize);
    if (result < 0)
    {
        OSReport("CARDGetSectorSize() failed. (%d)\n", result);
        return 1;
    }
    OSReport("Sector size %u bytes.\n", sectorSize);

    // Unmount
    CARDUnmount(CARD_CHAN);

    return 0;
}
