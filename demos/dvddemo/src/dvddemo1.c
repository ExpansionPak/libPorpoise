/*---------------------------------------------------------------------------*
  Project:  Dolphin DVD Demo1
  File:     dvddemo1.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/dvddemo/src/dvddemo1.c $
    
    6     5/26/00 5:45p Hashida
    #ifdef'ed out the comment "Hit Command+Q to quit this demo, which is
    only needed for Mac.
    
    5     3/01/00 3:27p Hashida
    Changed #ifndef EPPC to #ifdef MAC to ifdef out DVDSetRoot.
    
    4     2/29/00 5:55p Hashida
    Changed /dolphin/data to /dolphin/dvddata
    
    3     2/25/00 5:34p Hashida
    Changed so that it works on MINNOW_MARLIN
    
    2     2/16/00 2:39p Tian
    Removed DVDSetRoot for EPPC builds
    
    9     9/13/99 3:56p Hashida
    Removed warnings.
    
    8     8/26/99 10:44p Hashida
    Removed DCInvalidate because DVDRead* functions do that now.
    
    5     6/11/99 10:50p Hashida
    Changed to use DVDSetRoot()
    
    4     6/09/99 9:22p Hashida
    Changed to use OSAlloc to allocate the buffer for files.
    Added "volatile" to the variables that are accessed from two contexts.
    Used DVDGetLength for demo1 and demo2.
    
    2     6/07/99 8:26p Hashida
    changed the files' path name from "dvdoverview" to "dvddemo"
    
    1     6/07/99 2:47p Hashida
    initial revision

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This program shows how to use synchronous read function
 *---------------------------------------------------------------------------*/


#include <dolphin.h>

void        main            ( void );
static void MyOSInit        ( void );

// The name of the file we are going to read.
char*   fileName = "texts/test1.txt";


void main(void)
{
    DVDFileInfo fileInfo;
    u32         fileSize;
    u8*         buffer;         // pointer to the buffer
    u8*         ptr;
    u32         i;

    // Init the OS and heap
    MyOSInit();

#ifdef MAC
    OSReport("\n-----------------------------------");
    OSReport("\n  Hit Command+Q to quit this demo");
    OSReport("\n-----------------------------------\n\n");
#endif

    DVDInit();

#ifdef MAC
    DVDSetRoot("DOLPHIN/dvddata");
#endif

    // We MUST open the file before accessing the file
    if (FALSE == DVDOpen(fileName, &fileInfo))
    {
        OSHalt("Cannot open file");
    }

    // Get the size of the file
    fileSize = DVDGetLength(&fileInfo);

    // Allocate a buffer to read the file.
    // Note that pointers returned by OSAlloc are always 32byte aligned.
    buffer = (u8*)OSAlloc(OSRoundUp32B(fileSize));

    // read the entire file here
    if (0 > DVDRead(&fileInfo, (void*)buffer, (s32)OSRoundUp32B(fileSize), 0))
    {
        OSHalt("Error occurred when reading file");
    }

    // From here, we can use the file
    OSReport("read end\n");

    // Close the file
    DVDClose(&fileInfo);

    // Show the files
    OSReport("\n");
    ptr = buffer;
    for (i = 0; i < fileSize; i++)
    {
        OSReport("%c", *ptr++);
    }
    OSReport("\n\n");

    OSFree(buffer);

    OSHalt("End of demo");

    // NOT REACHED HERE
}

 /*---------------------------------------------------------------------------*
    Name:               MyOSInit
    
    Description:        Initialize the operating system.
                        Create a heap so we can use OSAlloc().
                                
    Arguments:          none
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MyOSInit(void)
{
    void*               arenaLo;
    void*               arenaHi;

    OSInit();

    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();

    // OSInitAlloc should only ever be invoked once.
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 1); // 1 heap
    OSSetArenaLo(arenaLo);

    // The boundaries given to OSCreateHeap should be 32B aligned
    OSSetCurrentHeap(OSCreateHeap((void*)OSRoundUp32B(arenaLo),
                                  (void*)OSRoundDown32B(arenaHi)));
    // From here on out, OSAlloc and OSFree behave like malloc and free
    // respectively

    OSSetArenaLo(arenaLo = arenaHi);

    return;
}
