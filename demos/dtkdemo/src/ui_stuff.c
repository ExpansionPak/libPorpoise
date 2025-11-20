/*---------------------------------------------------------------------------*
 *       N I N T E N D O  C O N F I D E N T I A L  P R O P R I E T A R Y
 *---------------------------------------------------------------------------*
 *
 * Project: Dolphin OS - DTK Demo
 * File:    ui_stuff.c - user interface stuff
 *
 * Copyright 2001 Nintendo.  All rights reserved.
 *
 * These coded instructions, statements, and computer programs contain
 * proprietary information of Nintendo of America Inc. and/or Nintendo
 * Company Ltd., and are protected by Federal copyright law.  They may
 * not be disclosed to third parties or copied or duplicated in any form,
 * in whole or in part, without the prior written consent of Nintendo.
 *
 * $Log: /Dolphin/build/demos/dtkdemo/src/ui_stuff.c $
    
    4     5/20/01 11:18p Eugene
    DEMOWIN.H moved from include/dolphin to include/demo
    
    3     5/18/01 5:55p Eugene
    Added new "PREPARE" state for streams. This allows developers to setup
    a track for playback, but does not assert the AI streaming clock. Once
    they receive the "PREPARED" event callback, they can set the state to
    PLAY to start playback at their discretion. This is handy for
    synchronizing streamed dvd audio with in-game cinematics or such.
    
    2     5/10/01 4:08p Eugene
    Added ability to change directories and such. 
    
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

#include <demo/DEMOWin.h>    // for DEMO library windowing system
#include <dolphin/dtk.h>        // for Track Player!


/*---------------------------------------------------------------------------*
 * Function prototypes
 *---------------------------------------------------------------------------*/

// these are public interface functions
void   Init_DTK_System        (void);
void   Init_Player_Windows    (void);
void   Run_Player             (void);



// these are private to this module
static      DEMOWinMenuInfo  *__create_dir_menu (void); // reading directory
static void                   __init_play_list  (void); // first time init of play list
static void                   __clear_play_list (void); // clearing playlist after flush


// these will muck with the user interface stuff, then call the functions in 
// dtk_stuff.c to actually invoke the track player library
static void MNU_prepare       (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_play          (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_pause         (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_stop          (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_next          (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_prev          (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_repeat        (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_add           (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_flush         (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_set_vol       (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_select_file   (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_toggle_event  (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_set_poll      (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_change_dir    (DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_change_dir_up (DEMOWinMenuInfo *menu, u32 item, u32 *result);

// these are all in dtk_stuff.c
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

/*---------------------------------------------------------------------------*
 * Debug and Status window stuff!
 *---------------------------------------------------------------------------*/
DEMOWinInfo *debug_win;     // debug window
DEMOWinInfo *status_win;    // status window

// this is the status-window update callback. This function is in this module.
static void __status_refresh(DEMOWinInfo *handle);
static void __event_callback(u32 event);

static BOOL event_msg_flag = FALSE;

/*---------------------------------------------------------------------------*
 * Directory listing stuff!
 *---------------------------------------------------------------------------*/
#define MAX_DIR_ENTRIES     512
#define MAX_FILENAME_LENGTH 128

DEMOWinMenuItem dir_entry[MAX_DIR_ENTRIES+1];
DEMOWinMenuInfo dir_list;

char dir_entry_name[MAX_DIR_ENTRIES][MAX_FILENAME_LENGTH];

/*---------------------------------------------------------------------------*
 * Playlist stuff!
 *---------------------------------------------------------------------------*/

u16 track_index = 0;

#define MAX_NUM_TRACKS        32
#define MAX_TRACKNAME_LENGTH  24

// these are for the playlist window
char track_names[MAX_NUM_TRACKS+1][MAX_TRACKNAME_LENGTH+1];
DEMOWinListItem track_items[MAX_NUM_TRACKS+1];
DEMOWinListInfo track_list;
DEMOWinListInfo *list_ptr;


/*---------------------------------------------------------------------------*
 * Main Control Menu Stuff!
 *---------------------------------------------------------------------------*/
DEMOWinMenuItem control_menu_items[] = 
{
    { "PLAYLIST",      DEMOWIN_ITM_SEPARATOR,  NULL,         NULL },
    { "  Add Track(s)",DEMOWIN_ITM_NONE,       MNU_add,      NULL },
    { "  Remove All",  DEMOWIN_ITM_NONE,       MNU_flush,    NULL },
    { " ",             DEMOWIN_ITM_SEPARATOR,  NULL,         NULL },
    { "CONTROL",       DEMOWIN_ITM_SEPARATOR,  NULL,         NULL },
    { "  Prepare",     DEMOWIN_ITM_NONE,       MNU_prepare,  NULL },
    { "  Play",        DEMOWIN_ITM_NONE,       MNU_play,     NULL },
    { "  Pause",       DEMOWIN_ITM_NONE,       MNU_pause,    NULL },
    { "  Stop",        DEMOWIN_ITM_NONE,       MNU_stop,     NULL },
    { "  Next",        DEMOWIN_ITM_NONE,       MNU_next,     NULL },
    { "  Previous",    DEMOWIN_ITM_NONE,       MNU_prev,     NULL },
    { "  Repeat Mode", DEMOWIN_ITM_NONE,       MNU_repeat,   NULL },
    { " ",             DEMOWIN_ITM_SEPARATOR,  NULL,         NULL },
    { "VOLUME",        DEMOWIN_ITM_SEPARATOR,  NULL,         NULL },
    { "  Set Volume",  DEMOWIN_ITM_NONE,       MNU_set_vol,  NULL },
    { " ",             DEMOWIN_ITM_SEPARATOR,  NULL,         NULL },
    { "DEBUG",         DEMOWIN_ITM_SEPARATOR,  NULL,         NULL },
    { "  Event Msgs",  DEMOWIN_ITM_NONE,       MNU_toggle_event, NULL },
    { "  Poll Period", DEMOWIN_ITM_NONE,       MNU_set_poll, NULL },
    { "",              DEMOWIN_ITM_TERMINATOR, NULL,         NULL }
};

DEMOWinMenuInfo control_menu = 
{
   "Disc Track Player",     // title
    NULL,                   // window handle
    control_menu_items,     // list of menu items
    19,                     // max num of items to display at a time
    DEMOWIN_MNU_NONE,       // attribute flags?

    // user callbacks for misc menu operations
    NULL, NULL, NULL, NULL, 
    // private menu info members; do not touch
    0, 0, 0, 0, 0
}; 

DEMOWinMenuInfo *control_menu_ptr;


/*===========================================================================*
 *                   F U N C T I O N    D E F I N I T I O N S
 *===========================================================================*/


/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Init_Player_Windows(void)
{
    DEMOWinInfo *p;

        // initialize the play list
        __init_play_list();

        control_menu_ptr = DEMOWinCreateMenuWindow(&control_menu, 20, 20);
        p = control_menu_ptr->handle;

        list_ptr   = DEMOWinCreateListWindow(&track_list, (u16)p->x1, (u16)(p->y2+5));
        status_win = DEMOWinCreateWindow((u16)(p->x2+5), p->y1, 620, (u16)p->y2, "Status", 0, __status_refresh);
        debug_win  = DEMOWinCreateWindow((u16)(p->x2+5), (u16)(p->y2+5), 620, 434, "Debug", 1024, NULL);


        DEMOWinOpenWindow(debug_win);
        DEMOWinOpenWindow(status_win);
        DEMOWinOpenWindow(list_ptr->handle);

        DEMOWinListSetCursor(list_ptr, TRUE);

} // end Init_Player_Windows()

/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void Run_Player(void)
{

    DEMOWinPadInfo pad;



        DEMOWinLogPrintf(debug_win, "-------------------------------\n");
        DEMOWinLogPrintf(debug_win, "DTK - Disc Track Player\n");
        DEMOWinLogPrintf(debug_win, "-------------------------------\n");
        DEMOWinLogPrintf(debug_win, "- Stick Up/Down to move cursor\n");
        DEMOWinLogPrintf(debug_win, "- Button A to select an item\n");
        DEMOWinLogPrintf(debug_win, "- Button B to exit a menu\n");


        while(1)
        {

            DEMOWinMenu(control_menu_ptr);

            DEMOWinLogPrintf(debug_win, "-------------------------------------------\n");
            DEMOWinLogPrintf(debug_win, "\nUse Stick Up/Down to scroll debug buffer.\n");
            DEMOWinLogPrintf(debug_win, "\nHit Start to resume track player.\n");

            DEMOBeforeRender();
            DEMOWinRefresh();
            DEMODoneRender();
            DEMOWinPadRead(&pad);

            while(1)
            {

                DEMOWinPadRead(&pad);

                if (pad.pads[0].stickY < -50)
                {
                    DEMOWinScrollWindow(debug_win, DEMOWIN_SCROLL_DOWN);
                    if (pad.pads[0].triggerLeft > 150)
                    {
                        DEMOWinScrollWindow(debug_win, DEMOWIN_SCROLL_DOWN);
                        DEMOWinScrollWindow(debug_win, DEMOWIN_SCROLL_DOWN);
                    }
                }
                else if (pad.pads[0].stickY > 50)
                {
                    DEMOWinScrollWindow(debug_win, DEMOWIN_SCROLL_UP);
                    if (pad.pads[0].triggerLeft > 150)
                    {
                        DEMOWinScrollWindow(debug_win, DEMOWIN_SCROLL_UP);
                        DEMOWinScrollWindow(debug_win, DEMOWIN_SCROLL_UP);
                    }

                }
                else if (pad.changed_button[0] & PAD_BUTTON_START)
                {
                    DEMOWinScrollWindow(debug_win, DEMOWIN_SCROLL_HOME);
                    break;
                }


                DEMOBeforeRender();
                DEMOWinRefresh();
                DEMODoneRender();


            } // debug buffer scroll loop

        } // forever loop


} // end Init_Player_Windows()



/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void MNU_prepare(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    if (DTK_STATE_STOP != DTKGetState())
    {
        DEMOWinLogPrintf(debug_win, "ERROR: Player must be STOPPED before preparing\nanother track!\n");
    }
    else
    {
        Prepare_Track();
    }
}

/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void MNU_play(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)


    Play_Track();
}

/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void MNU_pause(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)


    Pause_Track();
}


/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void MNU_stop(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    Stop_Track();
}
/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void MNU_next(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    Next_Track();
}
/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void MNU_prev(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    Prev_Track();
}

/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void MNU_repeat(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    u32 repeat_mode;

        repeat_mode = DTKGetRepeatMode();
        repeat_mode = (repeat_mode + 1) % 3;

        Set_Repeat(repeat_mode);
}



/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void MNU_add(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)


    DEMOWinInfo     *p; // parent window
    DEMOWinMenuInfo *m; // directory menu

    u32 val = 0;

        p = menu->handle;

        DEMOWinLogPrintf(debug_win, "\nAdding Tracks...\n");
        DEMOWinLogPrintf(debug_win, "Press A to queue a track or change directory.\nPress B to exit.\n");

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
                DEMOWinLogPrintf(debug_win, "Failed to create directory list!\n");
            }
            
        } while (val == 0xDEADBEEF);

    
}


/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
void MNU_flush(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    Flush_All();
    __clear_play_list();
    
}



/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
static void MNU_set_vol(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    DEMOWinPadInfo pad;
    DEMOWinInfo *handle;

    s32 vol;

    
        handle = DEMOWinCreateWindow(210, 150, 430, 220, "Set Volume", 0, NULL);

        DEMOWinOpenWindow(handle);

        DEMOWinPrintfXY(handle, 0,0, "Stick up/down to adjust");
        DEMOWinPrintfXY(handle, 0,1, "      (B to exit)");

        DEMOWinPadInit(&pad);

        DEMOBeforeRender();
        DEMOWinRefresh();
        DEMODoneRender();

        DEMOWinPadRead(&pad);

        vol = (s32)(DTKGetVolume() & 0xff); 

        while (1)
        {

            if (pad.repeat_button[0] &  (PAD_BUTTON_UP|DEMOWIN_STICK_U))
            {

                vol++;
                if (pad.pads[0].triggerLeft > 152)
                {
                    vol += 4;
                }
                if (vol > 255)
                {
                    vol = 255;
                }

            }
            else if (pad.repeat_button[0] &  (PAD_BUTTON_DOWN|DEMOWIN_STICK_D))
            {
                vol--;
                if (pad.pads[0].triggerLeft > 152)
                {
                    vol -= 4;
                }
                if (vol < 0)
                {
                    vol = 0;
                }

            }
            else if (pad.changed_button[0] & PAD_BUTTON_B) // no repeat!
            {
                break;
            }

            Set_Volume((u8)vol);
            DEMOWinPrintfXY(handle, 0,3, "      Volume: %-3d ",vol);

            DEMOBeforeRender();
            DEMOWinRefresh();
            DEMODoneRender();

            DEMOWinPadRead(&pad);


        }

        DEMOWinCloseWindow(handle);
        DEMOWinDestroyWindow(handle);   
    
}
/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
static void MNU_set_poll(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    DEMOWinPadInfo pad;
    DEMOWinInfo *handle;

    s32 period;

    
        handle = DEMOWinCreateWindow(210, 150, 430, 220, "Set Poll Period", 0, NULL);

        DEMOWinOpenWindow(handle);

        DEMOWinPrintfXY(handle, 0,0, "Stick up/down to adjust");
        DEMOWinPrintfXY(handle, 0,1, "      B to exit");
        DEMOWinPrintfXY(handle, 0,5, "     (in samples)");

        DEMOWinPadInit(&pad);

        DEMOBeforeRender();
        DEMOWinRefresh();
        DEMODoneRender();

        DEMOWinPadRead(&pad);

        period = (s32)DTKGetInterruptFrequency();

        while (1)
        {

            if (pad.repeat_button[0] &  (PAD_BUTTON_UP|DEMOWIN_STICK_U))
            {

                period += 10;
                if (pad.pads[0].triggerLeft > 152)
                {
                    period += 990;
                }
                if (period > 96000)
                {
                    period = 96000;
                }

            }
            else if (pad.repeat_button[0] &  (PAD_BUTTON_DOWN|DEMOWIN_STICK_D))
            {
                period -= 10;
                if (pad.pads[0].triggerLeft > 152)
                {
                    period -=990;
                }
                if (period < 1000)
                {
                    period = 1000;
                }

            }
            else if (pad.changed_button[0] & PAD_BUTTON_B) // no repeat!
            {
                DTKSetInterruptFrequency((u32)period);
                break;
            }

            DEMOWinPrintfXY(handle, 0,3, "      Period: %-3d ",period);

            DEMOBeforeRender();
            DEMOWinRefresh();
            DEMODoneRender();

            DEMOWinPadRead(&pad);


        }

        DEMOWinCloseWindow(handle);
        DEMOWinDestroyWindow(handle);   
    
}

/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/


static char toupper(char c);

static char toupper(char c)
{

    if ((c >= 'a') && (c <= 'z'))
        return((char)(c-32));
    else
        return(c);
}


static BOOL compare_strings(char *s1, char *s2);

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
    u32 val;
    u32 i;

        if (track_index >= MAX_NUM_TRACKS)
        {
            DEMOWinLogPrintf(debug_win, "Track list is full! (%d tracks)\n", track_index);
        }
        else
        {

            i = strlen(menu->items[item].name);
            while ((i >=0) && (menu->items[item].name[i] != '.'))
            {
                i--;
            }

            if (compare_strings(&menu->items[item].name[i], ".adp")) 
            {

                val = Add_Track(menu->items[item].name, __event_callback);
                if (DTK_QUEUE_SUCCESS == val)
                {

                    menu->items[item].flags |= DEMOWIN_ITM_CHK_STATE;
                    strncpy(track_list.items[track_index].name,  menu->items[item].name, MAX_TRACKNAME_LENGTH-1);
                    track_list.items[track_index].name[MAX_TRACKNAME_LENGTH] = 0;
                    track_index++;
                    track_list.curr_pos = track_index;

                }
                else
                {
                    DEMOWinLogPrintf(debug_win, "Failed to queue '%s'\n", menu->items[item].name);
                }
            }
            else
            {
                DEMOWinLogPrintf(debug_win, "\n--> File '%s' is not an .ADP file!\n", menu->items[item].name);
            }

        }

        *result = track_index;
        
} // end __select_file


/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

static void __event_callback(u32 event)
{
    if (event_msg_flag)
    {


        switch(event)
        {
            case DTK_EVENT_PLAYBACK_STARTED:
                DEMOWinLogPrintf(debug_win, "EVENT: Track has started.\n");
                break;
            case DTK_EVENT_PLAYBACK_STOPPED:
                DEMOWinLogPrintf(debug_win, "EVENT: Track has stopped.\n");
                break;
            case DTK_EVENT_PLAYBACK_PAUSED:
                DEMOWinLogPrintf(debug_win, "EVENT: Track has been paused.\n");
                break;
            case DTK_EVENT_TRACK_QUEUED:
                DEMOWinLogPrintf(debug_win, "EVENT: Track has been queued.\n");
                break;
            case DTK_EVENT_TRACK_ENDED:     
                DEMOWinLogPrintf(debug_win, "EVENT: Track has ended.\n");
                break;
            case DTK_EVENT_TRACK_PREPARED:
                DEMOWinLogPrintf(debug_win, "EVENT: Track prepared! Hit PLAY to run.\n");
                break;

            default:
                DEMOWinLogPrintf(debug_win, "!?!? Unknown event: %d", event);
                break;
        }
    }

}
/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

static void MNU_toggle_event(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    event_msg_flag ^=1;
} 

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

    if (track_index)
    {
        // if tracks exist then flush 'em
        DEMOWinLogPrintf(debug_win, "\nClearing playlist...\n(Can't play tracks from different directories.\nYes, lame, sorry.)\n");
        Flush_All();
        __clear_play_list();
    }


    menu->items[item].flags |= DEMOWIN_ITM_EOF;

    DEMOWinLogPrintf(debug_win, "\nChanging to parent directory...\n");

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


        DEMOWinLogPrintf(debug_win, "\nChanging directory to: '%s'...\n", menu->items[item].name);

        if (track_index)
        {
            // if tracks exist then flush 'em
            DEMOWinLogPrintf(debug_win, "\nClearing playlist...\n(Can't play tracks from different directories.\nYes, lame, sorry.)\n");
            Flush_All();
            __clear_play_list();
        }



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
            DEMOWinLogPrintf(debug_win, "\nUnable to change directory...??\n");
            menu->items[item].flags &= ~DEMOWIN_ITM_EOF;
        }

        DVDCloseDir(&dir);

        *result = 0xDEADBEEF;

} // MNU_change_dir


/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

char *__state_names[] = 
{
    "Stopped",
    "Running",
    "Paused ",
    "BUSY   "
};

char *__repeat_names[] = 
{
    "None ",
    "All  ",
    "One  "
};

DVDCommandBlock block;

static void __status_refresh(DEMOWinInfo *handle)
{
#pragma unused(handle)

    DTKTrack *ptr;
    s32       play_addr;

        ptr       = DTKGetCurrentTrack();
        play_addr = DVDGetStreamPlayAddr(&block);

        DEMOWinClearRow(handle, 0);
        DEMOWinPrintfXY(handle, 0,  0, "Current TracK         : '%s'", (ptr == NULL) ? "<none>" : ptr->fileName);

        DEMOWinPrintfXY(handle, 0,  1, "Player State          : %s", __state_names[DTKGetState()]);
        DEMOWinPrintfXY(handle, 0,  2, "Repeat Mode           : %s", __repeat_names[DTKGetRepeatMode()]);

        DEMOWinPrintfXY(handle, 0,  4, "Volume                : %-3d", DTKGetVolume() & 0xff);
        DEMOWinPrintfXY(handle, 0,  5, "Poll Period (samples) : %-6d", DTKGetInterruptFrequency());

        DEMOWinPrintfXY(handle, 0,  7, "AIS Sample Counter    : %010d", AIGetStreamSampleCount());
        DEMOWinPrintfXY(handle, 0,  8, "Current Play Address  : 0x%08X\n", play_addr);

        DEMOWinPrintfXY(handle, 0, 10, "Event Messages        : %s", (event_msg_flag) ? "ON " : "OFF");


} // end __status_refresh()

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
                dir_entry[index].flags    = DEMOWIN_ITM_NONE;
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
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
static void __init_play_list(void)
{

    u16 i;

        track_index = 0;

        // clear items in track list window
        for (i=0; i<MAX_NUM_TRACKS; i++)
        {                          
            sprintf(track_names[i], "<empty>          ", i);
            track_items[i].name = &track_names[i][0];
            track_items[i].flags = DEMOWIN_ITM_NONE;
        }
        track_items[i].flags = DEMOWIN_ITM_TERMINATOR; // last item is a list terminator

        // 
        track_list.items = track_items;
        track_list.title = "Play List";

        track_list.max_display_items = 27;
        
        track_list.flags             = 0;
        track_list.num_display_items = 0;
        track_list.num_items         = 0;
        track_list.max_str_len       = 0;
        track_list.curr_pos          = 0;
        track_list.display_pos       = 0;
        track_list.cursor_state      = 0;

        
} // end __init_play_list

/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/
static void __clear_play_list(void)
{

    u16 i;

        track_index = 0;

        // clear items in track list window
        for (i=0; i<MAX_NUM_TRACKS; i++)
        {                          
            sprintf(track_names[i], "<empty>          ", i);
            track_items[i].name = &track_names[i][0];
            track_items[i].flags = DEMOWIN_ITM_NONE;
        }
        track_items[i].flags = DEMOWIN_ITM_TERMINATOR; // last item is a list terminator
       
} // end __clear_play_list


