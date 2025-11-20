/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     smp-toy-stick.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Simple/smp-toy-stick.c $    
    
    3     3/27/00 3:24p Hirose
    changed to use DEMOPad library
    
    2     3/21/00 3:24p Hirose
    deleted PADInit() call because this function is called once in
    DEMOInit()
    
    1     3/16/00 7:24p Alligator
    rename to match other demos conventions
    
    1     3/06/00 12:11p Alligator
    move from gxdemos to gxdemos/Simple
    
    2     2/24/00 7:06p Yasu
    Rename gamepad key to match HW1
    
    4     9/15/99 3:00p Ryan
    update to fix compiler warnings
    
    3     7/23/99 2:51p Ryan
    changed dolphinDemo.h to demo.h
    
    2     7/23/99 12:15p Ryan
    included dolphinDemo.h
    
    1     6/17/99 1:25a Alligator
    simple interactive toy.  demonstrates texture api, multiple vertex
    formats and many primitive types.
    

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include "toy-stick.h"

#define AX_ZERO         0.9F
#define AY_ZERO         0.9F
#define DIGX_INC        0.05F
#define DIGY_INC        0.05F
#define DEAD_X          0.03F
#define DEAD_Y          0.03F

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static f32       AnalogX  = 0.0F;
static f32       AnalogY  = 0.0F;

/*---------------------------------------------------------------------------*
    Name:           GetAnalogX
    
    Description:    returns value of x tilt
                    
    Arguments:      none
    
    Returns:        f32, x tilt value
 *---------------------------------------------------------------------------*/
f32 GetAnalogX( void )
{
    return(AnalogX);
}

/*---------------------------------------------------------------------------*
    Name:           GetAnalogY
    
    Description:    returns value of y tilt
                    
    Arguments:      none
    
    Returns:        f32, y tilt value
 *---------------------------------------------------------------------------*/
f32 GetAnalogY( void )
{
    return(AnalogY);
}

/*---------------------------------------------------------------------------*
    Name:           StickTick
    
    Description:    computes analog tilt x, y 
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/

void StickTick( void )
{
    static f32 digitalX = 0.0F;
    static f32 digitalY = 0.0F;

    DEMOPadRead();

    //
    //    tilt table
    //
    if (DEMOPadGetStickX(0) > 0)
        digitalX = DIGX_INC;
    else if (DEMOPadGetStickX(0) < 0)
        digitalX = -DIGX_INC;
    else
        digitalX = 0.0F;
 
    AnalogX += digitalX;
    AnalogX *= AX_ZERO;
    if (AnalogX < DEAD_X && AnalogX > -DEAD_X)
        AnalogX = 0.0F;

    if (DEMOPadGetStickY(0) > 0)
        digitalY = DIGY_INC;
    else if (DEMOPadGetStickY(0) < 0)
        digitalY = -DIGY_INC;
    else
        digitalY = 0.0F;

    AnalogY += digitalY;
    AnalogY *= AY_ZERO;
    if (AnalogY < DEAD_Y && AnalogY > -DEAD_Y)
        AnalogY = 0.0F;
}

/*---------------------------------------------------------------------------*
    Name:           StickDone
    
    Description:    return true when START button pressed
                    
    Arguments:      none
    
    Returns:        u16
 *---------------------------------------------------------------------------*/
u16 StickDone(void)
{
    return((u16)(DEMOPadGetButton(0) & PAD_BUTTON_MENU));
}

