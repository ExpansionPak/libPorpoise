/*---------------------------------------------------------------------------*
  Project:  CARD utilities
  File:     CARDUtil.h

  Copyright 2000-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/include/cardutil.h $
    
    5     7/13/01 12:03 Shiki
    Moved demo specified CARDUTIL_CMD_ to listdemo.c.

    4     5/24/01 8:53a Shiki
    Removed #include <private/CARDDir.h>.

    3     5/18/01 5:07p Shiki
    Added CardUtilGetProgress().

    2     01/04/25 14:33 Shiki
    Added size param to CardUtilDrawIcon().

    1     01/04/23 17:18 Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <string.h>
#include <demo.h>
#include <dolphin/card.h>

static void DrawIcon(s16 x, s16 y, void* image, void* tlut, u16 width, u16 height, int format);

enum
{
    CARDUTIL_CMD_NONE = 0,
    CARDUTIL_CMD_MOUNT,
    CARDUTIL_CMD_UNMOUNT,
    CARDUTIL_CMD_FORMAT,
    CARDUTIL_CMD_LIST,
    CARDUTIL_CMD_ERASE,
    CARDUTIL_CMD_OPEN,
    CARDUTIL_CMD_SAVE
};

typedef struct CardUtilDirent
{
    u8          buffer[2 * (CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT +
                       CARD_ICON_MAX * CARD_ICON_WIDTH * CARD_ICON_HEIGHT) +
                       CARD_READ_SIZE];

    char        comment[CARD_COMMENT_SIZE];

    int         fileNo;
    CARDStat    stat;

    int         cFrame;                         // total # of icon animation frames
    int         nFrame[2 * CARD_ICON_MAX - 2];  // start frame number of each icon
    int         iIcon[2 * CARD_ICON_MAX - 2];   // icon # of each icon

    u8          padding[28];
} CardUtilDirent;   // 32 byte aligned

void CardUtilInit      ( void* stackBase, u32 stackSize, OSPriority prio );

s32 CardUtilResultCode ( void );
s32 CardUtilMount      ( s32 slot, void* workArea );
s32 CardUtilUnmount    ( s32 slot );
s32 CardUtilList       ( s32 slot, CardUtilDirent* directory );
s32 CardUtilFormat     ( s32 slot );
s32 CardUtilErase      ( s32 slot, s32 fileNo );
s32 CardUtilOpen       ( s32 slot, s32 fileNo, void* buffer );
s32 CardUtilSave       ( s32 slot, CARDStat* stat, void* buffer );
s32 CardUtilGetProgress( s32 slot );

s32 CardUtilByteNotUsed  ( void );
s32 CardUtilBlocksNotUsed( void );
s32 CardUtilFilesNotUsed ( void );
s32 CardUtilSectorSize   ( void );

s32 CardUtilLockDirectory   ( void );
void CardUtilUnlockDirectory( void );
s32 CardUtilNumFiles        ( void );

void CardUtilDrawIcon        ( int x, int y, int size, void* image, void* tlut, u16 width, u16 height, int format );
void CardUtilDrawAnimatedIcon( CardUtilDirent* ent, int x, int y, int size );
