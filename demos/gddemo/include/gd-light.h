/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gd-light.h

  Copyright 1998 - 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/include/gd-light.h $
    
    1     9/21/01 4:08p Hirose
    Initial check in.
  
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   gd-light
     Displaylist demo with lighting commands
     [Header file]
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <dolphin/gd.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
// Torus model geometric properties
#define MODEL_N0            24
#define MODEL_N1            64
#define MODEL_R             0.30F
#define QUANTIZE            14
#define SCALE_Q             (1<<QUANTIZE)

// Display list properties
#define MODELDL_SIZE_MAX            16384

#define MODELDL_PATCH_LIGHT0POS     0
#define MODELDL_PATCH_LIGHT1DIR     1
#define MODELDL_NUM_PATCHES         2

/*---------------------------------------------------------------------------*
   External function definitions
 *---------------------------------------------------------------------------*/
extern u32  CreateModelDL   ( void* dlPtr, u32* plPtr );



/*============================================================================*/
