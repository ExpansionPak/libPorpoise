/*---------------------------------------------------------------------------*
  Project:  Dolphin OS private routines
  File:     b.h

  Copyright 1998-2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/tests/rel/include/b.h $
    
    2     01/04/02 13:41 Shiki
    Added IsBThere().

    1     10/31/00 3:51p Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef __B_H__
#define __B_H__

#include "a.h"

class B : public A
{
    static int count;
public:
    B() { OSReport("Hi! I'm class B! (%d)\n", ++count); }
    ~B() { OSReport("Bye! I'm class B! (%d)\n", count--); }
};

void MainB(void);
void IsBThere(void);

#endif  // __B_H__
