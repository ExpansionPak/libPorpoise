/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     create-8icon.h

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/include/create-8icon.h $
    
    1     6/19/01 1:26p Hosogai
    Initial check-in
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   create-8icon
	 A simple program that creates a memory card file with 8 icons
 *---------------------------------------------------------------------------*/
#ifndef __CREATE8ICON_H__
#define __CREATE8ICON_H__

#define CARD_CHAN 1

#define ICON_RGB5A3_SIZE   (2 * CARD_ICON_WIDTH * CARD_ICON_HEIGHT)
#define ICON_CI_SIZE   (CARD_ICON_WIDTH * CARD_ICON_HEIGHT)
#define CLUT_SIZE 512
#define FILE_BLOCK_SIZE 3

/*---------------------------------------------------------------------------*
   Example memory card data structures
 *---------------------------------------------------------------------------*/
typedef struct CARDHeaderRGB5A3
{
	u8 Title[CARD_COMMENT_SIZE/2];
	u8 Description[CARD_COMMENT_SIZE/2];
	u8 Icons[CARD_ICON_MAX][ICON_RGB5A3_SIZE];
} CARDHeaderRGB5A3;

typedef struct CARDHeaderC8
{
	u8 Title[CARD_COMMENT_SIZE/2];
	u8 Description[CARD_COMMENT_SIZE/2];
	u8 Icons[CARD_ICON_MAX][ICON_CI_SIZE];
	u8 CLUT[CLUT_SIZE];
} CARDHeaderC8;

#endif