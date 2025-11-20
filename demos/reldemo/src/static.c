/*---------------------------------------------------------------------------*
  Project:  Relocatable module test
  File:     static.c

  Copyright 2000-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/reldemo/src/static.c $
    
    8     11/27/01 21:49 Shiki
    Modified to include <dolphin.h> only.

    7     6/09/01 5:25p Shiki
    Fixed to load .rel files from lower arena to support 48MB ORCA.

    6     01/04/02 13:44 Shiki
    Separated OSModuleInfo into OSModuleInfo and OSModuleHeader.

    5     01/03/01 12:55 Shiki
    Clean up.

    4     01/02/27 13:11 Shiki
    Modified to load string table with non-debug build as well.

    3     10/31/00 3:51p Shiki
    Modified to call OSUnlink().

    2     4/19/00 12:47a Shiki
    Added a call to unlinked function to test.

    1     4/14/00 11:37p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

#ifdef _DEBUG
#define MODULE_A     "/aD.rel"
#define MODULE_B     "/bD.rel"
#define STRING_TABLE "/staticD.str"
#else
#define MODULE_A     "/a.rel"
#define MODULE_B     "/b.rel"
#define STRING_TABLE "/static.str"
#endif

static void DumpModuleInfo(OSModuleInfo* moduleInfo)
{
    OSReport("id                %d\n",         moduleInfo->id);
    OSReport("numSections       %d\n",         moduleInfo->numSections);
    OSReport("sectionInfoOffset %08xh\n",      moduleInfo->sectionInfoOffset);
    OSReport("nameOffset        %08xh [%s]\n", moduleInfo->nameOffset, moduleInfo->nameOffset);
    OSReport("nameSize          %d\n",         moduleInfo->nameSize);
    OSReport("\n");
}

static void DumpModuleHeader(OSModuleHeader* module)
{
    DumpModuleInfo(&module->info);
    OSReport("bssSize           %d\n",         module->bssSize);
    OSReport("relOffset         %08xh\n",      module->relOffset);
    OSReport("impOffset         %08xh\n",      module->impOffset);
    OSReport("impSize           %08xh\n",      module->impSize);
    OSReport("prolog            %08xh\n",      module->prolog);
    OSReport("epilog            %08xh\n",      module->epilog);
    OSReport("unresolved        %08xh\n",      module->unresolved);
    OSReport("\n");
}

int main(void)
{
    BOOL            result;
    DVDFileInfo     fileInfo;
    u32             length;
    OSModuleHeader* moduleA;
    OSModuleHeader* moduleB;
    void*           ptr;

    OSInit();
    DVDInit();

    // Load string table to use debugger
    result = DVDOpen(STRING_TABLE, &fileInfo);
    if (!result)
        return 1;
    length = OSRoundUp32B(DVDGetLength(&fileInfo));
    ptr = OSAllocFromArenaLo(length, 32);
    result = DVDRead(&fileInfo, ptr, (s32) length, 0);
    if (!result)
        return 1;
    OSSetStringTable(ptr);

    // Load and link module A
    result = DVDOpen(MODULE_A, &fileInfo);
    if (!result)
        return 1;
    length = OSRoundUp32B(DVDGetLength(&fileInfo));
    moduleA = OSAllocFromArenaLo(length, 32);
    result = DVDRead(&fileInfo, moduleA, (s32) length, 0);
    if (!result)
        return 1;
    ptr = OSAllocFromArenaLo(moduleA->bssSize, 32); // alloc bss area
    OSLink(&moduleA->info, ptr);
    DumpModuleHeader(moduleA);
    ((u32 (*)(void)) moduleA->prolog)();

    // Load and link module B
    result = DVDOpen(MODULE_B, &fileInfo);
    if (!result)
        return 1;
    length = OSRoundUp32B(DVDGetLength(&fileInfo));
    moduleB = OSAllocFromArenaLo(length, 32);
    result = DVDRead(&fileInfo, moduleB, (s32) length, 0);
    if (!result)
        return 1;
    ptr = OSAllocFromArenaLo(moduleB->bssSize, 32); // alloc bss area
    OSLink(&moduleB->info, ptr);
    DumpModuleHeader(moduleB);
    ((u32 (*)(void)) moduleB->prolog)();

    // Unlink module B
    ((u32 (*)(void)) moduleB->epilog)();
    OSUnlink(&moduleB->info);

    // Unlink module A
    ((u32 (*)(void)) moduleA->epilog)();
    OSUnlink(&moduleA->info);

    return 0;
}
