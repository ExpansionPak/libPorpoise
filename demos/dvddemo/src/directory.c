/*---------------------------------------------------------------------------*
  Project: test for open and read
  File:    open-read.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/dvddemo/src/directory.c $
    
    3     3/28/00 3:40p Hashida
    Added DVDSetRoot for MAC build
    
    2     3/01/00 1:14p Hashida
    Renamed dirent->filename to name.
    
    1     1/13/00 12:17p Hashida
    Initial revision
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*

 */
#include <dolphin/os.h>
#include <dolphin/dvd.h>

#include <string.h>

static void MyOSInit( void );
static void printOneLevel(char* pathName);

struct dirs_t
{
    struct dirs_t*  next;       // must be first
    DVDDirEntry     dirEntry;
};

typedef struct dirs_t       dirs;


static void printOneLevel(char* pathName)
{
    DVDFileInfo     finfo;
    DVDDir          dir;
    DVDDirEntry     dirent;
    dirs*           start = (dirs*)NULL;
    dirs*           curr;
    dirs            *p, *q;
    char            path[256];

    curr = (dirs*)&start;
    
    if (FALSE == DVDOpenDir(".", &dir))
    {
        OSReport("Can't open dir %s\n", path);
        return;
    }
    
    while(1)
    {
        if (FALSE == DVDReadDir(&dir, &dirent))
        {
            for(p = start; p != (dirs*)NULL; )
            {
                strcpy(path, pathName);
                strcat(path, "/");
                strcat(path, p->dirEntry.name);
                OSReport("\n%s:\n", path);
                
                if (FALSE == DVDChangeDir(p->dirEntry.name))
                {
                    OSReport("Can't change dir to %s\n", path);
                    return;
                }
                printOneLevel(path);
                if (FALSE == DVDChangeDir(".."))
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
                   sizeof(DVDDirEntry));
            // we open directories later
        }
        else
        {
            if (FALSE == DVDOpen(dirent.name, &finfo))
            {
                OSReport("Can't open file %s/%s\n", path, dirent.name);
                return;
            }
            
            OSReport("F %9d %s\n", DVDGetLength(&finfo), dirent.name);

            DVDClose(&finfo);
        }
        
    } // while(1)
    
} // void printOneLevel(char*)


void main(void)
{
    MyOSInit();
    
#ifdef MAC
    OSReport("\n-----------------------------------");
    OSReport("\n  Hit Command+Q to quit this demo");
    OSReport("\n-----------------------------------\n\n");

    DVDSetRoot("DOLPHIN/dvddata");
#endif

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
