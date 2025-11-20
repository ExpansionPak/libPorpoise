/*---------------------------------------------------------------------------*
  Project:  Dolphin OS Overview - Cache Demo
  File:     cachedemo.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /dolphin/build/demos/osoverview/src/cachedemo.c $
    
    1     6/04/99 3:04p Tianli01
    Initial Checkin

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

void main (void)
{
    u32* cachedAddress;
    u32* uncachedAddress;

    OSInit();
    
    cachedAddress   = (u32*)OSGetArenaLo();
    uncachedAddress = OSCachedToUncached(cachedAddress);

    OSReport("STORE EXAMPLE\n");
    *cachedAddress 	= 0xFFFF;
    *uncachedAddress	= 0xAAAA;

    OSReport("Cache copy           = 0x%x\n", *cachedAddress);
    OSReport("Physical memory copy = 0x%x\n", *uncachedAddress);
	
    DCStoreRange(cachedAddress, sizeof(u32));
	
    OSReport("After STORE, Cache copy           = 0x%x\n", 
	     *cachedAddress);
    OSReport("After STORE, Physical memory copy = 0x%x\n",
	     *uncachedAddress);

    OSReport("\nINVALIDATE EXAMPLE\n");
    *cachedAddress 	= 0xFFFF;
    *uncachedAddress	= 0xAAAA;

    OSReport("Cache copy           = 0x%x\n", *cachedAddress);
    OSReport("Physical memory copy = 0x%x\n", *uncachedAddress);
	
    DCInvalidateRange(cachedAddress, sizeof(u32));
	
    OSReport("After INVALIDATE, Cache copy           = 0x%x\n",
	     *cachedAddress);
    OSReport("After INVALIDATE, Physical memory copy = 0x%x\n",
	     *uncachedAddress);

    OSHalt("Demo complete");
}
