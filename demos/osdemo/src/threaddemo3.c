/*---------------------------------------------------------------------------*
  Project:  Thread Demo -- No. 3
  File:     threaddemo3.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/osdemo/src/threaddemo3.c $
    
    2     6/07/00 6:12p Tian
    Updated OSCreateThread and idle function APIs
    
    1     2/16/00 2:23p Tian
    Cleaned up and moved to demos
    
    1     2/08/00 5:11p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

OSMessageQueue MessageQueue;
OSMessage      MessageArray[16];

OSThread       Thread;
u8             ThreadStack[4096];

//
// Print server thread function
//
static void* Printer(void* param)
{
    #pragma unused (param)
    OSMessage msg;

    for (;;)
    {
        OSReceiveMessage(&MessageQueue, &msg, OS_MESSAGE_BLOCK);
        OSReport("%s\n", msg);
    }

    return 0;   
}

void main(void)
{
    int i;

    OSInit();
    VIInit();

    OSReport("Thread Demo 2: Synchronizing with messages\n");

    //
    // Initializes the message queue
    //
    OSInitMessageQueue(
        &MessageQueue,                      // pointer to message queue
        MessageArray,                       // pointer to message boxes
        sizeof MessageArray / sizeof MessageArray[0]);  // # of message boxes

    //
    // Creates a new thread. The thread is suspended by default.
    //
    OSCreateThread(
        &Thread,                            // ptr to the thread to init
        Printer,                            // ptr to the start routine
        0,                                  // param passed to start routine
        ThreadStack + sizeof ThreadStack,   // initial stack address
        sizeof ThreadStack,                 // stack size
        8,                                  // scheduling priority
        OS_THREAD_ATTR_DETACH);             // detached since it will 
                                            // never return


    //
    // Starts the thread
    //
    OSResumeThread(&Thread);

    //
    // Main loop
    //
    for (i = 0; i < 16; i++)
    {
        OSSendMessage(&MessageQueue, "Hello!", OS_MESSAGE_NOBLOCK);
        VIWaitForRetrace();                 // Sleep till next V-sync
    }

    OSHalt("Demo complete");
}
