/*---------------------------------------------------------------------------*
 *       N I N T E N D O  C O N F I D E N T I A L  P R O P R I E T A R Y
 *---------------------------------------------------------------------------*
 *
 * Project: Dolphin OS - DTK Demo
 * File:    dtk_stuff.c - all calls to DTK are done in this module
 *
 * Copyright 2001 Nintendo.  All rights reserved.
 *
 * These coded instructions, statements, and computer programs contain
 * proprietary information of Nintendo of America Inc. and/or Nintendo
 * Company Ltd., and are protected by Federal copyright law.  They may
 * not be disclosed to third parties or copied or duplicated in any form,
 * in whole or in part, without the prior written consent of Nintendo.
 *
 * $Log: /Dolphin/build/demos/dtkdemo/src/dtk_stuff.c $
    
    2     5/18/01 5:55p Eugene
    Added new "PREPARE" state for streams. This allows developers to setup
    a track for playback, but does not assert the AI streaming clock. Once
    they receive the "PREPARED" event callback, they can set the state to
    PLAY to start playback at their discretion. This is handy for
    synchronizing streamed dvd audio with in-game cinematics or such.
    
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

#include <dolphin/dtk.h>

/*---------------------------------------------------------------------------*
 * Local definitions/declarations
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Module variables
 *---------------------------------------------------------------------------*/
volatile u32 event_play_start_ctr = 0;
volatile u32 event_play_stop_ctr  = 0;
volatile u32 event_play_queue_ctr = 0;
volatile u32 event_play_pause_ctr = 0;
volatile u32 event_play_end_ctr   = 0;



#define MAX_NUM_TRACKS 32

DTKTrack __Free_Tracks[MAX_NUM_TRACKS];

u32 free_track_index = 0;



/*---------------------------------------------------------------------------*
 * Function prototypes
 *---------------------------------------------------------------------------*/

void      Init_DTK_System(void);

void      Prepare_Track  (void);
void      Play_Track     (void);
void      Pause_Track    (void);
void      Stop_Track     (void);
void      Next_Track     (void);
void      Prev_Track     (void);
void      Set_Repeat     (u32 repeat_mode);
u32       Add_Track      (char *name, DTKCallback cb);
void      Flush_All      (void);
void      Set_Volume     (u8 vol);
u8        Get_Volume     (void);




static void __event_callback(u32 event);
static void __flush_callback(void);


/*===========================================================================*
 *                   F U N C T I O N    D E F I N I T I O N S
 *===========================================================================*/

 
/*---------------------------------------------------------------------------*
 * Name        : Init_DTK_System()
 *
 * Description : 
 *
 * Arguments   : None.
 *
 * Returns     : None.
 *
 *---------------------------------------------------------------------------*/
void Init_DTK_System(void)
{

    // Initialize the AI interface first
    AIInit(NULL);

    // To initialize the Disc Track Player sysem, you must call this first
    DTKInit();
}

/*---------------------------------------------------------------------------*
 * Name        : Add_Track()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
u32 Add_Track(char *name, DTKCallback cb)
{
    u32 result;


        if (free_track_index < MAX_NUM_TRACKS)
        {
            result = DTKQueueTrack( name, &__Free_Tracks[free_track_index],
                                    DTK_EVENT_PLAYBACK_STARTED |
                                    DTK_EVENT_PLAYBACK_STOPPED |
                                    DTK_EVENT_PLAYBACK_PAUSED  |
                                    DTK_EVENT_TRACK_QUEUED     |
                                    DTK_EVENT_TRACK_ENDED      |
                                    DTK_EVENT_TRACK_PREPARED,
                                    cb);

            free_track_index++;

            return(result);
        }
        else
        {
            return(0x4);
        }

} // end Add_Track()

/*---------------------------------------------------------------------------*
 * Name        : Prepare_Track()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Prepare_Track(void)
{

    if (DTKGetCurrentTrack())
    {
        while (DTK_STATE_BUSY == DTKGetState())
        {
            // do nothing
        }
        DTKSetState(DTK_STATE_PREPARE);
    }

}

/*---------------------------------------------------------------------------*
 * Name        : Play_Track()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Play_Track(void)
{

    if (DTKGetCurrentTrack())
    {
        while (DTK_STATE_BUSY == DTKGetState())
        {
            // do nothing
        }
        DTKSetState(DTK_STATE_RUN);
    }

}

/*---------------------------------------------------------------------------*
 * Name        : Pause_Track()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Pause_Track(void)
{
    if (DTKGetCurrentTrack())
    {
        while (DTK_STATE_BUSY == DTKGetState())
        {
            // do nothing
        }
        DTKSetState(DTK_STATE_PAUSE);
    }

}

/*---------------------------------------------------------------------------*
 * Name        : Stop_Track()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Stop_Track(void)
{
    if (DTKGetCurrentTrack())
    {
        while (DTK_STATE_BUSY == DTKGetState())
        {
            // do nothing
        }

        DTKSetState(DTK_STATE_STOP);
    }
}

/*---------------------------------------------------------------------------*
 * Name        : Next_Track()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Next_Track(void)
{
    if (DTKGetCurrentTrack())
    {
        while (DTK_STATE_BUSY == DTKGetState())
        {
            // do nothing
        }
        DTKNextTrack();
    }
}

/*---------------------------------------------------------------------------*
 * Name        : Prev_Track()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Prev_Track(void)
{
    if (DTKGetCurrentTrack())
    {
        while (DTK_STATE_BUSY == DTKGetState())
        {
            // do nothing
        }
        DTKPrevTrack();
    }
}


/*---------------------------------------------------------------------------*
 * Name        : Set_Repeat()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Set_Repeat(u32 repeat_mode)
{
    DTKSetRepeatMode(repeat_mode);
}

/*---------------------------------------------------------------------------*
 * Name        : Get_Volume()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
u8 Get_Volume(void)
{
    // we only care about one channel, because we will set both channels to 
    // the same volume
    return((u8)(DTKGetVolume() & 0xFF));

} // end Get_Volume()

/*---------------------------------------------------------------------------*
 * Name        : Set_Volume()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Set_Volume(u8 vol)
{

    // set both channels to the same volume
    DTKSetVolume(vol, vol);

} // end Get_Volume()


/*---------------------------------------------------------------------------*
 * Name        : Flush_All()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

volatile BOOL flush_flag;

void Flush_All(void)
{
    if (DTKGetCurrentTrack())
    {

        flush_flag = FALSE;

        while (DTK_STATE_BUSY == DTKGetState())
        {
            // do nothing
        }
        DTKFlushTracks(__flush_callback);

        while (FALSE == flush_flag)
        {
            // do nothing
        }
    }


} // end Flush_All



/*---------------------------------------------------------------------------*
 * Name        : __flush_callback()
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

static void __flush_callback(void)
{
    flush_flag = TRUE;
}

