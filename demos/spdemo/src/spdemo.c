/*---------------------------------------------------------------------------*
  Project:  SP Demo application
  File:     spdemo.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/spdemo/src/spdemo.c $
    
    1     9/05/01 8:09p Eugene
    Demonstration of SP and AM libraries. Uses AX!
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Includes
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <demo/DEMOWin.h>
#include <dolphin.h>
#include <dolphin/mix.h>
#include <dolphin/sp.h>
#include <dolphin/am.h>

#include "spdemo.h"


/*---------------------------------------------------------------------------*
 * SP data 
 *---------------------------------------------------------------------------*/

#define SPT_FILE "/SPDEMO/spdemo.spt"
#define SPD_FILE "/SPDEMO/spdemo.spd"

static SPSoundTable *sp_table;

/*---------------------------------------------------------------------------*
 * AX Profiling
 *---------------------------------------------------------------------------*/

// store up to 8 frames, just to be safe
#define NUM_AX_PROFILE_FRAMES 8

static AXPROFILE        ax_profile[NUM_AX_PROFILE_FRAMES]; 

 /*---------------------------------------------------------------------------*
 * ARAM initialization
 *---------------------------------------------------------------------------*/

// Use AR allocator to divide ARAM into 3 blocks
#define MAX_ARAM_BLOCKS  3

// Give a whopping 8MB of ARAM to audio!
#define AUDIO_BLOCK_SIZE_BYTES (8*1024*1024)


static u32  aramZeroBase;
static u32  aramUserBase;
static u32  aramMemArray[MAX_ARAM_BLOCKS];


// transfer buffer for ARAM audio manager (AM)
#define XFER_BUFFER_SIZE_BYTES (16*1024)

u8 xfer_buffer[XFER_BUFFER_SIZE_BYTES] ATTRIBUTE_ALIGN(32);

/*---------------------------------------------------------------------------*
 * Application-layer voice abstraction
 *---------------------------------------------------------------------------*/

#define MAX_DEMO_VOICES  64

typedef struct 
{
    AXVPB *ax_voice;
    SPSoundEntry *sp_entry;

} DEMO_VOICE;

DEMO_VOICE demo_voice[MAX_DEMO_VOICES];

// Checks SP entry 'type' to see if the voice is looped or not
#define mISLOOPED(x) ((x->type)&0x1)
 

/*---------------------------------------------------------------------------*
 * Prototypes
 *---------------------------------------------------------------------------*/

static DEMO_VOICE  *get_demo_voice          (void);
static void         init_demo_voices        (void);
static void         ax_demo_callback        (void);
static void         ax_drop_voice_callback  (void *p);
static void         play_sfx                (u32 sfx);

// for UI menus
static void         MNU_play_click          (DEMOWinMenuInfo *menu, u32 item);
static void         MNU_play_sfx            (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void         MNU_stop_sfx            (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void         MNU_stop_looping        (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void         ax_profile_update       (DEMOWinInfo *window);


/*---------------------------------------------------------------------------*
 * UI Stuff
 *---------------------------------------------------------------------------*/

DEMOWinInfo *DebugWin;
DEMOWinInfo *ProfileWin;

DEMOWinMenuItem MenuItem[] = 
{
    { "Sound Effect",           DEMOWIN_ITM_SEPARATOR,  NULL,             NULL },
    { "  Noisy Drum",           DEMOWIN_ITM_NONE,       MNU_play_sfx,     NULL },
    { "  Gunshot",              DEMOWIN_ITM_NONE,       MNU_play_sfx,     NULL },
    { "  Voice-Man",            DEMOWIN_ITM_NONE,       MNU_play_sfx,     NULL },
    { "  Voice-Woman",          DEMOWIN_ITM_NONE,       MNU_play_sfx,     NULL },
    { "  Looping Strings",      DEMOWIN_ITM_NONE,       MNU_play_sfx,     NULL },
    { " ",                      DEMOWIN_ITM_SEPARATOR,  NULL,             NULL },
    { "Voice Control",          DEMOWIN_ITM_SEPARATOR,  NULL,             NULL },
    { "  Stop All",             DEMOWIN_ITM_NONE,       MNU_stop_sfx,     NULL },
    { "  Stop All Looping",     DEMOWIN_ITM_NONE,       MNU_stop_looping, NULL },
    { " ",                      DEMOWIN_ITM_SEPARATOR,  NULL,             NULL },
    { "",                       DEMOWIN_ITM_TERMINATOR, NULL,             NULL }
};

DEMOWinMenuInfo Menu = 
{
    "AX Sound Pipeline Demo",   // title
    NULL,                       // window handle
    MenuItem,                   // list of menu items
    10,                         // max num of items to display at a time
    DEMOWIN_MNU_NONE,           // attribute flags

    // user callbacks
    NULL,                       // callback for menu open event 
    MNU_play_click,             // callback for cursor move event
    NULL,                       // callback for item select event
    NULL,                       // callback for cancel event
    
    // private members
    0, 0, 0, 0, 0
};

DEMOWinMenuInfo *MenuPtr;

/*===========================================================================*
 *                   F U N C T I O N    D E F I N I T I O N S
 *===========================================================================*/

/*---------------------------------------------------------------------------*
 * Name        : ax_profile_updatek()
 * Description : refresh callback for AX profile window
 * Arguments   : 
 * Returns     : 
 *---------------------------------------------------------------------------*/

static void ax_profile_update(DEMOWinInfo *window)
{

    BOOL old;

    u32 i;
    
    u32 cpuCycles;
    u32 userCycles;
    u32 axCycles;
    u32 voices;   

    u32 maxCpuCycles =0;
    u32 maxUserCycles=0;
    u32 maxAxCycles  =0;
    u32 maxVoices    =0;   

        old = OSDisableInterrupts();

        i = AXGetProfile();

        if (i)
        {
            // up to 4 audio frames can complete within a 60Hz video frame
            // so spin thru the accumulated audio frame profiles and find the peak values
            while (i)
            {
                i--;

                cpuCycles   = (u32)(ax_profile[i].axFrameEnd      - ax_profile[i].axFrameStart);
                userCycles  = (u32)(ax_profile[i].userCallbackEnd - ax_profile[i].userCallbackStart);
                axCycles    = cpuCycles - userCycles;
                voices      = ax_profile[i].axNumVoices;

                // find peak values over the last i audio frames
                if (cpuCycles > maxCpuCycles)     maxCpuCycles    = cpuCycles;
                if (userCycles > maxUserCycles)   maxUserCycles   = userCycles;
                if (axCycles > maxAxCycles)       maxAxCycles     = axCycles;
                if (voices > maxVoices)           maxVoices       = voices;

            }
            OSRestoreInterrupts(old);

            DEMOWinPrintfXY(window, 0, 2, "Total CPU : %5.2f\%", (f32)OSTicksToNanoseconds(maxCpuCycles) / 50000);
            DEMOWinPrintfXY(window, 0, 4, "User      : %5.2f\%", (f32)OSTicksToNanoseconds(maxUserCycles) / 50000);
            DEMOWinPrintfXY(window, 0, 5, "AX        : %5.2f\%", (f32)OSTicksToNanoseconds(maxAxCycles) / 50000);
            DEMOWinPrintfXY(window, 0, 7, "Voices    : %5d",    maxVoices);
            
        }

        OSRestoreInterrupts(old);

} // end profile_update()

/*---------------------------------------------------------------------------*
 * Name        : MNU_play_click()
 * Description : Callback for menu system, plays 'click' for cursor movement
 * Arguments   : 
 * Returns     : 
 *---------------------------------------------------------------------------*/

static void MNU_play_click(DEMOWinMenuInfo *menu, u32 item)
{

#pragma unused(menu)
#pragma unused(item)

    play_sfx(SFX_MENU);


} // end MNU_play_click()


/*---------------------------------------------------------------------------*
 * Name        : MNU_play_sfx()
 * Description : Play sound effect selected from menu.
 * Arguments   : 
 * Returns     : 
 *---------------------------------------------------------------------------*/

static void MNU_play_sfx(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{

#pragma unused(menu)
#pragma unused(result)

    play_sfx(item);

} // end MNU_play_sfx()



/*---------------------------------------------------------------------------*
 * Name        : MNU_stop_sfx()
 * Description : Stops all voices. Note that voices are freed by the AX user
 *               callback on the next frame.
 * Arguments   : 
 * Returns     : 
 *---------------------------------------------------------------------------*/

static void MNU_stop_sfx(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    u32    i;
    BOOL old;

        old = OSDisableInterrupts();

        for (i=0; i<MAX_DEMO_VOICES; i++)
        {
            if (demo_voice[i].ax_voice)
            {
                AXSetVoiceState(demo_voice[i].ax_voice, AX_PB_STATE_STOP);
            }
        }

        OSRestoreInterrupts(old);

} // end MNU_stop_sfx()

/*---------------------------------------------------------------------------*
 * Name        : MNU_stop_looping()
 * Description : Stops looped sfx only. Note that voices are freed by the 
 *               AX user callback. Note also that SPPrepareEnd() is used, 
 *               so the sound effect will play to the end (beyond the loop)
 *               and THEN get stopped.
 * Arguments   : 
 * Returns     : 
 *---------------------------------------------------------------------------*/

static void MNU_stop_looping(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    u32    i;
    BOOL old;

        old = OSDisableInterrupts();

        for (i=0; i<MAX_DEMO_VOICES; i++)
        {
            if ( (demo_voice[i].ax_voice) && mISLOOPED(demo_voice[i].sp_entry) )
            {
                SPPrepareEnd(demo_voice[i].sp_entry, demo_voice[i].ax_voice);
            }
        }

        OSRestoreInterrupts(old);

} // end MNU_stop_looping()



/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
static void init_demo_voices()
{

    u32 i;

        for (i=0; i<MAX_DEMO_VOICES; i++)
        {
            demo_voice[i].ax_voice = NULL;
            demo_voice[i].sp_entry = NULL;
        }

} // end init_demo_voices()


/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
static DEMO_VOICE *get_demo_voice()
{

    u32 i;

        i=0;
        while (i < MAX_DEMO_VOICES)
        {

            if (NULL == demo_voice[i].ax_voice)
            {
                return(&demo_voice[i]);
            }
            i++;
        }

        return(NULL);

}  // end get_demo_voice()

/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

static void ax_demo_callback(void)
{

    u32 i;

        for (i=0; i<MAX_DEMO_VOICES; i++)
        {
            if (demo_voice[i].ax_voice)
            {
                if ( AX_PB_STATE_STOP == ((demo_voice[i].ax_voice)->pb.state))
                {
                    MIXReleaseChannel(demo_voice[i].ax_voice);
                    AXFreeVoice(demo_voice[i].ax_voice);
                    demo_voice[i].ax_voice = NULL;
                }
            }
        }

} // end ax_demo_callback()

/*---------------------------------------------------------------------------*
 * Name        : ax_drop_voice_callback()
 * Description : Invoked by AX when a voice has been forciby dropped.
 *               Must delete references to the voice from our abstraction layer
 *               and release the associated MIXer channel. 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

static void ax_drop_voice_callback(void *p)
{

    u32 i;

        // search for abstracted voice associated with low-level AX voice.
        for (i=0; i<MAX_DEMO_VOICES; i++)
        {
            // found it! 
            if  ( (AXVPB *)(p) == demo_voice[i].ax_voice)
            {
                // release mixer channel, delete reference to AX voice (and SP entry, just for neatness)
                MIXReleaseChannel(demo_voice[i].ax_voice);
                demo_voice[i].ax_voice = NULL;
                demo_voice[i].sp_entry = NULL;
                break;
            }
        }

        // freak out if the voice doesn't exist in our voice abstraction list
        ASSERTMSG(i != MAX_DEMO_VOICES, "AXVoiceCallback: unknown voice reference!\n");

} // end ax_demo_callback()

/*---------------------------------------------------------------------------*
 * Name        : play_sfx()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

static void play_sfx(u32 sfx)
{

    DEMO_VOICE *v;
    BOOL old;
   

        old = OSDisableInterrupts();

        v = get_demo_voice();
        if (v)
        {

            v->ax_voice = AXAcquireVoice(15, ax_drop_voice_callback, 0);
            if (v->ax_voice)
            {

                v->sp_entry = SPGetSoundEntry(sp_table, sfx);

                SPPrepareSound(v->sp_entry, v->ax_voice, (v->sp_entry)->sampleRate);

                MIXInitChannel(v->ax_voice, 0, 0, -960, -960, 64, 127, 0);
                AXSetVoiceState(v->ax_voice, AX_PB_STATE_RUN);

                OSRestoreInterrupts(old);

            }
            else
            {
                OSRestoreInterrupts(old);
                DEMOWinLogPrintf(DebugWin, "SFX: AX Voice allocation failed.\n");
            }

        }
        else
        {
            OSRestoreInterrupts(old);
            DEMOWinLogPrintf(DebugWin, "(No free voices in abstraction layer)\n");
        }

} // end play_sfx()


/*---------------------------------------------------------------------------*
 * Name        : main()
 * Description : Hold on to your seatbelts!
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/


void main(void)
{

    // initialize system
    DEMOInit(NULL);
    DEMOWinInit();

    // initialize ARAM w/ stack allocator
    ARInit(aramMemArray, MAX_ARAM_BLOCKS);
    ARQInit();

    // initialize AI subsystem
    AIInit(NULL);

    // initialize AX audio system and MIXer application
    AXInit();
    MIXInit();

    // -----------------------------------------------------------
    // Initialize ARAM audio manager (AM)
    // -----------------------------------------------------------

    // get a block from the AR ARAM allocator
    aramUserBase = ARAlloc(AUDIO_BLOCK_SIZE_BYTES);

    // initialize AM with the block
    AMInit(aramUserBase, AUDIO_BLOCK_SIZE_BYTES);

    // retrieve start of zero buffer, as created by AM
    aramZeroBase = AMGetZeroBuffer();



    // -----------------------------------------------------------
    // Load SP data!
    // -----------------------------------------------------------

    // Retrieve sound table 
    sp_table = (SPSoundTable *)AMLoadFile(SPT_FILE, NULL);

    // Load sound effects into ARAM
    aramUserBase = AMPushBuffered(SPD_FILE, (void *)xfer_buffer, XFER_BUFFER_SIZE_BYTES);

    // -----------------------------------------------------------
    // initialize sound table! 
    // -----------------------------------------------------------
    SPInitSoundTable(sp_table, aramUserBase, aramZeroBase);

    // -----------------------------------------------------------
    // Initialize demo voice abstraction layer
    // -----------------------------------------------------------
    init_demo_voices();
    AXRegisterCallback(ax_demo_callback);

    // initialize profiling for AX
    AXInitProfile(ax_profile, NUM_AX_PROFILE_FRAMES); 


    // -----------------------------------------------------------
    // Invoke menu system!
    // -----------------------------------------------------------

   
    MenuPtr    = DEMOWinCreateMenuWindow(&Menu, 20, 100); 
    DebugWin   = DEMOWinCreateWindow((u16)(MenuPtr->handle->x2+10), 20, 620, 440, "Debug", 1024, NULL);
    ProfileWin = DEMOWinCreateWindow((u16)(MenuPtr->handle->x1), (u16)(MenuPtr->handle->y2+10), (u16)(MenuPtr->handle->x2), (u16)(MenuPtr->handle->y2+160), "AX Profile", 0, ax_profile_update);
                                         
    DEMOWinOpenWindow(DebugWin);
    DEMOWinOpenWindow(ProfileWin);

    DEMOWinLogPrintf(DebugWin, "-------------------------------\n");
    DEMOWinLogPrintf(DebugWin, "AX Sound Pipeline Demo!\n");
    DEMOWinLogPrintf(DebugWin, "-------------------------------\n");

    while (1)
    {

        DEMOWinMenu(MenuPtr);

    }
    

} // end main()

