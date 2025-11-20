/*---------------------------------------------------------------------------*
  Project:  Relocatable module test
  File:     a.cpp

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/tests/rel/src/a.cpp $
    
    4     01/05/09 15:12 Shiki
    Added const variables to generate .rodata section.

    3     01/04/02 13:42 Shiki
    Added _unresolved() and IsAThere().

    2     10/31/00 4:54p Shiki
    Fixed OSReport msg.

    1     10/31/00 3:50p Shiki
    Modified from .c version.

    2     4/19/00 12:47a Shiki
    Added _unresolved().

    1     4/14/00 11:37p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>
#include "a.h"
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
    MainA();
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

    OSReport("\nError: A called an unlinked function.\n");
    OSReport("Address:      Back Chain    LR Save\n");
    for (i = 0, p = (u32*) OSGetStackPointer(); // get current sp
         p && (u32) p != 0xffffffff && i++ < 16;
         p = (u32*) *p)                         // get caller sp
    {
        OSReport("0x%08x:   0x%08x    0x%08x\n", p, p[0], p[1]);
    }
    OSReport("\n");
}

int A::count;

A StaticA01;
A StaticA02;

static u8 Zero;
static u8 One = 1;

void MainA(void)
{
    OSReport("Hello, I'm MainA()! %d %d\n", Zero, One);
    A a;

    const Vec up = {0.0f, 1.0f, 0.0f};
    const Vec trg = {0.0f, 0.0f, 0.0f};
    const Vec pos = {0.0f, 0.0f, -10.0f};

    IsBThere();
}

void IsAThere(void)
{
    OSReport("Yes, I'm A.\n");
}
