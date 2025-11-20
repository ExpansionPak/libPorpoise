/*---------------------------------------------------------------------------*
  Project:  Dolphin GXdemos
  File:     toy-stick.h

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/include/toy-stick.h $
    
    2     3/27/00 3:24p Hirose
    changed to use DEMOPad library
    
    1     6/17/99 1:26a Alligator
    header files for toy demo.  
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef __TOY_STICK_H__
#define __TOY_STICK_H__

f32  GetAnalogX(void);
f32  GetAnalogY(void);
void StickTick(void);
u16  StickDone(void);

#endif  // __TOY_STICK_H__
