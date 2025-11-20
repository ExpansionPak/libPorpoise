/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     create-bnr-4icon.h

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/include/create-bnr-4icon.h $
    
    1     6/19/01 1:26p Hosogai
    Initial check-in
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   create-bnr-4icon
	 A simple program that creates a memory card file with banner & 4 icons
 *---------------------------------------------------------------------------*/
#ifndef __CREATEBNR4ICON_H__
#define __CREATEBNR4ICON_H__

#define CARD_CHAN 1

#define ICON_RGB5A3_SIZE   (2 * CARD_ICON_WIDTH * CARD_ICON_HEIGHT)
#define ICON_CI_SIZE   (CARD_ICON_WIDTH * CARD_ICON_HEIGHT)
#define CARD_BANNER_SIZE (CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT*2)
#define CLUT_SIZE 512
#define FILE_BLOCK_SIZE 8

typedef struct CARDHeaderRGB5A3
{
	u8 Title[CARD_COMMENT_SIZE/2];
	u8 Description[CARD_COMMENT_SIZE/2];
	u8 Banner[CARD_BANNER_SIZE];
	u8 Icons[4][ICON_RGB5A3_SIZE];
} CARDHeaderRGB5A3;

#endif