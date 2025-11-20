/*---------------------------------------------------------------------------*
  Project:  Dolphin AX + synth Demo
  File:     syndemo.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/axdemo/src/seqdemo.c $
    
    12    12/11/01 7:02p Billyjack
    - keep interrupts disabled during audio frame callback
    
    11    9/06/01 3:17p Billyjack
    fixed profile bug
    
    10    9/05/01 4:33p Eugene
    Updated AM API. 
    
    9     8/29/01 1:52p Billyjack
    
    8     8/17/01 10:59a Billyjack
    changed AMLoadFile() API
    
    7     8/16/01 12:25p Billyjack
    - now uses AM
    - changed code for SYN and SEQ init API change 
    
    6     8/03/01 4:32p Billyjack
    added OSEnableInterrupts() and OSRestoreInterrupts() to AX aufdio frame
    callback per change in AX lib.
    
    5     7/06/01 11:50a Billyjack
    commented DCInvalidateRange for DVD to RAM transfers
    
    4     5/21/01 12:01p Billyjack
    changed demowin.h location
    
    3     5/14/01 1:39p Billyjack
    - reworked for DVDDATA file location and names
    - uses ARGetBaseAddress where applicable
    
    2     5/10/01 7:03p Billyjack
    now uses DEMOWin
    
    1     5/09/01 6:09p Billyjack
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This program shows how to set up and use the AX
 *---------------------------------------------------------------------------*/

#include <string.h>
#include <demo.h>
#include <dolphin.h>
#include <dolphin/mix.h>
#include <dolphin/seq.h>
#include <dolphin/syn.h>
#include <dolphin/axfx.h>
#include <dolphin/am.h>
#include <demo/demowin.h>



static volatile int samplesLoaded;
static u32 maxCpuCycles, maxAuxCycles, maxUserCycles, maxAxCycles, maxVoices;

static AXFX_REVERBSTD   reverbStd;
static AXFX_REVERBHI    reverbHi;
static AXFX_CHORUS      chorus;
static AXFX_DELAY       delay;
static AXPROFILE        profile[256];
static u32              auxa;
static u32              auxb;
static u32              samples;

#define MAX_ARAM_BLOCKS     2
static u32 aramMemArray[MAX_ARAM_BLOCKS];
static ARQRequest   taskPCM;

static SEQSEQUENCE  sequence;
static u32          sequenceInitialized;

static void         *wavetable;
static void         *wavetablePcm;
static void         *wavetableAdpcm;
static u32          arambase;
static u32          aramBasePcm;
static u32          aramBaseAdpcm;
static u32          zeroBase;
static u8           *midiFile;
static char         filename[1024];

DEMOWinInfo *log_win;

static u32          exitProgram;

/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void callbackAudioFrame(void)
{
    // run the sequencer
    SEQRunAudioFrame();

    // run the synth
    SYNRunAudioFrame();

    // tell the mixer to update settings
    MIXUpdateSettings();
}


/*---------------------------------------------------------------------------*
    setup the audio system for MIDI sequencer playback and profiling
 *---------------------------------------------------------------------------*/
static void setupAudioSystem(void)
{
    AXInit();   // initialize AX
    MIXInit();  // initialize mixer
    SYNInit();  // initialize synthesizer
    SEQInit();  // initialize sequencer

    AXRegisterCallback(&callbackAudioFrame);
    AXInitProfile(profile, 256); // initialize profiling for AX

    reverbStd.tempDisableFX     = FALSE;
    reverbStd.time              = 3.0f;
    reverbStd.preDelay          = 0.1f;
    reverbStd.damping           = 0.5f;
    reverbStd.coloration        = 0.5f;
    reverbStd.mix               = 0.5f;

    reverbHi.tempDisableFX      = FALSE;
    reverbHi.time               = 3.0f;
    reverbHi.preDelay           = 0.1f;
    reverbHi.damping            = 0.5f;
    reverbHi.coloration         = 0.5f;
    reverbHi.crosstalk          = 0.3f;
    reverbHi.mix                = 0.5f;

    chorus.baseDelay            = 15;
    chorus.variation            = 0;
    chorus.period               = 500;

    delay.delay[0]              = 500;
    delay.delay[1]              = 500;
    delay.delay[2]              = 500;
    delay.feedback[0]           = 50;
    delay.feedback[1]           = 50;
    delay.feedback[2]           = 50;
    delay.output[0]             = 100;
    delay.output[1]             = 100;
    delay.output[2]             = 100;
    
    AXFXReverbStdInit(&reverbStd);  // initialize reverb
    AXFXReverbHiInit(&reverbHi);    // initialize reverb
    AXFXChorusInit(&chorus);        // initialize chorus
    AXFXDelayInit(&delay);          // initialize delay
}


/*---------------------------------------------------------------------------*
    shutdown the audio system
 *---------------------------------------------------------------------------*/
static void shutdownAudioSystem(void)
{
    AXFXReverbStdShutdown(&reverbStd);
    AXFXReverbHiShutdown(&reverbHi);
    AXFXChorusShutdown(&chorus);
    AXFXDelayShutdown(&delay);

    SEQQuit();
    SYNQuit();
    MIXQuit();
    AXQuit();
}


/*---------------------------------------------------------------------------*
 * Directory listing stuff!
 *---------------------------------------------------------------------------*/
#define MAX_DIR_ENTRIES     512
#define MAX_FILENAME_LENGTH 128

DEMOWinMenuItem dir_entry[MAX_DIR_ENTRIES+1];
DEMOWinMenuInfo dir_list;

char dir_entry_name[MAX_DIR_ENTRIES][MAX_FILENAME_LENGTH];


/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

static void MNU_change_dir_up(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    menu->items[item].flags |= DEMOWIN_ITM_EOF;

    DEMOWinLogPrintf(log_win, "Changing to parent directory.\n");

    DVDChangeDir("..");

    *result = 0xDEADBEEF;

} // MNU_change_dir_up


/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
static void MNU_change_dir(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)


    DVDDir      dir;
    DVDDirEntry dirent;

    u32 i;

        DEMOWinLogPrintf(log_win, "Changing directory to: '%s'.\n", menu->items[item].name);

        DVDOpenDir(".", &dir);
        for (i=1; i<=item; i++)
        {
            DVDReadDir(&dir, &dirent);
        }
        if (dirent.isDir)
        {
            DVDChangeDir(dirent.name);
            menu->items[item].flags |= DEMOWIN_ITM_EOF;

        }
        else
        {
            DEMOWinLogPrintf(log_win, "Unable to change directory...??\n");
            menu->items[item].flags &= ~DEMOWIN_ITM_EOF;
        }

        DVDCloseDir(&dir);

        *result = 0xDEADBEEF;

} // MNU_change_dir

static char toupper(char c)
{

    if ((c >= 'a') && (c <= 'z'))
        return((char)(c-32));
    else
        return(c);
}


static BOOL compare_strings(char *s1, char *s2)
{
    u32 i;

        for (i=0; i<strlen(s1); i++)
        {
            if (toupper(s1[i]) != toupper(s2[i]))
                return(FALSE);
        }
        return(TRUE);
}


static void MNU_select_file(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
    u32 i;

        i = strlen(menu->items[item].name);
    
        while ((i >=0) && (menu->items[item].name[i] != '.'))
            i--;

        if (compare_strings(&menu->items[item].name[i], ".mid")) 
        {
            strcpy(filename, menu->items[item].name);

            DEMOWinLogPrintf(log_win, "Selected %s.\n", menu->items[item].name);
            midiFile = AMLoadFile(menu->items[item].name, NULL);

            if (midiFile)
            {
                DEMOWinLogPrintf(log_win, "%s loaded into memory.\n", menu->items[item].name);

                SEQAddSequence(
                    &sequence,
                    midiFile,
                    wavetable,
                    arambase,
                    zeroBase,
                    16,
                    15,
                    1
                    );

                DEMOWinLogPrintf(log_win, "%s initialized\n", menu->items[item].name);
                sequenceInitialized = 1;
            }    
        
            *result = sequenceInitialized;
        }
        else
        {
            DEMOWinLogPrintf(log_win, "--> File '%s' is not an .mid file!\n", menu->items[item].name);
        }
       
} // end __select_file

/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
static DEMOWinMenuInfo *__create_dir_menu(void)
{

    DVDDir      dir;
    DVDDirEntry dirent;

    u32         index;
    u32         len;

        index = 0;


        // first entry is always parent directory
        strcpy(dir_entry_name[index], "../");

        dir_entry[index].name     = &dir_entry_name[index][0];
        dir_entry[index].flags    = DEMOWIN_ITM_EOF;
        dir_entry[index].function = MNU_change_dir_up;
        dir_entry[index].link     = NULL;

        index++;


        DVDOpenDir(".", &dir);

        while ( (DVDReadDir(&dir, &dirent)) && (index < MAX_DIR_ENTRIES))
        {
            // get name, append '/' if it's a directory
            strcpy(dir_entry_name[index], dirent.name);
            if (dirent.isDir)
            {
                len = strlen(dirent.name);
                dir_entry_name[index][len]   = '/';
                dir_entry_name[index][len+1] = 0;

                dir_entry[index].function = MNU_change_dir;
                dir_entry[index].flags    = DEMOWIN_ITM_EOF;
            }
            else
            {
                dir_entry[index].function = MNU_select_file;
                dir_entry[index].flags    = DEMOWIN_ITM_EOF;
            }

            dir_entry[index].name     = &dir_entry_name[index][0];
            dir_entry[index].link  = NULL;
            index++;
        }

        // terminate list
        dir_entry[index].flags    = DEMOWIN_ITM_TERMINATOR;
        dir_entry[index].function = NULL;
        dir_entry[index].link     = NULL;


        // Initialize Menu info structure

        dir_list.title  = "Directory Listing";  // later, init with directory name
        dir_list.handle = NULL;                 // placeholder for window handle
        dir_list.items  = &dir_entry[0];        // list of items

        dir_list.max_display_items = 24;        // display 22 files at a time
        dir_list.flags             = 0;         // no flags
    
        dir_list.cb_open   = NULL;              // no callbacks
        dir_list.cb_move   = NULL;
        dir_list.cb_select = NULL;
        dir_list.cb_cancel = NULL;

        // private members; initialize to zero, they will be calculated at runtime
        dir_list.num_display_items = 0;
        dir_list.num_items         = 0;  
        dir_list.max_str_len       = 0;
        dir_list.curr_pos          = 0;   
        dir_list.display_pos       = 0;

        DVDCloseDir(&dir);

        if (index)
        {
            return(&dir_list);
        }
        else
        {
            return(NULL);
        }


} // end __create_dir_menu()

/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_select(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    DEMOWinInfo     *p; // parent window
    DEMOWinMenuInfo *m; // directory menu

    u32 val = 0;

    DEMOWinLogPrintf(log_win, "Please select MIDI file to play.\n");

    if (sequenceInitialized)
    {
        DEMOWinLogPrintf(log_win, "Removing sequence.\n");
                    
        SEQRemoveSequence(&sequence);
        sequenceInitialized = 0;
        
        if (midiFile)
        {
            DEMOWinLogPrintf(log_win, "Free MIDI file from memory.\n");
            OSFree(midiFile);
        }
    }

    p = menu->handle;

    do 
    {
        m = __create_dir_menu();

        if (m)
        {      
            DEMOWinCreateMenuWindow(m, (u16)(p->x2-10), (u16)(p->y1+24));
            val = DEMOWinMenu(m);
            DEMOWinDestroyMenuWindow(m);
        }
        else
        {
            DEMOWinLogPrintf(log_win, "Failed to create directory list.\n");
        }
            
    } while (val == 0xDEADBEEF);
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_play(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    if (sequenceInitialized)
    {
        DEMOWinLogPrintf(log_win, "Setting sequence state to SEQ_STATE_RUN.\n");
        SEQSetState(&sequence, SEQ_STATE_RUN);
    }
    else
    {
        DEMOWinLogPrintf(log_win, "Please select a MIDI file to play!!!\n");
    }
}

/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_playloop(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    if (sequenceInitialized)
    {
        DEMOWinLogPrintf(log_win, "Setting sequence state to SEQ_STATE_RUNLOOPED.\n");
        SEQSetState(&sequence, SEQ_STATE_RUNLOOPED);
    }
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_pause(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    if (sequenceInitialized)
    {
        DEMOWinLogPrintf(log_win, "Setting sequence state to SEQ_STATE_PAUSE.\n");
        SEQSetState(&sequence, SEQ_STATE_PAUSE);
    }
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_stop(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    if (sequenceInitialized)
    {
        DEMOWinLogPrintf(log_win, "Setting sequence state to SEQ_STATE_STOP.\n");
        SEQSetState(&sequence, SEQ_STATE_STOP);
    }
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_auxa(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    auxa ++;

    switch(auxa % 3)
    {
    case 0:

        AXRegisterAuxACallback(NULL, NULL);
        menu->items[item].name = "  AUX A None";

        break;

    case 1:

        AXRegisterAuxACallback((void*)&AXFXReverbStdCallback, (void*)&reverbStd);    
        menu->items[item].name = "  AUX A AXFXReverbStd";

        break;

    case 2:
        
        AXRegisterAuxACallback((void*)&AXFXReverbHiCallback, (void*)&reverbHi);    
        menu->items[item].name = "  AUX A AXFXReverbHi";

        break;
    }
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_auxb(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    auxb ++;

    switch(auxb % 3)
    {
    case 0:

        AXRegisterAuxBCallback(NULL, NULL);
        menu->items[item].name = "  AUX B None";

        break;

    case 1:

        AXRegisterAuxBCallback((void*)&AXFXChorusCallback, (void*)&chorus);    
        menu->items[item].name = "  AUX B AXFXChorus";

        break;

    case 2:
        
        AXRegisterAuxBCallback((void*)&AXFXDelayCallback, (void*)&delay);    
        menu->items[item].name = "  AUX B AXFXDelay";

        break;
    }
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_sample(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    samples ++;

    switch(samples % 2)
    {
    case 0:

        wavetable = wavetablePcm;
        arambase  = aramBasePcm;

        menu->items[item].name = "  PCM Samples";

        break;

    case 1:

        wavetable = wavetableAdpcm;
        arambase  = aramBaseAdpcm;

        menu->items[item].name = "  ADPCM Samples";

        break;
    }

    // if the sequence is running, initialize the synth with the new wavetable
    if ((SEQGetState(&sequence) == SEQ_STATE_RUN) ||
        (SEQGetState(&sequence) == SEQ_STATE_RUNLOOPED))
    {
        u32 state = SEQGetState(&sequence);

        DEMOWinLogPrintf(log_win, "Stopping playback to change wavetable.\n");
        SEQSetState(&sequence, SEQ_STATE_STOP);
    }

    if (sequenceInitialized)
    {
        SYNQuitSynth(&sequence.synth);

        SYNInitSynth(
            &sequence.synth,
            wavetable,
            arambase,
            zeroBase,
            16,
            15,
            1
            );
    }

    if (arambase == 256)
        DEMOWinLogPrintf(log_win, "Now using PCM samples.\n");
    else
        DEMOWinLogPrintf(log_win, "Now using ADPCM samples.\n");
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_clearmax(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    maxCpuCycles = maxAuxCycles = maxUserCycles = maxAxCycles = maxVoices = 0;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void MNU_exit(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    exitProgram = 1;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void updateProfile(DEMOWinInfo *window)
{
    u32 cpuCycles, auxCycles, userCycles, axCycles, voices, i;

    i = AXGetProfile();

    if (i)
    {
        while (i)
        {
//            i--;

            cpuCycles   = (u32)(profile[i].axFrameEnd - profile[i].axFrameStart);
            auxCycles   = (u32)(profile[i].auxProcessingEnd - profile[i].auxProcessingStart);
            userCycles  = (u32)(profile[i].userCallbackEnd - profile[i].userCallbackStart);
            axCycles    = cpuCycles - auxCycles - userCycles;
            voices      = profile[i].axNumVoices;

            if (cpuCycles > maxCpuCycles)       maxCpuCycles    = cpuCycles;
            if (auxCycles > maxAuxCycles)       maxAuxCycles    = auxCycles;
            if (userCycles > maxUserCycles)     maxUserCycles   = userCycles;
            if (axCycles > maxAxCycles)         maxAxCycles     = axCycles;
            if (voices > maxVoices)             maxVoices       = voices;

            i--;
        }

        DEMOWinLogPrintf(window,"Current profile for 5ms frame\n\n");
        DEMOWinLogPrintf(window,"Total:         %f\%\n", (f32)OSTicksToNanoseconds(cpuCycles) / 50000);
        DEMOWinLogPrintf(window,"AX:            %f\%\n", (f32)OSTicksToNanoseconds(axCycles) / 50000);
        DEMOWinLogPrintf(window,"AUX:           %f\%\n", (f32)OSTicksToNanoseconds(auxCycles) / 50000);
        DEMOWinLogPrintf(window,"MIX, SYN, SEQ: %f\%\n", (f32)OSTicksToNanoseconds(userCycles) / 50000);
        DEMOWinLogPrintf(window,"Voices:        %d\n", voices);

        DEMOWinLogPrintf(window,"\nMax profile for 5ms frame\n\n");
        DEMOWinLogPrintf(window,"Total:         %f\%\n", (f32)OSTicksToNanoseconds(maxCpuCycles) / 50000);
        DEMOWinLogPrintf(window,"AX:            %f\%\n", (f32)OSTicksToNanoseconds(maxAxCycles) / 50000);
        DEMOWinLogPrintf(window,"AUX:           %f\%\n", (f32)OSTicksToNanoseconds(maxAuxCycles) / 50000);
        DEMOWinLogPrintf(window,"MIX, SYN, SEQ: %f\%\n", (f32)OSTicksToNanoseconds(maxUserCycles) / 50000);
        DEMOWinLogPrintf(window,"Voices:        %d\n", maxVoices);
    }
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void updateStatus(DEMOWinInfo *window)
{
    if (sequenceInitialized)
    {
        DEMOWinLogPrintf(window, "MIDI File: %s\n", filename);
        DEMOWinLogPrintf(window, "Tracks:    %d\n", sequence.nTracks);
        DEMOWinLogPrintf(window, "Remaining: %d\n", sequence.tracksRunning);
    }
    else
    {
        DEMOWinLogPrintf(window, "MIDI File: None\n");
        DEMOWinLogPrintf(window, "Tracks:    0\n");
        DEMOWinLogPrintf(window, "Running:   0\n");
    }
    
    switch (SEQGetState(&sequence))
    {
    case SEQ_STATE_STOP:

        DEMOWinLogPrintf(window, "State:     SEQ_STATE_STOP\n");

        break;

    case SEQ_STATE_RUN:

        DEMOWinLogPrintf(window, "State:     SEQ_STATE_RUN\n");

        break;
    
    case SEQ_STATE_RUNLOOPED:

        DEMOWinLogPrintf(window, "State:     SEQ_STATE_RUNLOOPED\n");
        
        break;
    
    case SEQ_STATE_PAUSE:

        DEMOWinLogPrintf(window, "State:     SEQ_STATE_PAUSE\n");

        break;
    }


    DEMOWinLogPrintf(window, "Voices:    %d\n", SYNGetActiveNotes(&sequence.synth));
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
DEMOWinMenuItem control_menu_items[] = 
{
    { "",                       DEMOWIN_ITM_SEPARATOR,  NULL,           NULL },
    { "  Select MIDI File",     DEMOWIN_ITM_NONE,       MNU_select,     NULL },
    { "",                       DEMOWIN_ITM_SEPARATOR,  NULL,           NULL },
    { "  Play",                 DEMOWIN_ITM_NONE,       MNU_play,       NULL },
    { "  Play Looped",          DEMOWIN_ITM_NONE,       MNU_playloop,   NULL },
    { "  Pause",                DEMOWIN_ITM_NONE,       MNU_pause,      NULL },
    { "  Stop",                 DEMOWIN_ITM_NONE,       MNU_stop,       NULL },
    { "",                       DEMOWIN_ITM_SEPARATOR,  NULL,           NULL },
    { "  AUX A None",           DEMOWIN_ITM_NONE,       MNU_auxa,       NULL },
    { "  AUX B None",           DEMOWIN_ITM_NONE,       MNU_auxb,       NULL },
    { "",                       DEMOWIN_ITM_SEPARATOR,  NULL,           NULL },
    { "  PCM Samples",          DEMOWIN_ITM_NONE,       MNU_sample,     NULL },
    { "",                       DEMOWIN_ITM_SEPARATOR,  NULL,           NULL },
    { "  Clear Max Profile",    DEMOWIN_ITM_NONE,       MNU_clearmax,   NULL },
    { "",                       DEMOWIN_ITM_SEPARATOR,  NULL,           NULL },
    { "  Exit Program",         DEMOWIN_ITM_EOF,        MNU_exit,       NULL },
    { "",                       DEMOWIN_ITM_TERMINATOR, NULL,           NULL }
};

DEMOWinMenuInfo control_menu = 
{
#ifdef NDEBUG
   "Seqdemo - NDEBUG build", // title
#else
   "Seqdemo - DEBUG build", // title
#endif
    NULL,                   // window handle
    control_menu_items,     // list of menu items
    18,                     // max num of items to display at a time
    DEMOWIN_MNU_NONE,       // attribute flags?

    // user callbacks for misc menu operations
    NULL, NULL, NULL, NULL, 
    // private menu info members; do not touch
    0, 0, 0, 0, 0
}; 

DEMOWinMenuInfo *control_menu_ptr;


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
void main(void)
{
    u32 aramBase;
    u32 aramSize;
    DEMOWinInfo *profile_win;
    DEMOWinInfo *status_win;
    

        // set flags used for menu
        auxa = auxb = samples = 0;
        exitProgram = 0;
        sequenceInitialized = 0;

        // initialize bean counters for profiling
        maxCpuCycles = maxAuxCycles = maxUserCycles = maxAxCycles = maxVoices = 0;

        // initialize subsystems
        DEMOInit(NULL);
        DEMOWinInit();
        ARInit(aramMemArray, MAX_ARAM_BLOCKS);
        ARQInit();
        AIInit(NULL);

        // initialize windows on the TV
        control_menu_ptr = DEMOWinCreateMenuWindow(&control_menu, 20, 20);
    
        profile_win = DEMOWinCreateWindow(300, 20, 600, 170, "AX Profile", 1024, updateProfile);        
        status_win  = DEMOWinCreateWindow(300, 175, 600, 245, "Sequence Status", 1024, updateStatus);
        log_win     = DEMOWinCreateWindow(20, 250, 600, 400, "Log", 1024, NULL);

        DEMOWinOpenWindow(control_menu_ptr->handle);
        DEMOWinOpenWindow(profile_win);
        DEMOWinOpenWindow(status_win);
        DEMOWinOpenWindow(log_win);

        DEMOWinLogPrintf(log_win, "Setting up audio\n");
        setupAudioSystem();

        // for the purpose of this demo we take all the ARAM *chuckle*
        aramSize = ARGetSize() - ARGetBaseAddress();
        aramBase = ARAlloc(aramSize);
        AMInit(aramBase, aramSize); 
    
        // push sample data into ARAM
        aramBasePcm     = AMPush("/axdemo/synth/gm16pcm.pcm");
        aramBaseAdpcm   = AMPush("/axdemo/synth/gm16adpcm.pcm");

        // load table data into main memory
        wavetablePcm    = AMLoadFile("/axdemo/synth/gm16pcm.wt", NULL);
        wavetableAdpcm  = AMLoadFile("/axdemo/synth/gm16adpcm.wt", NULL);
    
        // default to PCM samples
        wavetable   = wavetablePcm;
        arambase    = aramBasePcm;
        zeroBase    = AMGetZeroBuffer();

        while (1)
        {
            DEMOWinMenu(control_menu_ptr);

            if (exitProgram == 1)
                break;
        }

        DEMOWinLogPrintf(log_win, "Shutting down up audio\n");
        DEMOBeforeRender();
        DEMOWinRefresh();
        DEMODoneRender();

        shutdownAudioSystem();

        DEMOWinLogPrintf(log_win, "Free wavetable\n");
        DEMOBeforeRender();
        DEMOWinRefresh();
        DEMODoneRender();

        if (wavetablePcm)
            OSFree(wavetablePcm);

        if (wavetableAdpcm)
            OSFree(wavetableAdpcm);

        if (midiFile)
            OSFree(midiFile);

        OSHalt("End of program\n");    
}