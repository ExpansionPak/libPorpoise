/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - PAD module for core loop manager
File:     corePAD.c

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

// ========================================
//  PAD input management module for core loop manager
// ========================================
// Initializes.
//   Initializes PAD.
// Task.
//   Calls callback if PAD input changes.


//  HEADER INCLUDE
// ====================
#include <dolphin.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "corePAD.h"

//  INTERNAL DEFINES
// ====================

//  INTERNAL VARIABLES
// ====================
static PADStatus Pads[PAD_MAX_CONTROLLERS]; // Status of PAD.
static u16  gLastButton=0;                  // Status of previous BUTTON.
static u32  gLastCount=0;                   // Number of calls.
static CORE_PAD_CALLBACK gCallback=NULL;    // Callback to be called if PAD status changes.

//  INTERNAL FUNCTION PROTOTYPES
// ====================

// ============================================================
//  CORE API
// ============================================================
void CorePAD_Begin()
{
    PADReset(PAD_CHAN0_BIT|PAD_CHAN1_BIT|PAD_CHAN2_BIT|PAD_CHAN3_BIT);
}

void CorePAD_Task( void* dummy)
{
#pragma unused(dummy)
    u32 count;
    if( gLastCount == (count=VIGetRetraceCount()) ){
    }else{
        u16 upButton,button;
        //  Pad Keyin
        PADRead(Pads);
        button = Pads[0].button;
        if( PADButtonUp(gLastButton, button)||
            PADButtonDown(gLastButton, button))
        {
            if(!gCallback){
                OSReport("Button:%08X \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",button);
                upButton = PADButtonUp(gLastButton, button);
                OSReport("UpButton:%08X\n",upButton);
            }else{
                gCallback(
                    PADButtonUp(gLastButton, button),
                    PADButtonDown(gLastButton, button));
            }
        }
        gLastButton = button;
        // VI_RETRANCE_COUNT
        gLastCount = count;
    }
}

// ============================================================
//  API
// ============================================================
void CorePAD_SetCallback( CORE_PAD_CALLBACK callback )
{
    gCallback = callback;
}
