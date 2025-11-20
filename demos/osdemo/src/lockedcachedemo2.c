/*---------------------------------------------------------------------------*
  Project:  Low level locked cache API demo
  File:     lockedcachedemo2.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/osdemo/src/lockedcachedemo2.c $
    
    3     6/11/01 7:54p Tian
    bzero definition not required for SN
    
    2     6/08/00 12:45p Tian
    Corrected PMC usage to enable cycle count last.
    
    1     2/15/00 7:03p Tian
    Initial checkin.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <dolphin.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Low level locked cache api demo

/*---------------------------------------------------------------------------*
  Performance monitor macros
 *---------------------------------------------------------------------------*/
// STARTPMC sets both MMCRs (monitor control registers) going.
// PMC1 measures instruction count
// PMC2 measures # of loads and stores
// PMC3 measures # of cycles lost to L1 misses
// PMC4 measures cycle count
// Note : cycle counter is turned on last
#define STARTPMC            PPCMtmmcr0(MMCR0_PMC1_INSTRUCTION |   \
                                       MMCR0_PMC2_LOAD_STORE);    \
                            PPCMtmmcr1(MMCR1_PMC3_L1_MISS_CYCLE | \
                                       MMCR1_PMC4_CYCLE);
                            
// STOPPMC pauses all performance counters by writing 0 to the MMCRs.  
// Note that cycle counter is turned off first.
#define STOPPMC             PPCMtmmcr1(0); \
                            PPCMtmmcr0(0);

#define PRINTPMC            OSReport("<%d loadstores / %d miss cycles / %d cycles / %d Instructions>\n", \
                                     PPCMfpmc2(), PPCMfpmc3(), PPCMfpmc4(), PPCMfpmc1()); 

#define RESETPMC            PPCMtpmc1(0); \
                            PPCMtpmc2(0); \
                            PPCMtpmc3(0); \
                            PPCMtpmc4(0);



/*---------------------------------------------------------------------------*
  Buffer management
 *---------------------------------------------------------------------------*/
// use 4 4k buffers
// note that NUMBUFFERS * BUFFER_SIZE <= 16k
#define BUFFER_SIZE         (4*1024)
#define NUM_BUFFERS         (4)
#define DATA_ELEMENTS       (2*1024*1024)

// value to write to each buffer
#define TESTVALUE           0xA

void VerifyData(u8* buffer, u8 value);
void ProcessBuf(u8* buffer);
#ifndef __SN__
void bzero(void*, u32);
#endif
// real memory location of Buffers[i] is at BufAddr[i]
u8*                         Buffers[NUM_BUFFERS];
u8*                         BufAddr[NUM_BUFFERS];   
#ifndef __SN__
void bzero(void* ptr, u32 bytes)
{
    u32     i;
    u8*     p = ptr;
    
    for (i = 0; i < bytes; i++)
    {
        p[i] = 0;
    }
}
#endif

// verify that the buffer pointed to by buffer is equal to value
void VerifyData(u8* buffer, u8 value)
{
    u32 i;
    
    for (i = 0; i < DATA_ELEMENTS; i++)
    {
        if (buffer[i] != value)
        {
            OSReport("ERROR : Buffer[%d]@0x%x (%d) != %d\n",
                     i,
                     buffer + i,
                     buffer[i],
                     value);
            OSHalt("Test failed");
        }
    }
}


// trivial buffer processing - increment each buffer element with TESTVALUE
// this ensures we perform a READ on the data.
void ProcessBuf(u8* buffer)
{
    u32 i;
    u8  val;
    
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        // Because this loop might overrun the locked cache 
        // during a mispredicted branch, pad the top of the loop 
        // with a bunch of non-loads.
        val = TESTVALUE;
        val *= 2;
        val /= 2;
        buffer[i] = (u8)(buffer[i] + val);
    }
}


void main ()
{
    u8*         data;
    u8*         currDataPtr;        // offset into data
    u32         i;
    void*       arenaLo;
    void*       arenaHi;
    
#ifndef GEKKO
    OSHalt("This test is GEKKO specific");
#endif
    OSInit();

    OSReport(" Locked Cache Demo 1 : ");
    OSReport("using high level interface for DMA load/store \n");

    LCEnable();

    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();

    // OSInitAlloc should only ever be invoked once.
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 1); // 1 heap
    OSSetArenaLo(arenaLo);

    // Ensure boundaries are 32B aligned
    arenaLo = (void*)OSRoundUp32B(arenaLo);
    arenaHi = (void*)OSRoundDown32B(arenaHi);

    // The boundaries given to OSCreateHeap should be 32B aligned
    OSSetCurrentHeap(OSCreateHeap(arenaLo, arenaHi));
    // From here on out, OSAlloc and OSFree behave like malloc and free
    // respectively
    OSSetArenaLo(arenaLo=arenaHi);

    OSReport("Splitting locked cache into %d buffers\n", NUM_BUFFERS);

    for (i = 0; i < NUM_BUFFERS; i++)
    {
        Buffers[i] = (u8*) ((u32)LCGetBase() + BUFFER_SIZE*i);
        OSReport("  Locked Cache : Allocated %d bytes at 0x%x\n",
                 BUFFER_SIZE,
                 Buffers[i]);
    }

    // Initialize source data
    data = (u8*)OSAlloc(DATA_ELEMENTS * sizeof(u8));
    OSReport("Initializing source data <0x%x - 0x%x> to all 0's\n",
             data,
             data + DATA_ELEMENTS);
    bzero(data, DATA_ELEMENTS);
    DCFlushRange(data, DATA_ELEMENTS);
    
    // Initialize the first buffers
    for (i = 0; i < NUM_BUFFERS; i++)
    {
        BufAddr[i]      = data + BUFFER_SIZE*i;
        LCLoadBlocks(Buffers[i], BufAddr[i], 0); // 0 is max number of lines
    }

    currDataPtr = data + (BUFFER_SIZE * NUM_BUFFERS);

    RESETPMC
    STARTPMC
    LCQueueWait(NUM_BUFFERS-1);

    while (currDataPtr < data+DATA_ELEMENTS)
    {   
        for (i = 0; i < NUM_BUFFERS; i++)
        {
            // prevstore + prevload, each takes 2
            LCQueueWait((NUM_BUFFERS-1)*2); 
            ProcessBuf(Buffers[i]);
            LCStoreBlocks(BufAddr[i], Buffers[i], 0);
            BufAddr[i] = currDataPtr;   // move to next unprocessed buffer
            LCLoadBlocks(Buffers[i], BufAddr[i], 0);
            // advance the next block to be read
            currDataPtr += BUFFER_SIZE;
        }
    }

    // process last buffers
    for (i = 0; i < NUM_BUFFERS; i++)
    {
        ProcessBuf(Buffers[i]);
        LCStoreBlocks(BufAddr[i], Buffers[i],  0);
    }

    LCQueueWait(NUM_BUFFERS); 
    STOPPMC
    OSReport("Dumping performance monitors- L1 miss cycles should be zero:\n");
    PRINTPMC

    OSReport("Verifying data...\n");
    VerifyData(data,TESTVALUE);

    OSHalt("Demo complete");
}
