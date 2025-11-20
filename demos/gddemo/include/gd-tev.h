/*---------------------------------------------------------------------------*
  Project:  Dolphin GD demo
  File:     gd-tev.h

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/include/gd-tev.h $
    
    2     10/18/02 11:07p Hirose
    Fixed quantize shift amount.
    
    1     10/04/01 2:47p Hirose
    Initial check in.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   gd-tev
     Displaylist demo with multitexture shader commands
     [Header file]
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <dolphin/gd.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define QUANTIZE_SHIFT  14
#define QSCALE          (1<<QUANTIZE_SHIFT)

#define MODEL_MESHX     16
#define MODEL_MESHY     16
#define MODEL_ZSCALE    0.65F

#define BUMP_SCALE      0x50
#define REFLEX_SCALE    0x50
#define DIFFUSE_BASE    0xA0
#define SPECULAR_BASE   0x80

#define NUM_TEXTURES    4

#define NUM_SHADERDLS   4
#define NUM_MODELDLS    1
#define NUM_DLS         (NUM_SHADERDLS+NUM_MODELDLS)
#define NUM_PLS         NUM_SHADERDLS

#define MDL_SIZE_MAX    4096
#define SDL_SIZE_MAX    512
#define PL_SIZE_MAX     96

/*---------------------------------------------------------------------------*
   External function references
 *---------------------------------------------------------------------------*/
extern void CreateModelDL   ( void* dlPtr, u32* dlSize );
extern void CreateShader0DL ( void* dlPtr, u32* dlSize, u32* plPtr, u32* plSize );
extern void CreateShader1DL ( void* dlPtr, u32* dlSize, u32* plPtr, u32* plSize );
extern void CreateShader2DL ( void* dlPtr, u32* dlSize, u32* plPtr, u32* plSize );
extern void CreateShader3DL ( void* dlPtr, u32* dlSize, u32* plPtr, u32* plSize );


/*============================================================================*/
