/*---------------------------------------------------------------------------*
  Project:  CARD icon viewer
  File:     listdemo.c

  Copyright 2000-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/src/listdemo.c $
    
    23    11/21/01 21:41 Shiki
    Removed obsolete OS function calls

    22    8/23/01 14:21 Shiki
    Modified to use OSResetSystem for restart.

    21    7/13/01 12:01 Shiki
    Implemented more friendly free space check routines.

    20    7/12/01 21:24 Shiki
    Added support for Japanese menu.

    19    01/06/25 14:38 Shiki
    Modified ProbeAnimTick() using CARDProbeEx().

    18    01/06/22 11:05 Shiki
    Removed redundant VIWaitForRetrace().

    17    01/06/19 13:49 Shiki
    Minor fix.

    16    5/18/01 5:30p Shiki
    Modified to show progress bar.

    15    5/17/01 8:22p Shiki
    Revised using new OSResetSystem function.

    14    5/11/01 5:05p Shiki
    Revised using new DEMO API interface.

    13    01/04/26 10:38 Shiki
    Set banner background color to black.

    12    01/04/25 14:57 Shiki
    Merged US console font size setting.

    11    01/04/25 14:35 Shiki
    Revised using new DEMO API.

    10    01/04/25 11:40 Shiki
    Modified to load disk banner file.

    9     01/04/23 17:26 Shiki
    Revised using cardutil.c

    8     01/04/10 16:50 Shiki
    Added support for icon animation speed.

    7     01/03/06 17:19 Shiki
    Modified InitCont() to support controller recalibration.

    6     01/02/22 13:10 Shiki
    Added support for multiple sector sizes.

    5     01/01/23 9:35 Shiki
    Fixed typo.

    4     12/12/00 12:39a Shiki
    Bug fix.

    3     12/11/00 11:13p Shiki
    Revised to display icons.

    3     8/10/00 4:39p Shiki
    Modified CARDStat.length from u8 to u32.

    2     8/08/00 5:23p Shiki
    Improved the output format.

    1     7/14/00 4:16p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <stdarg.h>
#include <string.h>
#include <demo.h>
#include <dolphin.h>
#include <dolphin/dvd/DVDBanner.h>
#include <dolphin/os/OSReset.h>
#include "cont.h"
#include "cardutil.h"

GXColor Black = {   0,   0,   0, 0 };
GXColor Blue  = {   0,   0, 192, 0 };
GXColor Red   = { 255,   0,   0, 0 };
GXColor Green = {   0, 224,   0, 0 };
GXColor White = { 255, 255, 255, 0 };

enum
{
    STR_Null,
    STR_Cancel,
    STR_Select,
    STR_Confirm,
    STR_Finish,
    STR_Yes,
    STR_No,
    STR_Save,
    STR_Erase,

    STR_Ignore,
    STR_Menu,

    STR_Mounting,

    STR_Saving,
    STR_Saved,
    STR_NotSaved,
    STR_MayNotSaved,

    STR_Erasing,
    STR_Erased,
    STR_MayNotErased,

    STR_Formatting,
    STR_Formatted,
    STR_MayNotFormatted,

    STR_Format,

    STR_InsSpace,

    STR_MemoryCard,
    STR_NotMemoryCard,
    STR_Nothing,
    STR_Broken,
    STR_Open,
    STR_Stat,
    STR_End
};

char* StringResource[2][STR_End] =
{
    // US English version
    {
        NULL,
        "Cancel",
        "Select",
        "Confirm",
        "Finish",
        "Yes",
        "No",
        "Save",
        "Erase",

        "Continue",
        "Go to GameCube main menu",

        "Loading...",

        "Saving...",
        "The data was saved.",
        "The data has not been saved.",
        "The data may not have been saved.",

        "Erasing...",
        "The data was erased.",
        "The data may not have been erased.",

        "Formatting...",
        "The Memory Card was formatted.",
        "The Memory Card may not have been formatted.",

        "The Memory Card must be formatted. \n"
        "If the Memory Card is formatted, all saved \n"
        "data will be erased. \n"
        "Format it now?",

        "The Memory Card has no space available or \n"
        "it has too many files saved to it to play \n"
        "the game. Game data cannot be saved.",

        "Inserted (%dMbits, %dK sector)",
        "Inserted (not memory card)",
        "Nothing is inserted",
        "The object inserted cannot be used.",
        "<Open>",
        "%d blocks free. %d files free.",
    },

    // Japanese version
    {
        NULL,
        "戻る",
        "選択",
        "決定",
        "終了",
        "はい",
        "いいえ",
        "保存",
        "消去",

        "保存しないで進める",
        "ゲームキューブ メインメニューへ",

        "読み込み中です。",

        "保存中です。",
        "保存しました。",
        "保存できませんでした。",
        "保存に失敗した可能性があります。",

        "消去中です。",
        "消去しました。",
        "消去に失敗した可能性があります。",

        "初期化中です。",
        "初期化しました。",
        "初期化に失敗した可能性があります。",

        "メモリーカードは初期化が必要です。\n"
        "初期化するとメモリーカード内の全ファイルが\n"
        "消えます。\n"
        "初期化しますか？",

        "メモリーカードにゲームに必要な空き容量がないか、\n"
        "ファイル数の制限を超えてしまいます。\n"
        "このまま始めると、保存することができません。",

        "メモリーカード (%dMbits, %dKB セクター)",
        "スロットにささっているものは使えません。",
        "何もささっていません。",
        "スロットにささっているものは使えません。",
        "<空き>",
        "空き容量 %d ブロック。空きファイル数 %d。",
    },
};

#define GetString(str)  (StringResource[OSGetFontEncode()][str])

//
// Define save data size of the game in bytes
//
#define FILE_SIZE   (32 * 1024)

typedef struct Interface
{
    void (*DrawTick)(void);
    void (*AnimTick)(void);
} Interface;

enum
{
    STATE_PROBE = 0,                    // State
    STATE_LIST,

    SUBSTATE_SELECT = 0,                // SubState
    SUBSTATE_COMMAND,
    SUBSTATE_VERIFY,
    SUBSTATE_BUSY,
    SUBSTATE_WAIT,

    YES = 0,
    NO,

    CARDUTIL_CMD_CHECKSPACE = 0x1000,
    CARDUTIL_CMD_IGNORE,
    CARDUTIL_CMD_MAIN_MENU,
    CARDUTIL_CMD_ERROR
};

#define LIST_ROW    8                   // # of rows in the list screen

// Work area for card threads and card API
static u8             CardStack[8192]                  ATTRIBUTE_ALIGN(32);
static u8             CardWorkArea[CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);

// Save data template
static DVDBanner*     Banner;
static CARDStat       CardStatTemplate; // CARDStat for save file
static void*          CardData;         // Pointer to file save data
static TEXPalettePtr  TplBanner;        // Card banner texture image
static TEXPalettePtr  TplIcons;         // Card icon texture image

// Program state
static int    Slot     = 0;             // Current memory card slot
static int    State    = STATE_PROBE;   // Current program state
static int    SubState;                 // Current program substate
static s32    Command;                  // Current command
static OSTime BeginWaitAt;              // Start time of the SUBSTATE_WAIT substate
static int    YesNo;                    // 0: Yes, No: 1

// Directory
static CardUtilDirent Directory[CARD_MAX_FILE]         ATTRIBUTE_ALIGN(32);
static BOOL   List;                     // TRUE if Directory is valid
static int    Offset;                   // The first line number in the directory on screen
static int    Current;                  // The currently selected entry number
static s32    NumEntries;               // # of entries in Directory

static void ProbeDrawTick(void);
static void ProbeAnimTick(void);
static void ListDrawTick(void);
static void ListAnimTick(void);

static Interface InterfaceTable[] =
{
    ProbeDrawTick,  ProbeAnimTick,
    ListDrawTick,   ListAnimTick,
};

static BOOL   ForceMenu;                // TRUE if goes back to GameCube IPL main menu

OSFontHeader* FontData;                 // Pointer to ROM font data

/*---------------------------------------------------------------------------*
  Name:         DrawRectangle

  Description:  Draws rectangle
 *---------------------------------------------------------------------------*/
static void DrawRectangle(int x, int y, int cx, int cy, GXColor color)
{
    // Set rendering mode
    GXSetNumChans(1);           // # of color channels
    GXSetChanCtrl(GX_COLOR0,
               GX_FALSE,        // passed through
               GX_SRC_REG,      // amb source
               GX_SRC_REG,      // mat source
               GX_LIGHT_NULL,   // light mask
               GX_DF_NONE,      // diffuse function
               GX_AF_NONE );    // atten   function
    GXSetChanMatColor(GX_COLOR0, color);

    GXSetNumTexGens(0);         // # of Tex gens
    GXSetNumTevStages(1);       // # of Tev Stage
    GXSetTevOrder(GX_TEVSTAGE0,
                  GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    // Draw rectangle
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16((s16) (x),      (s16) (y),      0);
        GXPosition3s16((s16) (x + cx), (s16) (y),      0);
        GXPosition3s16((s16) (x + cx), (s16) (y + cy), 0);
        GXPosition3s16((s16) (x),      (s16) (y + cy), 0);
    GXEnd( );

    // Restore rendering mode as same as DEMOInitCaption()
    GXSetNumChans(0);
    GXSetNumTexGens(0);         // # of Tex gens
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
}

/*---------------------------------------------------------------------------*
  Name:         DrawMenu

  Description:  ex. DrawMenu(x, y, color, current, item0, item1, NULL);

                item must be a character string
 *---------------------------------------------------------------------------*/
static int DrawMenu(int x, int y, GXColor color, int current, ...)
{
    va_list ap;
    char*   item;
    int     cx;
    s32     width;
    int     i;
    s16     size;
    s16     space;

    cx = 0;
    va_start(ap, current);
    while ((item = va_arg(ap, char*)) != NULL)
    {
        width = DEMOGetRFTextWidth(item);
        if (cx < width)
        {
            cx = width;
        }
    }
    va_end(ap);

    i = 0;
    DEMOGetROMFontSize(&size, &space);
    y += FontData->leading * size / FontData->cellWidth;
    va_start(ap, current);
    while ((item = va_arg(ap, char*)) != NULL)
    {
        DEMOSetFontType(DM_FT_OPQ);
        DrawRectangle(x, y - FontData->ascent * size / FontData->cellWidth, cx, FontData->leading * size / FontData->cellWidth,
                      (i++ == current) ? color : Black);
        DEMOSetFontType(DM_FT_XLU);
        DEMORFPrintf((s16) x, (s16) y, 0, item);
        y += DEMOGetRFTextHeight(item);
    }
    va_end(ap);

    return cx;
}

/*---------------------------------------------------------------------------*
  Name:         DrawMessage

  Description:  Draws message
 *---------------------------------------------------------------------------*/
static int DrawMessage(int x, int y, GXColor color, char* message)
{
    int cy;
    int cx;
    s16 size;
    s16 space;

    cx = DEMOGetRFTextWidth(message);
    cy = DEMOGetRFTextHeight(message);

    DEMOSetFontType(DM_FT_OPQ);
    DrawRectangle(x, y, cx, cy, color);

    DEMOGetROMFontSize(&size, &space);
    y += FontData->leading * size / FontData->cellWidth;
    DEMOSetFontType(DM_FT_XLU);
    DEMORFPrintf((s16) x, (s16) y, 0, message);

    return cx;
}

/*---------------------------------------------------------------------------*
  Name:         DrawUsage

  Description:  Draws usages of buttons
 *---------------------------------------------------------------------------*/
static int DrawUsage(int slot, char* commandA, char* commandB, char* message)
{
    int width;

    width = DEMORFPrintf(16, 45, 0, "SLOT-%c: ", "AB"[slot]);
    if (commandB)
    {
        width += DEMORFPrintf((s16) (16 + width), 45, 0, "<B> %s ", commandB);
    }
    if (commandA)
    {
        width += DEMORFPrintf((s16) (16 + width), 45, 0, "<A> %s ", commandA);
    }
    if (message)
    {
        width += DEMORFPrintf((s16) (16 + width), 45, 0, message);
    }
    return width;
}

/*---------------------------------------------------------------------------*
  Name:         GetBlockSize

  Description:  Returns the number of required blocks to save the specified
                size of data in the current memory card.
 *---------------------------------------------------------------------------*/
static s32 GetBlockSize(s32 size)
{
    return (size + CardUtilSectorSize() - 1) / CardUtilSectorSize();
}

/*---------------------------------------------------------------------------*
  Name:         IsSpaceAvailable

  Description:  Checks if a file of the specified size can be created in
                the current memory card.

  Note:         This function is for reliable file save. It requires the
                same number of temporary free blocks as the number of saved
                file will consume as well as one free file directory entry.

  Arguments:    size        file size in bytes

  Returns:      TRUE is space is available. Otherwise FALSE.
 *---------------------------------------------------------------------------*/
static BOOL IsSpaceAvailable(s32 size)
{
    s32 numFiles;
    s32 numBlocks;

    numFiles = CardUtilNumFiles();
    numBlocks = GetBlockSize(size);
    if (CardUtilFilesNotUsed() == 0 ||
        numFiles == 0 && CardUtilBlocksNotUsed() < 2 * numBlocks ||
        0 < numFiles && CardUtilBlocksNotUsed() < numBlocks)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         ProbeDrawTick

  Description:  Draw memory card slot selection screen
 *---------------------------------------------------------------------------*/
static void ProbeDrawTick(void)
{
    GXRenderModeObj* rmp;
    s32              chan;

    rmp = DEMOGetRenderModeObj();
    DEMOInitCaption(DM_FT_XLU, (s16) rmp->fbWidth, (s16) rmp->efbHeight);

    DEMOSetFontType(DM_FT_RVS);
    DrawUsage(Slot, GetString(STR_Select), GetString(STR_Null), NULL);

    for (chan = 0; chan < 2; ++chan)
    {
        s32 memSize;
        s32 sectorSize;
        int width;

        DEMOSetFontType((Slot == chan) ? DM_FT_RVS : DM_FT_XLU);
        width = DEMORFPrintf(16, (s16) ((chan == 0) ? 75 : 105), 0, " SLOT-%c: ", "AB"[chan]);
        switch (CARDProbeEx(chan, &memSize, &sectorSize))
        {
          case CARD_RESULT_READY:
            DEMORFPrintf((s16) (16 + width), (s16) ((chan == 0) ? 75 : 105), 0, GetString(STR_MemoryCard),
                         memSize, sectorSize / 1024);
            break;
          case CARD_RESULT_WRONGDEVICE:
            DEMORFPrintf((s16) (16 + width), (s16) ((chan == 0) ? 75 : 105), 0, GetString(STR_NotMemoryCard));
            break;
          default:
            DEMORFPrintf((s16) (16 + width), (s16) ((chan == 0) ? 75 : 105), 0, GetString(STR_Nothing));
            break;
        }
    }

    //
    // Draw DVD banner data
    //
    // Note: In GameCube file viewer, not all the characters appears on screen.
    //       Effective areas are shown as green boxes.
    //

    DEMOSetFontType(DM_FT_OPQ);
    DrawRectangle(16, 330, 2 * DVD_BANNER_WIDTH, 2 * DVD_BANNER_HEIGHT, Black);
    CardUtilDrawIcon(16, 330, 2 * DVD_BANNER_WIDTH,
                     Banner->image,
                     NULL,
                     DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT,
                     CARD_STAT_ICON_RGB5A3);

    DEMOSetFontType(DM_FT_XLU);

    // 'titl', gam_lay 18h, -1, 1ca0h
    DEMOSetFontType(DM_FT_OPQ);
    DrawRectangle(16, 200 - FontData->ascent, 458, FontData->leading, Green);
    DEMOSetFontType(DM_FT_XLU);
    DEMOSetROMFontSize(24, -1);
    DEMORFPrintf(16, 200, 0, "%-64.64s",    Banner->longTitle);

    // 'makr', gam_lay 14h, -1, 1ca0h
    DEMOSetFontType(DM_FT_OPQ);
    DrawRectangle(16, 224 - FontData->ascent, 458, FontData->leading, Green);
    DEMOSetFontType(DM_FT_XLU);
    DEMOSetROMFontSize(20, -1);
    DEMORFPrintf(16, 224, 0, "%-64.64s",    Banner->longMaker);

    // 'info', gam_lay 14h, -1, 1ca0h * 2
    DEMOSetFontType(DM_FT_OPQ);
    DrawRectangle(16, 250 - FontData->ascent, 458, 2 * FontData->leading + 15, Green);
    DEMOSetFontType(DM_FT_XLU);
    DEMOSetROMFontSize(20, -1);
    DEMORFPutsEx(16, 265, 0, (char*) Banner->comment, 458, sizeof Banner->comment);

    // 'game', lay_gam 12h, -1, 0c00h (jp)
    // 'game', lay_gam 10h, -2, 0c00h (us)
    DEMOSetFontType(DM_FT_OPQ);
    DrawRectangle(220, 354 - FontData->ascent, 192, FontData->leading, Green);
    DEMOSetFontType(DM_FT_XLU);
    if (OSGetFontEncode() == OS_FONT_ENCODE_SJIS)
    {
        DEMOSetROMFontSize(18, -1);
    }
    else
    {
        DEMOSetROMFontSize(16, -2);
    }
    DEMORFPrintf(220, 354, 0, "%-32.32s",   Banner->shortTitle);

    // 'makr', lay_gam 12h, -1, 0c00h (jp)
    // 'makr', lay_gam 10h, -2, 0c00h (us)
    DEMOSetFontType(DM_FT_OPQ);
    DrawRectangle(220, 378 - FontData->ascent, 192, FontData->leading, Green);
    DEMOSetFontType(DM_FT_XLU);
    if (OSGetFontEncode() == OS_FONT_ENCODE_SJIS)
    {
        DEMOSetROMFontSize(18, -1);
    }
    else
    {
        DEMOSetROMFontSize(16, -2);
    }
    DEMORFPrintf(220, 378, 0, "%-32.32s",   Banner->shortMaker);

    DEMOSetROMFontSize((s16) FontData->cellWidth, -1);
}

/*---------------------------------------------------------------------------*
  Name:         ProbeAnimTick

  Description:  Switches the current slot selection.
                Starts the mount operation if A button is pushed.
 *---------------------------------------------------------------------------*/
static void ProbeAnimTick(void)
{
    s32 memSize;
    s32 sectorSize;

    if (Conts[4].repeat & PAD_BUTTON_UP)
    {
        Slot = 0;
    }
    else if (Conts[4].repeat & PAD_BUTTON_DOWN)
    {
        Slot = 1;
    }
    else if ((Conts[4].down & PAD_BUTTON_A) &&
             CARDProbeEx(Slot, &memSize, &sectorSize) == CARD_RESULT_READY)
    {
        CardUtilMount(Slot, CardWorkArea);
        List     = FALSE;
        Offset   = 0;
        Current  = 0;
        State    = STATE_LIST;
        SubState = SUBSTATE_SELECT;

        // At first, the program should check the available space in the
        // memory card.
        Command  = CARDUTIL_CMD_CHECKSPACE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DrawProgressBar

  Description:  Draw progress bar
 *---------------------------------------------------------------------------*/
static void DrawProgressBar(void)
{
    DEMOSetFontType(DM_FT_OPQ);
    DrawRectangle(16, 250, 300,                           25, Black);
    DrawRectangle(16, 250, 3 * CardUtilGetProgress(Slot), 25, Green);
}

/*---------------------------------------------------------------------------*
  Name:         DrawInstructions

  Description:  Draw command instructions on screen.
 *---------------------------------------------------------------------------*/
static void DrawInstructions(void)
{
    s32 result;
    int width;

    DEMOSetROMFontSize((s16) FontData->cellWidth, -1);
    result = CardUtilResultCode();
    switch (Command)
    {
      case CARDUTIL_CMD_SAVE:
        DEMOSetFontType(DM_FT_RVS);
        switch (SubState)
        {
          case SUBSTATE_COMMAND:
            DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Cancel), NULL);
            DrawMenu(200, 200, Green, 0, GetString(STR_Save), GetString(STR_Erase), NULL);
            break;
          case SUBSTATE_VERIFY:
            DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Cancel), NULL);
            width = DrawMenu(200, 200, Green, 0, GetString(STR_Save), GetString(STR_Erase), NULL);
            DrawMenu(200 + 5 + width, 200, Green,
                     YesNo, GetString(STR_Yes), GetString(STR_No), NULL);
            break;
          case SUBSTATE_BUSY:
            DrawProgressBar();
            DEMOSetFontType(DM_FT_RVS);
            DrawUsage(Slot, GetString(STR_Null), GetString(STR_Null), GetString(STR_Saving));
            break;
          case SUBSTATE_WAIT:
            DrawProgressBar();
            DEMOSetFontType(DM_FT_RVS);
            DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Null), NULL);
            switch (result)
            {
              case CARD_RESULT_READY:
                DrawMessage(16, 200, Green, GetString(STR_Saved));
                break;
              case CARD_RESULT_NOENT:
              case CARD_RESULT_INSSPACE:
                DrawMessage(16, 200, Red,   GetString(STR_NotSaved));
                break;
              default:
                DrawMessage(16, 200, Red,   GetString(STR_MayNotSaved));
                break;
            }
            break;
        }
        break;

      case CARDUTIL_CMD_ERASE:
        DEMOSetFontType(DM_FT_RVS);
        switch (SubState)
        {
          case SUBSTATE_COMMAND:
            DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Cancel), NULL);
            DrawMenu(200, 200, Green, 1, GetString(STR_Save), GetString(STR_Erase), NULL);
            break;
          case SUBSTATE_VERIFY:
            DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Cancel), NULL);
            width = DrawMenu(200, 200, Green, 1, GetString(STR_Save), GetString(STR_Erase), NULL);
            DrawMenu(200 + 5 + width, 200 + FontData->leading, Green,
                     YesNo, GetString(STR_Yes), GetString(STR_No), NULL);
            break;
          case SUBSTATE_BUSY:
            DrawProgressBar();
            DEMOSetFontType(DM_FT_RVS);
            DrawUsage(Slot, GetString(STR_Null), GetString(STR_Null), GetString(STR_Erasing));
            break;
          case SUBSTATE_WAIT:
            DrawProgressBar();
            DEMOSetFontType(DM_FT_RVS);
            DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Null), NULL);
            switch (result)
            {
              case CARD_RESULT_READY:
                DrawMessage(16, 200, Green, GetString(STR_Erased));
                break;
              default:
                DrawMessage(16, 200, Red,   GetString(STR_MayNotErased));
                break;
            }
            break;
        }
        break;

      case CARDUTIL_CMD_FORMAT:
        switch (SubState)
        {
          case SUBSTATE_VERIFY:
            DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Cancel), NULL);
            DrawMessage(16, 75, Red, GetString(STR_Format));
            DrawMenu(200, 200, Green, YesNo, GetString(STR_Yes), GetString(STR_No), NULL);
            break;
          case SUBSTATE_BUSY:
            DrawProgressBar();
            DEMOSetFontType(DM_FT_RVS);
            DrawUsage(Slot, GetString(STR_Null), GetString(STR_Null), GetString(STR_Formatting));
            break;
          case SUBSTATE_WAIT:
            DrawProgressBar();
            DEMOSetFontType(DM_FT_RVS);
            DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Null), NULL);
            switch (result)
            {
              case CARD_RESULT_READY:
                DrawMessage(16, 200, Green, GetString(STR_Formatted));
                break;
              default:
                DrawMessage(16, 200, Red,   GetString(STR_MayNotFormatted));
                break;
            }
            break;
        }
        break;

      case CARDUTIL_CMD_CHECKSPACE:
      case CARDUTIL_CMD_IGNORE:
      case CARDUTIL_CMD_MAIN_MENU:
        // At the begining of the program, it should check if the player
        // has a memory card that can be used to save the game data.
        switch (SubState)
        {
          case SUBSTATE_SELECT: // mounting...
            DEMOSetFontType(DM_FT_RVS);
            DrawUsage(Slot, GetString(STR_Null), GetString(STR_Null), GetString(STR_Mounting));
            break;
          case SUBSTATE_COMMAND:
            DEMOSetFontType(DM_FT_RVS);
            DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Cancel), NULL);
            DrawMessage(16, 75, Red, GetString(STR_InsSpace));
            DrawMenu(200, 200, Green, Command - CARDUTIL_CMD_CHECKSPACE,
                     GetString(STR_Cancel), GetString(STR_Ignore), GetString(STR_Menu), NULL);
            break;
        }
        break;

      case CARDUTIL_CMD_ERROR:
        DrawUsage(Slot, GetString(STR_Confirm), GetString(STR_Null), NULL);
        DrawMessage(16, 69, Red, GetString(STR_Broken));
        break;

      default:
        DEMOSetFontType(DM_FT_RVS);
        DrawUsage(Slot, GetString(STR_Select), GetString(STR_Finish), NULL);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         ListDrawTick

  Description:  Draw card file directory
 *---------------------------------------------------------------------------*/
static void ListDrawTick(void)
{
    GXRenderModeObj* rmp;
    CardUtilDirent*  ent;
    int              i;
    s32              numFiles;
    s32              numBlocks;

    // Locks directory to prevents card utility thread updates directory behind
    numFiles = CardUtilLockDirectory();

    // Check if new files can be created.
    NumEntries = numFiles;
    numBlocks = GetBlockSize(FILE_SIZE);
    if (0 < CardUtilFilesNotUsed() &&
        2 * numBlocks <= CardUtilBlocksNotUsed())
    {
        // Last directory entry is used for creating a new data file.
        ++NumEntries;
    }

    rmp = DEMOGetRenderModeObj();
    DEMOInitCaption(DM_FT_XLU, (s16) rmp->fbWidth, (s16) rmp->efbHeight);

    // Updates Current as card utility thread may have deleted directory entries
    if (NumEntries <= Current && 0 < NumEntries)
    {
        Current = NumEntries - 1;
        if (Current < Offset)
        {
            Offset = Current;
        }
    }

    // Draw directory
    for (i = 0; i < LIST_ROW && Offset + i < NumEntries; ++i)
    {
        DEMOSetFontType((Current == Offset + i) ? DM_FT_RVS : DM_FT_XLU);

        if (Offset + i == numFiles)
        {
            // The last entry for creating new file
            DEMORFPrintf(60, (s16) (75 + i * 34), 0, "%s (%d)",
                         GetString(STR_Open),
                         CardUtilBlocksNotUsed());
            continue;
        }

        ent = &Directory[Offset + i];

        // Note in GameCube file viewer, fileNo and fileName do not appear
        // on screen. This is program verification only.
        DEMORFPrintf(60, (s16) (75 + i * 34), 0, "%-.32s (#%d, %d blocks)",
                  ent->stat.fileName,
                  ent->fileNo,
                  ent->stat.length / CardUtilSectorSize());

        // Draw animated icon
        CardUtilDrawAnimatedIcon(ent, 16, 75 + i * 34 - 29, CARD_ICON_WIDTH);
    }
    DEMOSetROMFontSize(20, -1);
    DEMOSetFontType(DM_FT_XLU);
    DEMORFPrintf(180, 430, 0, GetString(STR_Stat),
                 CardUtilByteNotUsed() / CardUtilSectorSize(),
                 CardUtilFilesNotUsed());

    if (Current < numFiles)
    {
        // Show more detailed current file info
        ent = &Directory[Current];

        // Draw comment:
        // Note in GameCube file viewer, not all the characters appears on screen.
        // Effective areas are shown as green boxes.

        // 'titl', mem_lay 16h, -1, 1420h,
        DEMOSetFontType(DM_FT_OPQ);
        DrawRectangle(180, 354 - FontData->ascent, 322, FontData->leading, Green);
        DEMOSetFontType(DM_FT_XLU);
        DEMOSetROMFontSize(22, -1);
        DEMORFPrintf(180, 354, 0, "%-32.32s", ent->comment);

        // 'info', mem_lay 14h, -1, 10d0h
        DEMOSetFontType(DM_FT_OPQ);
        DrawRectangle(180, 378 - FontData->ascent, 269, FontData->leading, Green);
        DEMOSetFontType(DM_FT_XLU);
        DEMOSetROMFontSize(20, -1);
        DEMORFPrintf(180, 378, 0, "%-32.32s", &ent->comment[32]);
        DEMOSetROMFontSize((s16) FontData->cellWidth, -1);

        // Draw banner
        ent = &Directory[Current];
        if (CARDGetBannerFormat(&ent->stat) != CARD_STAT_BANNER_NONE)
        {
            DEMOSetFontType(DM_FT_OPQ);
            DrawRectangle(16, 330, DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT, Black);
            CardUtilDrawIcon(16, 330, CARD_BANNER_WIDTH,
                     (u8*) ent + ent->stat.offsetBanner     - ent->stat.iconAddr,
                     (u8*) ent + ent->stat.offsetBannerTlut - ent->stat.iconAddr,
                     CARD_BANNER_WIDTH, CARD_BANNER_HEIGHT,
                     CARDGetBannerFormat(&ent->stat));
        }

        // Draw all icons for program verifycation purpose only
        for (i = 0; i < CARD_ICON_MAX; ++i)
        {
            if (CARDGetIconFormat(&ent->stat, i) != CARD_STAT_ICON_NONE)
            {
                CardUtilDrawIcon((s16) (16 + (i % 4) * 36), (s16) (330 + 42 + (i / 4) * 36), CARD_ICON_WIDTH,
                         (u8*) ent + ent->stat.offsetIcon[i]  - ent->stat.iconAddr,
                         (u8*) ent + ent->stat.offsetIconTlut - ent->stat.iconAddr,
                         CARD_ICON_WIDTH, CARD_ICON_HEIGHT,
                         CARDGetIconFormat(&ent->stat, i));
            }
        }

        // Draw animated icon
        CardUtilDrawAnimatedIcon(ent, 16 + 3 * 36, 330, CARD_ICON_WIDTH);
    }

    // Unlocks directory so card utility thread can updates directory
    CardUtilUnlockDirectory();

    // Draw instructions and popup menu
    DrawInstructions();
}

/*---------------------------------------------------------------------------*
  Name:         NewFileName

  Description:  Generates a new file name. This function is only for
                illustration (no good). XXX
 *---------------------------------------------------------------------------*/
static void NewFileName(char* fileName)
{
    OSTime         time;
    OSCalendarTime ct;

    time = OSGetTime();
    OSTicksToCalendarTime(time, &ct);
    sprintf(fileName, "%04d/%02d/%02d %02d:%02d:%02d",
            ct.year, ct.mon + 1, ct.mday,
            ct.hour, ct.min, ct.sec);
}

/*---------------------------------------------------------------------------*
  Name:         SaveData

  Description:  Layouts banner, icon and comment data and save the file.

  Arguments:    fileName    filename whose length should be less than
                            CARD_FILENAME_MAX 'cause SaveData() creates
                            a temporary filename by adding '~' before the
                            head of the filename.
                banner      pointer to DVDBanner data
                tplBanner   banner texture image (NULL to use DVDbanner data)
                tplIcons    icon texture images (up to eight)
                iconSpeed   one of CARD_STAT_SPEED_*. Though each icon can
                            use different icon speed if desired, SaveData()
                            assigns the same speed for all icons.
                iconAnim    one of CARD_STAT_ANIM_*
                description game description (32 bytes)

  Note:         Use tplBanner to specify C8 texture image to save
                the game file size. Otherwise use DVDBanner image.
                In either case, the graphics should be same as
                the DVDBanner image.

  Returns:      Pointer to game data save area (after icon data)
 *---------------------------------------------------------------------------*/
static void* SaveData(char* fileName, DVDBanner* banner,
                      TEXPalettePtr tplBanner,
                      TEXPalettePtr tplIcons, u16 iconSpeed, u8 iconAnim,
                      char* description)
{
    TEXPalettePtr    tpl = 0;
    TEXDescriptorPtr tdp;
    int              i;
    u8*              ptr;

    ASSERT(banner);

    iconSpeed &= CARD_STAT_SPEED_MASK;
    iconAnim  &= CARD_STAT_ANIM_MASK;

    memset(&CardStatTemplate, 0, sizeof CardStatTemplate);
    CardStatTemplate.length = (u32) GetBlockSize(FILE_SIZE) * CardUtilSectorSize();

    ASSERT(CardData == 0);
    CardData = OSAlloc(CardStatTemplate.length);
    ASSERT(CardData);
    memset(CardData, 0, CardStatTemplate.length);
    strncpy(CardStatTemplate.fileName, fileName, CARD_FILENAME_MAX);

    ptr = (u8*) CardData;

    // Comment
    CardStatTemplate.commentAddr = 0;
    strncpy((char*) ptr, (char*) banner->shortTitle, CARD_COMMENT_SIZE / 2);
    ptr += CARD_COMMENT_SIZE / 2;
    strncpy((char*) ptr, description,        CARD_COMMENT_SIZE / 2);
    ptr += CARD_COMMENT_SIZE / 2;

    // Banner
    CardStatTemplate.iconAddr = (u32) (ptr - (u8*) CardData);
    if (tplBanner == NULL)
    {
        // Use DVDBanner image
        CardStatTemplate.bannerFormat = CARD_STAT_BANNER_RGB5A3;
        memcpy(ptr, banner->image,
               2 * CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT);
        ptr += 2 * CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT;
    }
    else
    {
        tdp = TEXGet(tplBanner, 0);

        // Update bannerFormat
        switch ((GXTexFmt) tdp->textureHeader->format)
        {
          case GX_TF_RGB5A3:
            CardStatTemplate.bannerFormat = CARD_STAT_BANNER_RGB5A3;
            break;
          case GX_TF_C8:
            CardStatTemplate.bannerFormat = CARD_STAT_BANNER_C8;
            ASSERT(tdp->CLUTHeader);
            break;
          default:
            OSHalt("Unsupported banner texture formant.");
            break;
        }

        // Copy texture image and tlut
        if (CARDGetBannerFormat(&CardStatTemplate) == CARD_STAT_BANNER_RGB5A3)
        {
            memcpy(ptr, tdp->textureHeader->data,
                   2 * CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT);
            ptr += 2 * CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT;
        }
        else    // CARD_STAT_ICON_C8
        {
            ASSERT(tdp->CLUTHeader);
            ASSERT((GXTlutFmt) tdp->CLUTHeader->format == GX_TL_RGB5A3);
            ASSERT(tdp->CLUTHeader->numEntries == 256);

            memcpy(ptr, tdp->textureHeader->data,
                   CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT);
            ptr += CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT;
            memcpy(ptr, tdp->CLUTHeader->data, 2 * 256);
            ptr += 2 * 256;
        }
    }

    // Icon
    if (tplIcons)
    {
        int iconTlut;

        // Update iconFormat
        for (i = 0; i < tplIcons->numDescriptors && i < CARD_ICON_MAX; i++)
        {
            tdp = TEXGet(tplIcons, (u32) i);

            switch ((GXTexFmt) tdp->textureHeader->format)
            {
              case GX_TF_RGB5A3:
                CARDSetIconFormat(&CardStatTemplate, i, CARD_STAT_ICON_RGB5A3);
                break;
              case GX_TF_C8:
                CARDSetIconFormat(&CardStatTemplate, i, CARD_STAT_ICON_C8);
                ASSERT(tdp->CLUTHeader);
                break;
              default:
                OSHalt("Unsupported icon texture formant.");
                break;
            }
            CARDSetIconSpeed(&CardStatTemplate, i, iconSpeed);
        }
        for (; i < CARD_ICON_MAX; i++)
        {
            CARDSetIconSpeed(&CardStatTemplate, i, CARD_STAT_SPEED_END);
        }

        CardStatTemplate.bannerFormat |= iconAnim;

        // Copy icon texture images and tlut
        iconTlut = -1;
        for (i = 0; i < tplIcons->numDescriptors && i < CARD_ICON_MAX; i++)
        {
            tdp = TEXGet(tplIcons, (u32) i);
            switch (CARDGetIconFormat(&CardStatTemplate, i))
            {
              case CARD_STAT_ICON_RGB5A3:
                memcpy(ptr, tdp->textureHeader->data,
                       2 * CARD_ICON_WIDTH * CARD_ICON_HEIGHT);
                ptr += 2 * CARD_ICON_WIDTH * CARD_ICON_HEIGHT;
                break;
              case CARD_STAT_ICON_C8:
                ASSERT(tdp->CLUTHeader);
                ASSERT((GXTlutFmt) tdp->CLUTHeader->format == GX_TL_RGB5A3);
                ASSERT(tdp->CLUTHeader->numEntries == 256);

                memcpy(ptr, tdp->textureHeader->data,
                       CARD_ICON_WIDTH * CARD_ICON_HEIGHT);
                ptr += CARD_ICON_WIDTH * CARD_ICON_HEIGHT;
                iconTlut = i;
                break;
            }
        }
        if (0 <= iconTlut)
        {
            tdp = TEXGet(tplIcons, (u32) i);
            memcpy(ptr, tdp->CLUTHeader->data, 2 * 256);
            ptr += 2 * 256;
        }
    }

    return ptr;
}

/*---------------------------------------------------------------------------*
  Name:         SelectAnimTick

  Description:  Selects the directory entry.
                Starts command selection if A button is pushed.
                Goes back to slot selection if B button is pushed.
 *---------------------------------------------------------------------------*/
static void SelectAnimTick(void)
{
    if (Conts[4].repeat & PAD_BUTTON_UP)
    {
        if (0 < Current)
        {
            --Current;
            if (Current < Offset)
            {
                --Offset;
            }
        }
    }
    else if (Conts[4].repeat & PAD_BUTTON_DOWN)
    {
        if (Current < NumEntries - 1)
        {
            ++Current;
            if (Offset + LIST_ROW <= Current)
            {
                ++Offset;
            }
        }
    }
    else if (Conts[4].down & PAD_BUTTON_B)
    {
        CardUtilUnmount(Slot);
        State = STATE_PROBE;
    }
    else if (Conts[4].down & PAD_BUTTON_A)
    {
        SubState = SUBSTATE_COMMAND;
        Command = CARDUTIL_CMD_SAVE;
    }

}

/*---------------------------------------------------------------------------*
  Name:         CommandAnimTick

  Description:  Switches the command.
                Selects the current command if A button is pushed.
                Goes back to file selection if B button is pushed.
 *---------------------------------------------------------------------------*/
static void CommandAnimTick(void)
{
    if (Conts[4].down & PAD_BUTTON_B)
    {
        switch (Command)
        {
          case CARDUTIL_CMD_SAVE:
          case CARDUTIL_CMD_ERASE:
            SubState = SUBSTATE_SELECT;
            Command = CARDUTIL_CMD_NONE;
            break;
          case CARDUTIL_CMD_CHECKSPACE:
          case CARDUTIL_CMD_IGNORE:
          case CARDUTIL_CMD_MAIN_MENU:
            CardUtilUnmount(Slot);
            State = STATE_PROBE;
            break;
        }
    }
    else if (Conts[4].repeat & PAD_BUTTON_UP)
    {
        switch (Command)
        {
          case CARDUTIL_CMD_SAVE:
          case CARDUTIL_CMD_ERASE:
            Command = CARDUTIL_CMD_SAVE;
            break;
          case CARDUTIL_CMD_CHECKSPACE:
          case CARDUTIL_CMD_IGNORE:
          case CARDUTIL_CMD_MAIN_MENU:
            if (CARDUTIL_CMD_CHECKSPACE < Command)
            {
                --Command;
            }
            break;
        }
    }
    else if (Conts[4].repeat & PAD_BUTTON_DOWN)
    {
        switch (Command)
        {
          case CARDUTIL_CMD_SAVE:
          case CARDUTIL_CMD_ERASE:
            Command = CARDUTIL_CMD_ERASE;
            break;
          case CARDUTIL_CMD_CHECKSPACE:
          case CARDUTIL_CMD_IGNORE:
          case CARDUTIL_CMD_MAIN_MENU:
            if (Command < CARDUTIL_CMD_MAIN_MENU)
            {
                ++Command;
            }
            break;
        }
    }
    else if (Conts[4].down & PAD_BUTTON_A)
    {
        YesNo = NO;
        switch (Command)
        {
          case CARDUTIL_CMD_SAVE:
            // Do not issue save if no space is available
            if (IsSpaceAvailable(FILE_SIZE))
            {
                SubState = SUBSTATE_VERIFY;
            }
            break;
          case CARDUTIL_CMD_ERASE:
            // Do not issue erase if no file is selected
            if (Current < CardUtilNumFiles())
            {
                SubState = SUBSTATE_VERIFY;
            }
            break;
          case CARDUTIL_CMD_CHECKSPACE:
            CardUtilUnmount(Slot);
            State = STATE_PROBE;
            break;
          case CARDUTIL_CMD_IGNORE:
            Command = CARDUTIL_CMD_NONE;
            SubState = SUBSTATE_SELECT;
            break;
          case CARDUTIL_CMD_MAIN_MENU:
            ForceMenu = TRUE;
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         VerifyAnimTick

  Description:  Starts the current command if A button is pushed.
                Cancels the current command if B button is pushed.

                If both A and B buttons are pushed, B button has a priority.
 *---------------------------------------------------------------------------*/
static void VerifyAnimTick(void)
{
    char fileName[CARD_FILENAME_MAX + 1];
    u8*  ptr;

    if (Conts[4].down & PAD_BUTTON_B)
    {
        switch (Command)
        {
          case CARDUTIL_CMD_SAVE:
          case CARDUTIL_CMD_ERASE:
            SubState = SUBSTATE_COMMAND;
            break;
          case CARDUTIL_CMD_FORMAT:
          default:
            CardUtilUnmount(Slot);
            State = STATE_PROBE;
            break;
        }
    }
    else if (Conts[4].repeat & PAD_BUTTON_UP)
    {
        YesNo = YES;
    }
    else if (Conts[4].repeat & PAD_BUTTON_DOWN)
    {
        YesNo = NO;
    }
    else if (Conts[4].down & PAD_BUTTON_A)
    {
        switch (YesNo)
        {
          case YES:
            switch (Command)
            {
              case CARDUTIL_CMD_SAVE:
                fileName[CARD_FILENAME_MAX] = '\0';
                if (CardUtilNumFiles() <= Current)
                {
                    // Create new file name
                    NewFileName(fileName);
                }
                else
                {
                    // Keep the current name
                    strncpy(fileName, Directory[Current].stat.fileName, CARD_FILENAME_MAX);
                }
                ptr = SaveData(fileName, Banner,
                               NULL,
                               TplIcons, CARD_STAT_SPEED_MIDDLE, CARD_STAT_ANIM_LOOP,
                               "File Description");

                //
                //  Implement your code here
                //
                //  Now ptr points to the game data save area after comment,
                //  banner and icon data.
                //

                CardUtilSave(Slot, &CardStatTemplate, CardData);
                SubState = SUBSTATE_BUSY;
                break;
              case CARDUTIL_CMD_ERASE:
                CardUtilErase(Slot, Directory[Current].fileNo);
                SubState = SUBSTATE_BUSY;
                break;
              case CARDUTIL_CMD_FORMAT:
                CardUtilFormat(Slot);
                SubState = SUBSTATE_BUSY;
                break;
            }
            break;
          case NO:
            switch (Command)
            {
              case CARDUTIL_CMD_FORMAT:
                CardUtilUnmount(Slot);
                State = STATE_PROBE;
                break;
              default:
                Command = CARDUTIL_CMD_NONE;
                SubState = SUBSTATE_SELECT;
                break;
            }
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         BusyAnimTick

  Description:  The function is called when card utility thread gets not-busy.
                Moves on to confirm screen.
 *---------------------------------------------------------------------------*/
static void BusyAnimTick(void)
{
    SubState = SUBSTATE_WAIT;
    BeginWaitAt = OSGetTime();  // Save current time so confirm screen can
                                // time-out if desired.
    switch (Command)
    {
      case CARDUTIL_CMD_SAVE:
        OSFree(CardData);
        CardData = 0;
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WaitAnimTick

  Description:  Goes back to default state after the player confirmed.
                Times out in 1.5 sec if the current command was successfully
                executed.
 *---------------------------------------------------------------------------*/
static void WaitAnimTick(s32 result)
{
    if (!((Conts[4].down & (PAD_BUTTON_A | PAD_BUTTON_B)) ||
          (OSMillisecondsToTicks(1500) < OSGetTime() - BeginWaitAt &&
           result == CARD_RESULT_READY)))
    {
        return;
    }

    switch (Command)
    {
      case CARDUTIL_CMD_ERROR:
        CardUtilUnmount(Slot);
        State = STATE_PROBE;
        break;
      default:
        Command = CARDUTIL_CMD_NONE;
        SubState = SUBSTATE_SELECT;
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         ListAnimTick

  Description:  Top level animation tick function for the directory screen
 *---------------------------------------------------------------------------*/
static void ListAnimTick(void)
{
    s32 result;

    result = CardUtilResultCode();

    // Listup
    if (!List && result == CARD_RESULT_READY)
    {
        List = TRUE;
        CardUtilList(Slot, Directory);
        result = CardUtilResultCode();
    }

    // The current command must not be aborted.
    if (result == CARD_RESULT_BUSY)
    {
        return;
    }

    // Check space
    if (Command == CARDUTIL_CMD_CHECKSPACE)
    {
        if (IsSpaceAvailable(FILE_SIZE))
        {
            Command = CARDUTIL_CMD_NONE;
            SubState = SUBSTATE_SELECT;
        }
        else if (result == CARD_RESULT_READY)
        {
            SubState = SUBSTATE_COMMAND;
        }
    }

    if (SubState < SUBSTATE_BUSY && !CARDProbe(Slot))
    {
        State = STATE_PROBE;
        return;
    }

    // Process error
    if (SubState < SUBSTATE_VERIFY)
    {
        switch (result)
        {
          case CARD_RESULT_READY:
          case CARD_RESULT_NOFILE:
          case CARD_RESULT_EXIST:
          case CARD_RESULT_NOPERM:
          case CARD_RESULT_LIMIT:
          case CARD_RESULT_NAMETOOLONG:
          case CARD_RESULT_CANCELED:
            break;

          case CARD_RESULT_NOENT:
          case CARD_RESULT_INSSPACE:
            break;

          case CARD_RESULT_BROKEN:
          case CARD_RESULT_ENCODING:
            Command = CARDUTIL_CMD_FORMAT;
            YesNo = NO;
            SubState = SUBSTATE_VERIFY;
            return;

          case CARD_RESULT_NOCARD:
            State = STATE_PROBE;
            return;

          default:
          case CARD_RESULT_IOERROR:
          case CARD_RESULT_FATAL_ERROR:
          case CARD_RESULT_WRONGDEVICE:
            Command = CARDUTIL_CMD_ERROR;
            SubState = SUBSTATE_WAIT;
            BeginWaitAt = OSGetTime();
            return;
        }
    }

    switch (SubState)
    {
      case SUBSTATE_SELECT:
        SelectAnimTick();
        break;
      case SUBSTATE_COMMAND:
        CommandAnimTick();
        break;
      case SUBSTATE_VERIFY:
        VerifyAnimTick();
        break;
      case SUBSTATE_BUSY:
        BusyAnimTick();
        break;
      case SUBSTATE_WAIT:
        WaitAnimTick(result);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         LoadBanner

  Description:  Load banner file "/opening.bnr"
 *---------------------------------------------------------------------------*/
static DVDBanner* LoadBanner(void)
{
    static DVDBanner banner ATTRIBUTE_ALIGN(32);
    DVDFileInfo      fileInfo;
    BOOL             result;
    s32              length;

    result = DVDOpen("/" DVD_BANNER_FILENAME, &fileInfo);
    if (!result)
        return NULL;
    length = (s32) OSRoundUp32B(DVDGetLength(&fileInfo));
    if (length < sizeof(DVDBanner))
        return NULL;
    result = DVDRead(&fileInfo, &banner, sizeof(DVDBanner), 0);
    if (!result)
        return NULL;
    if (banner.id != DVD_BANNER_ID)
        return NULL;
    return &banner;
}

/*---------------------------------------------------------------------------*
  Name:         main

  Description:  After loading ROM font and banner and icon textures,
                goes into the main loop.
 *---------------------------------------------------------------------------*/
int main(void)
{
    static BOOL reset = FALSE;  // TRUE if reset is requested

    DEMOInit(NULL);
    FontData = DEMOInitROMFont();  // Use IPL ROM font
    if (FontData == 0)
    {
        OSHalt("This program requires boot ROM ver 0.8 or later\n");
        // NOT REACHED HERE
    }

    InitCont(0, FALSE);

    CardUtilInit(CardStack + sizeof(CardStack), sizeof(CardStack), 18);

    // Clear EFB
    GXSetCopyClear(Blue, 0x00ffffff);
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);

    // Load banner file "/opening.bnr"
    Banner = LoadBanner();
    if (Banner == 0)
    {
        OSHalt("\"" DVD_BANNER_FILENAME "\" is not found in the root directory.\n");
        // NOT REACHED HERE
    }

    // Load banner and icon texture palettes
    //   banner.tpl should contain a single 96x32 C8 or RGB5A3 texture image.
    //   icon.tpl should contain from one to eight 32x32 C8 or RGB5A3 texture images.
    TEXGetPalette(&TplBanner, "/carddemo/banner.tpl");
    TEXGetPalette(&TplIcons,  "/carddemo/icon.tpl");

    for (;;)
    {
        DEMOBeforeRender();
        ReadCont();
        InterfaceTable[State].DrawTick();
        InterfaceTable[State].AnimTick();
        DEMODoneRender();

        if (ForceMenu && SubState != SUBSTATE_BUSY)     // Do not reset while saving data, etc.
        {
            // Use hot reset to start GameCube main menu
            OSResetSystem(OS_RESET_HOTRESET, 0x01, TRUE);
        }

        // Handle reset button
        if (OSGetResetButtonState())
        {
            reset = TRUE;
        }
        else if (reset && SubState != SUBSTATE_BUSY)    // Do not reset while saving data, etc.
        {
            // Restart
            OSResetSystem(OS_RESET_RESTART, 0x00, FALSE);
        }
    }

    return 0;   // lint
}
