/*---------------------------------------------------------------------------*
  Project:  Dolphin GXdemos
  File:     toy-anim.h

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /dolphin/build/demos/gxdemo/include/toy-anim.h $
    
    2     9/15/99 3:00p Ryan
    update to fix compiler warnings
    
    1     6/17/99 1:26a Alligator
    header files for toy demo.  
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef __TOY_ANIM_H__
#define __TOY_ANIM_H__

#include <dolphin.h>

#define BALL_SIZEX	5.0F
#define BALL_SIZEZ	5.0F
#define SLOPE_ZSTART	100.0F
#define SLOPE_SLOPE	(2.0F / 3.0F)



Vec GetCurPos(void);
u32 OnSlope(void);
u32 DoTeleport(void);
void AnimTick(void);

#endif  // __TOY_ANIM_H__
