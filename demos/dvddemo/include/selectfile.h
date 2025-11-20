/*---------------------------------------------------------------------------*
  Project:  header file for file selection window system
  File:     errorhandling.h

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/dvddemo/include/selectfile.h $
    
    1     6/15/01 9:03p Hashida
    Initial revision.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <demo/DEMOWin.h>

void InitSelectFile(DEMOWinInfo *debug_win);
char* SelectFile(DEMOWinInfo *handle);

