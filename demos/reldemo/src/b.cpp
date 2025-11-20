/*---------------------------------------------------------------------------*
  Project:  Relocatable module test
  File:     b.cpp

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/tests/rel/src/b.cpp $
    
    2     01/04/02 13:43 Shiki
    Added _unresolved() and IsBThere().

    1     10/31/00 3:50p Shiki
    Modified from .c version.

    1     4/14/00 11:37p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin/os.h>
#include "b.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*voidfunctionptr) (void); /* ptr to function returning void */
__declspec(section ".init") extern voidfunctionptr _ctors[];
__declspec(section ".init") extern voidfunctionptr _dtors[];

void _prolog(void);
void _epilog(void);
void _unresolved(void);

#ifdef __cplusplus
}
#endif

void _prolog(void)
{
    voidfunctionptr *constructor;

    /*
     *  call static initializers
     */
    for (constructor = _ctors; *constructor; constructor++) {
        (*constructor)();
    }
    MainB();
}

void _epilog(void)
{
    voidfunctionptr *destructor;

    /*
     *  call destructors
     */
    for (destructor = _dtors; *destructor; destructor++) {
        (*destructor)();
    }
}

void _unresolved(void)
{
    u32     i;
    u32*    p;

    OSReport("\nError: B called an unlinked function.\n");
    OSReport("Address:      Back Chain    LR Save\n");
    for (i = 0, p = (u32*) OSGetStackPointer(); // get current sp
         p && (u32) p != 0xffffffff && i++ < 16;
         p = (u32*) *p)                         // get caller sp
    {
        OSReport("0x%08x:   0x%08x    0x%08x\n", p, p[0], p[1]);
    }
    OSReport("\n");
}

int B::count;

B StaticB01;
B StaticB02;

void MainB(void)
{
    OSReport("Hello, I'm MainB()!\n");
    B b;
    IsAThere();
}

void IsBThere(void)
{
    OSReport("Yes, I'm B.\n");
}
