/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gd-light-gc-load.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-light-gc-load.c $
    
    1     9/24/01 2:24p Hirose
    Initial check in.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

// Note: This file exists only to work around makefile complications.
// This is how I compile the same source file with different #defines.

#define LOAD_DL_FROM_FILE
#include "../src/gd-light-gc.c"
