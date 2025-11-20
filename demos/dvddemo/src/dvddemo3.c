/*---------------------------------------------------------------------------*
  Project:  Dolphin DVD Demo3
  File:     dvddemo3.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/dvddemo/src/dvddemo3.c $
    
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
    
    11    9/13/99 3:56p Hashida
    Removed warnings.
    
    10    8/27/99 3:35p Yasu
    Add #pragma unused() to avoid warning messages
    
    9     8/26/99 10:45p Hashida
    Removed DCInvalidate because DVDRead* functions do that now.
    
    6     6/11/99 10:50p Hashida
    Changed to use DVDSetRoot()
    
    5     6/09/99 9:22p Hashida
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
  This program shows how to use readAsync function
 *---------------------------------------------------------------------------*/


#include <dolphin.h>

void        main    ( void );
static void MyOSInit( void );
static void callback(s32 result, DVDFileInfo* fileInfo);

// These variables should be declared "volatile" because this is shared
// by two contexts.
volatile BOOL    error = FALSE;
volatile s32     readCount = 0;

// The names of the files we are going to read.
char*   fileName1 = "texts/test1.txt";
char*   fileName2 = "texts/test2.txt";


 /*---------------------------------------------------------------------------*
    Name:               callback
    
    Description:        callback function for DVDReadAsync
                                
    Arguments:          result     -1 if read fails, file size if succeeded.
                        fileInfo   fileInfo for the file transferred.
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void callback(s32 result, DVDFileInfo* fileInfo)
{
#pragma unused(fileInfo)

    if (result == -1)
    {
        error = TRUE;
    }
    else
    {
        readCount++;
    }
    return;
}

void main(void)
{
    DVDFileInfo fileInfo1;
    DVDFileInfo fileInfo2;
    u32         fileSize1;
    u32         fileSize2;
    u8*         buffer1;         // pointer to the buffer
    u8*         buffer2;         // pointer to the buffer
    u8*         ptr1;   
    u8*         ptr2;
    u32         count1;
    u32         count2;
    u32         i;

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

    // Open the first file
    if (FALSE == DVDOpen(fileName1, &fileInfo1))
    {
        OSReport("Cannot open file \"%s\"", fileName1);
        OSHalt("");
    }
    // Open the second file
    if (FALSE == DVDOpen(fileName2, &fileInfo2))
    {
        OSReport("Cannot open file \"%s\"", fileName2);
        OSHalt("");
    }

    // Get the size of the files
    fileSize1 = DVDGetLength(&fileInfo1);
    fileSize2 = DVDGetLength(&fileInfo2);

    // Allocate buffers to read the files.
    // Note that pointers returned by OSAlloc are all 32byte aligned.
    buffer1 = (u8*)OSAlloc(OSRoundUp32B(fileSize1));
    buffer2 = (u8*)OSAlloc(OSRoundUp32B(fileSize2));

    if (FALSE == DVDReadAsync(&fileInfo1, (void*)buffer1,
                              (s32)OSRoundUp32B(fileSize1), 0, callback))
    {
        OSHalt("Error occurred when issuing read for the first file");
    }

    // You can issue next asynchronous read without waiting for the 
    // previous read to finish
    if (FALSE == DVDReadAsync(&fileInfo2, (void*)buffer2,
                              (s32)OSRoundUp32B(fileSize2), 0, callback))
    {
        OSHalt("Error occurred when issuing read for the second file");
    }

    // Counter to measure the time
    count1 = 0;
    count2 = 0;

    while (readCount < 2)
    {
        if (readCount == 0)
        {
            count1++;
        }
        else if (readCount == 1)
        {
            count2++;
        }

        if (error)
        {
            OSReport("readCount is %d\n", readCount);
            if (readCount == 0)
            {
                OSHalt("Error occurred when reading the first file");
            }
            else
            {
                OSHalt("Error occurred when reading the second file");
            }
        }

    }

    OSReport("read end\n");
    OSReport("  %d loops to read the first file\n", count1);
    OSReport("  %d loops to read the second file\n", count2);
    OSReport("\n");

    // Close the files
    DVDClose(&fileInfo1);
    DVDClose(&fileInfo2);

    // Show the files
    OSReport("First file:\n");
    ptr1 = buffer1;
    for (i = 0; i < fileSize1; i++)
    {
        OSReport("%c", *ptr1++);
    }
    OSReport("\n\n");

    OSReport("Second file:\n");
    ptr2 = buffer2;
    for (i = 0; i < fileSize2; i++)
    {
        OSReport("%c", *ptr2++);
    }
    OSReport("\n\n");

    OSFree(buffer1);
    OSFree(buffer2);

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
static void MyOSInit ( void )
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
