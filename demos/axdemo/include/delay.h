/*---------------------------------------------------------------------------*
  Project:  Simple single instance delay for AX
  File:     delay.h

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/axdemo/include/delay.h $
    
    1     5/09/01 6:09p Billyjack
    created
   
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <dolphin/axfx.h>   // for AXFX_BUFFERUPDATE definition

void    DelayInit       (void);
void    DelayShutdown   (void);
void    DelayCallback   (AXFX_BUFFERUPDATE *update, void *user);
