/*---------------------------------------------------------------------------*
  Project:  Dolphin AX + synth Demo
  File:     syndemo.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/axdemo/src/syndemo.c $
    
    9     12/11/01 7:02p Billyjack
    - keep interrupts disabled during audio frame callback
    
    8     9/05/01 4:33p Eugene
    Updated AM API. 
    
    7     8/29/01 1:52p Billyjack
    
    6     8/17/01 10:59a Billyjack
    changed AMLoadFile() API
    
    5     8/16/01 12:25p Billyjack
    - now uses AM
    - changed code for SYN and SEQ init API change 
    
    4     8/03/01 4:32p Billyjack
    added OSEnableInterrupts() and OSRestoreInterrupts() to AX aufdio frame
    callback per change in AX lib.
    
    3     7/06/01 11:50a Billyjack
    commented DCInvalidateRange for DVD to RAM transfers
    
    2     5/14/01 1:39p Billyjack
    - reworked for DVDDATA file location and names
    - uses ARGetBaseAddress where applicable
    
    1     5/09/01 6:09p Billyjack
    created
   
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This program shows how to set up and use the AX
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <dolphin.h>
#include <dolphin/mix.h>
#include <dolphin/syn.h>
#include <dolphin/axfx.h>
#include <dolphin/am.h>

#define MAX_ARAM_BLOCKS     2
static  u32 aramMemArray[MAX_ARAM_BLOCKS];

static u8 noteA = 0;
static u8 noteNumber = 0;
static u8 program = 0;
static int programChangeButton;

SYNSYNTH    synth;

static u32 hold = 0;


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void callbackDropVoice(void *p)
{
    AXVPB *pvpb = p;

    OSReport("PVPB %.8x is being dropped context %d\n", pvpb, pvpb->userContext);
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void callbackAudioFrame(void)
{
    // run the synth
    SYNRunAudioFrame();

    // tell the mixer to update settings
    MIXUpdateSettings();
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void programChange(void)
{
	u8 ch[3];
    int old;
 
    if (programChangeButton)
        return;

    programChangeButton = 1;

	program++;
    program %= 128;

    ch[0] = 0xC0;
    ch[1] = program;
    OSReport("program: %d\n", program);

    old = OSDisableInterrupts();

    SYNMidiInput(&synth, ch);

    OSRestoreInterrupts(old);
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void programChange_(void)
{
    programChangeButton = 0;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void startNote(void)
{
    if (noteA == 0)
    {
        u8 ch[3];
        int old;

        ch[0] = 0x90;
        ch[1] = noteNumber;
        ch[2] = 0x7F;

        old = OSDisableInterrupts();

        SYNMidiInput(&synth, ch);
        
        OSRestoreInterrupts(old);
        OSReport("program: %d keyNum: %d\n", program, noteNumber);
        noteA = 1;
    }
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void stopNote(void)
{
    if (noteA)
    {
        u8 ch[3];
        int old;

        ch[0] = 0x80;
        ch[1] = noteNumber;
        ch[2] = 0x00;

        old = OSDisableInterrupts();

        SYNMidiInput(&synth, ch);
        
        noteA = 0;

        noteNumber++;
        noteNumber %= 128;

        if (noteNumber == 127)
        {
            program++;
            program %= 128;

            ch[0] = 0xC0;
            ch[1] = program;

            SYNMidiInput(&synth, ch);
        }

        OSRestoreInterrupts(old);
    }
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void holdPedalDown(void)
{
    u8 ch[3];
    int old;
    
    if (hold)
        return;

    ch[0] = 0xB0;
    ch[1] = 0x40;
    ch[2] = 127;

    old = OSDisableInterrupts();

    SYNMidiInput(&synth, ch);

    OSRestoreInterrupts(old);

    hold = 1;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void holdPedalUp(void)
{
    if (hold)
    {
        u8 ch[3];
        int old;

        ch[0] = 0xB0;
        ch[1] = 0x40;
        ch[2] = 0;

        old = OSDisableInterrupts();

        SYNMidiInput(&synth, ch);

        OSRestoreInterrupts(old);
        
        hold = 0;
    }
}

/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
void main(void)
{
    u32        aramBase;
    u32        aramSize;
    PADStatus  pads[PAD_MAX_CONTROLLERS];
    void      *wt;
    u32        aramAddr;       
	u32		   vFrames;

        DEMOInit(NULL);

        ARInit(aramMemArray, MAX_ARAM_BLOCKS);
        ARQInit();
        AIInit(NULL);

        AXInit();
        MIXInit();
        SYNInit();

        // for the purpose of this demo we take all the ARAM HAHA! 
        aramSize = ARGetSize() - ARGetBaseAddress();
        aramBase = ARAlloc(aramSize);
        AMInit(aramBase, aramSize); 

        aramAddr    = AMPush("/axdemo/synth/gm16pcm.pcm");
        wt          = AMLoadFile("/axdemo/synth/gm16pcm.wt", NULL);

        // register user callback for audio frames notification
        AXRegisterCallback(&callbackAudioFrame);

        // initialize synth
        SYNInitSynth(
            &synth,             // pointer to synth object
            wt,                 // pointer to wavetable
            aramAddr,           // address of samples in ARAM
            AMGetZeroBuffer(),  // address of zero buffer in ARAM
            16,                 // priority for voice allocation
            15,                 // priority for notes
            1                   // priority for notes that have been released
            );

        OSReport("Press the A button to increment key number and play note.\n");
        OSReport("Press the B button to inclrement program number and change program.\n");
        OSReport("Press the left trigger to hold pedal.\n");
    
        OSReport("Press Menu button to exit program\n");
	    
	    vFrames = 0;
        programChangeButton = 0;

        while (1)
        {
            // wait for retrace
            VIWaitForRetrace();

            // check pad
            PADRead(pads);

            // see if we should quit
            if (pads[0].button & PAD_BUTTON_MENU)
                break;

            // use A button for key
            if (pads[0].button & PAD_BUTTON_A)
                startNote();
            else
                stopNote();

            // use B button for program change
            if (pads[0].button & PAD_BUTTON_B)
                programChange();
            else
                programChange_();

            // use the left trigger for a hold pedal
            if (pads[0].button & PAD_TRIGGER_L)
                holdPedalDown();
            else
                holdPedalUp();
        }

        SYNQuit();
        MIXQuit();
        AXQuit();

        if (wt)
        {
            OSFree(wt);
        }

        OSHalt("End of program\n");    


} // end main()