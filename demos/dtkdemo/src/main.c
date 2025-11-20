/*---------------------------------------------------------------------------*
 *       N I N T E N D O  C O N F I D E N T I A L  P R O P R I E T A R Y
 *---------------------------------------------------------------------------*
 *
 * Project: Dolphin OS - DTK Demo
 * File:    drivestuff.c
 *
 * Copyright 2001 Nintendo.  All rights reserved.
 *
 * These coded instructions, statements, and computer programs contain
 * proprietary information of Nintendo of America Inc. and/or Nintendo
 * Company Ltd., and are protected by Federal copyright law.  They may
 * not be disclosed to third parties or copied or duplicated in any form,
 * in whole or in part, without the prior written consent of Nintendo.
 *
 * $Log: /Dolphin/build/demos/dtkdemo/src/main.c $
    
    2     5/20/01 11:18p Eugene
    DEMOWIN.H moved from include/dolphin to include/demo
    
    1     5/10/01 5:06a Eugene
    Initial checkin. Fast hack job for DTK demo. 
    1. Allows users to build a play list from files in current directory. 
    2. Does not use DTKRemoveTrack() --> too painful to manage track list
    ordering for now. 
    3. Does allow users to flush the entire play list. 
    4. 32 track limit on play lists. 
    5. Using dynamic mem allocation for windows. sorry! 
 *   
 *
 * $NoKeywords: $
 *
 *---------------------------------------------------------------------------*/
 

/*---------------------------------------------------------------------------*
 * Includes
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>

#include <dolphin.h>

#include <demo/DEMOWin.h>

/*---------------------------------------------------------------------------*
 * Globals
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
 * Local definitions/declarations
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Module variables
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Function prototypes
 *---------------------------------------------------------------------------*/

void Init_DTK_System        (void);
void Init_Player_Windows    (void);
void Run_Player             (void);

/*===========================================================================*
 *                   F U N C T I O N    D E F I N I T I O N S
 *===========================================================================*/

 
/*---------------------------------------------------------------------------*
 * Name        : main()
 *
 * Description : 
 *
 * Arguments   : None.
 *
 * Returns     : None.
 *
 *---------------------------------------------------------------------------*/

   
void main(void)
{

    DEMOInit(NULL);
    DEMOWinInit();

    Init_DTK_System();
    Init_Player_Windows();

    Run_Player();


} // end main



