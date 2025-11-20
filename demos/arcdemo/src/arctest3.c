/*---------------------------------------------------------------------------*
  Project: test for archiver
  File:    arctest3.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/tests/arc/src/arctest3.c $
    
    1     7/03/01 3:16a Hashida
    Initial revision.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*

 */
#include <dolphin/os.h>
#include <dolphin/dvd.h>
#include <dolphin/db/dbinterface.h>
#include <string.h>
#include <private/arc.h>

static void MyOSInit( void );
static void printOneLevel(char* pathName);

ARCHandle       Arc;

struct dirs_t
{
    struct dirs_t*  next;       // must be first
    ARCDirEntry     dirEntry;
};

typedef struct dirs_t       dirs;


static void printOneLevel(char* pathName)
{
    ARCFileInfo     finfo;
    ARCDir          dir;
    ARCDirEntry     dirent;
    dirs*           start = (dirs*)NULL;
    dirs*           curr;
    dirs            *p, *q;
    char            path[256];

    curr = (dirs*)&start;
    
    if (FALSE == ARCOpenDir(&Arc, ".", &dir))
    {
        OSReport("Can't open dir %s\n", path);
        return;
    }
    
    while(1)
    {
        if (FALSE == ARCReadDir(&dir, &dirent))
        {
            for(p = start; p != (dirs*)NULL; )
            {
                strcpy(path, pathName);
                strcat(path, "/");
                strcat(path, p->dirEntry.name);
                OSReport("\n%s:\n", path);
                
                if (FALSE == ARCChangeDir(&Arc, p->dirEntry.name))
                {
                    OSReport("Can't change dir to %s\n", path);
                    return;
                }
                printOneLevel(path);
                if (FALSE == ARCChangeDir(&Arc, ".."))
                {
                    OSReport("Can't change dir to %s/..\n", path);
                    return;
                }

                q = p;
                p = p->next;
                OSFree((void*)q);
            }
            return;
        }
        
        if (dirent.isDir)
        {
            OSReport("D %9d %s\n", 0, dirent.name);

            if (NULL == ( curr->next = (dirs*)OSAlloc(sizeof(dirs)) ) )
            {
                OSReport("Can't allocate memory\n");
                return;
            }
            curr = curr->next;
            curr->next = (dirs*)NULL;
            memcpy((void*)&(curr->dirEntry), (void*)&dirent,
                   sizeof(ARCDirEntry));
            // we open directories later
        }
        else
        {
            if (FALSE == ARCOpen(&Arc, dirent.name, &finfo))
            {
                OSReport("Can't open file %s/%s\n", path, dirent.name);
                return;
            }
            
            OSReport("F %9d %s\n", ARCGetLength(&finfo), dirent.name);

            ARCClose(&finfo);
        }
        
    } // while(1)
    
} // void printOneLevel(char*)


static void* Read(s32 entrynum)
{
    BOOL                result;
    DVDFileInfo         finfo;
    u32                 length;
    void*               buf;


    result = DVDFastOpen(entrynum, &finfo);

    if (!result)
    {
        OSHalt("Read -- failed\n");
    }

    length = OSRoundUp32B(DVDGetLength(&finfo));
    
    buf = OSAlloc(OSRoundUp32B(length));

    if ( length != DVDRead(&finfo, buf, (s32)length, 0) )
    {
        OSReport("Error occurred when reading\n");
        OSHalt("");
    }
    DVDClose(&finfo);

    return buf;
}

void main(void)
{
    s32             entrynum;
    void*           arcStart;

    MyOSInit();
    
    entrynum = DVDConvertPathToEntrynum("constitu.arc");
    if (entrynum >= 0)
    {
        arcStart = Read(entrynum);

        if (ARCInitHandle(arcStart, &Arc) == FALSE)
        {
            OSHalt("Bad archive format");
        }
    }
    else
    {
        OSHalt("Can't find archive \"constitu.arc\". Abort.");
    }

    printOneLevel(".");
    
    OSHalt("End of program");

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

    DVDInit();
    
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
