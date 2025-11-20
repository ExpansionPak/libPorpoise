/*---------------------------------------------------------------------------*
  Project: test for open and read (archiver version)
  File:    arctest2.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/tests/arc/src/arctest2.c $
    
    1     7/03/01 3:16a Hashida
    Initial revision.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*

 */
#include <dolphin/os.h>
#include <dolphin/dvd.h>
#include <private/arc.h>

// XXX this definition is not needed any more. However, we use this because
// constitution files under dvddata is still 8.3 file names. (Archiver can
// handle long file names!)
#define SHORTFILENAME

static void MyOSInit( void );
void callback(s32 result, DVDFileInfo* finfo);
void chdirOpenAllocRead(char* dirName, char* pathName);

BOOL UseArc = FALSE;

ARCHeader       header          ATTRIBUTE_ALIGN(32);
DVDFileInfo     finfo;
ARCHandle       Arc;


/*---------------------------------------------------------------------------*
  Name:         chdirOpenAllocRead

  Description:  change dir to the specified dir, open the specified file,
                allocate memory enough to read it, and read

  Arguments:    dirName      directory to change. If NULL, don't change dir.
                pathName     file to read

  Returns:      None.
 *---------------------------------------------------------------------------*/
void chdirOpenAllocRead(char* dirName, char* pathName)
{
    BOOL               result;
    ARCFileInfo        afinfo;
    u32                length;
    char*              buf;
    char*              ptr;
    u32                i;

    if (dirName != (char*)NULL)
    {
        if (UseArc)
        {
            result = ARCChangeDir(&Arc, dirName);
        }
        else
        {
            result = DVDChangeDir(dirName);
        }
        
        OSReport("Changing dir to %s \n", dirName);

        if (result)
        {
            OSReport("-- succeeded\n");
        }
        else
        {
            OSHalt("-- failed\n");
        }
    }
    
    if (UseArc)
    {
        result = ARCOpen(&Arc, pathName, &afinfo);
    }
    else
    {
        result = DVDOpen(pathName, &finfo);
    }

    OSReport("Opening %s took \n", pathName);

    if (result)
    {
        OSReport("-- succeeded\n");
    }
    else
    {
        OSHalt("-- failed\n");
    }

    if (UseArc)
    {
        length = ARCGetLength(&afinfo);
    }
    else
    {
        length = DVDGetLength(&finfo);
    }
    
    buf = (char*)OSAlloc(OSRoundUp32B(length));

    if (UseArc)
    {
        if (OSRoundUp32B(length) !=
            DVDRead(&finfo, (void*)buf, (s32)OSRoundUp32B(length),
                    (s32)ARCGetStartOffset(&afinfo)))
        {
            OSReport("Error occurred when reading %s\n", pathName);
            OSHalt("");
        }
    }
    else
    {
        if (OSRoundUp32B(length) !=
            DVDRead(&finfo, (void*)buf, (s32)OSRoundUp32B(length), 0))
        {
            OSReport("Error occurred when reading %s\n", pathName);
            OSHalt("");
        }
    }
        
    OSReport("\n-- Contents of the file starts from the following line.\n");
    ptr = buf;
    for (i = 0; i < length; i++)
    {
        OSReport("%c", *ptr++);
    }
    OSReport("<eof>\n\n");

    if (UseArc)
    {
        OSFree(buf);

        ARCClose(&afinfo);
    }
    else
    {
        OSFree(buf);
    
        DVDClose(&finfo);
    }
}


static void* Read(s32 entrynum)
{
    BOOL                result;
    u32                 length;
    void*               buf;

    result = DVDFastOpen(entrynum, &finfo);

    if (!result)
    {
        OSHalt("Read -- failed\n");
    }

    length = OSRoundUp32B(DVDGetLength(&finfo));
    
    if ( sizeof(ARCHeader) !=
         DVDRead(&finfo, &header, (s32)sizeof(ARCHeader), 0) )
    {
        OSReport("Error occurred when reading archiver header\n");
        OSHalt("");
    }

    length = OSRoundUp32B(header.fileStart);

    buf = OSAlloc(length);

    if ( length != DVDRead(&finfo, buf, (s32)length, 0) )
    {
        OSReport("Error occurred when reading\n");
        OSHalt("");
    }

    return buf;
}

void main(void)
{
    s32             entrynum;
    void*           arcStart;


    MyOSInit();

    // Check the existence of the archive file.
    // If not, we use the normal way.
    entrynum = DVDConvertPathToEntrynum("constitu.arc");
    if (entrynum >= 0)
    {
        OSReport("Found an archive. Use archive APIs instead.\n");

        UseArc = TRUE;
        arcStart = Read(entrynum);

        if (ARCInitHandle(arcStart, &Arc) == FALSE)
        {
            OSHalt("Bad archive format");
        }
    }
    
    // absolute path
#ifndef SHORTFILENAME
    chdirOpenAllocRead((char*)NULL, "/constitution/article II/section 2");
#else
    chdirOpenAllocRead((char*)NULL, "/constitu/article2/section2");
#endif

    // relative path from root
#ifndef SHORTFILENAME
    chdirOpenAllocRead((char*)NULL, "constitution/article II/section 2");
#else
    chdirOpenAllocRead((char*)NULL, "constitu/article2/section2");
#endif

    // change directory
#ifndef SHORTFILENAME
    chdirOpenAllocRead("constitution/article I/", "section 5");
#else
    chdirOpenAllocRead("constitu/article1/", "section5");
#endif

    // goto parent
#ifndef SHORTFILENAME
    chdirOpenAllocRead((char*)NULL, "../article III/../article II/section 1");
#else
    chdirOpenAllocRead((char*)NULL, "../article3/../article2/section1");
#endif
    
    // from root
#ifndef SHORTFILENAME
    chdirOpenAllocRead((char*)NULL, "/constitution/article III/section 2");
#else
    chdirOpenAllocRead((char*)NULL, "/constitu/article3/section2");
#endif

    // parent of root
#ifndef SHORTFILENAME
    chdirOpenAllocRead("/", "../constitution/article I/section 8");
#else
    chdirOpenAllocRead("/", "../constitu/article1/section8");
#endif

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
