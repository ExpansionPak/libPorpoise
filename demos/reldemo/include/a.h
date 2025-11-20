/*---------------------------------------------------------------------------*
  Project:  Dolphin OS private routines
  File:     a.h

  Copyright 1998-2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/tests/rel/include/a.h $
    
    2     01/04/02 13:41 Shiki
    Added IsAThere().

    1     10/31/00 3:51p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef __A_H__
#define __A_H__

#define REL

class A
{
    static int count;
public:
    A() { OSReport("Hi! I'm class A! (%d)\n", ++count); }
    ~A() { OSReport("Bye! I'm class A! (%d)\n", count--); }
};

void MainA(void);
void IsAThere(void);

#endif  // __A_H__
