/*---------------------------------------------------------------------------*
  Project:  Dolphin DVD error handling demo
  File:     errorhandling.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/dvddemo/src/errorhandling.c $
    
    2     6/16/01 5:50a Hashida
    Changed so that error status and play address are shown only when it's
    succeeded.
    
    1     6/15/01 9:03p Hashida
    Initial revision.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * This demo illustrates how to handle errors. See __refresh_status() for
 * the actual error handling code!
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <dolphin.h>
#include <demo/DEMOWin.h>
#include <string.h>
#include <stdio.h>
#include "selectfile.h"

/*---------------------------------------------------------------------------*
 * DVD definitions
 *---------------------------------------------------------------------------*/
#define DVD_BUFFER_SIZE             0xf00000
static void*                        Buffer;

static DVDFileInfo                  FileInfo;
static volatile BOOL                FileInfoIsInUse = FALSE;


/*---------------------------------------------------------------------------*
 * Misc
 *---------------------------------------------------------------------------*/
#define MIN(x, y)                   ((x) < (y)? (x):(y))


/*---------------------------------------------------------------------------*
 * Data to pass from command issuing routines to status printing routine.
 *---------------------------------------------------------------------------*/
enum{
    COMMAND_READ,
    COMMAND_SEEK,
    COMMAND_PREPARE_STREAM,
    COMMAND_CANCEL_STREAM,
    COMMAND_STOP_STREAM_AT_END,
    COMMAND_GET_PLAY_ADDR,
    COMMAND_GET_ERROR_STATUS
};

typedef struct
{
    u32             command;
    s32             readLength;
} CommandBlockData;

static CommandBlockData         Data;


/*---------------------------------------------------------------------------*
 * Messages for errors
 *---------------------------------------------------------------------------*/
typedef struct
{
    char*           line[6];
} Message;

// Error messages. XXX Caution: Subject to change.
Message ErrorMessages[] = {
    {"Please close the NINTENDO GAMECUBE cover.", "", "", "", "", ""},
    {"Please insert the disc for xxx.", "", "", "", "", ""},
    {"This disc is not for xxx.", "Please insert the disc for xxx.", "", "", "", ""},
    {"The disc could not be read.", "", "Please read the NINTENDO GAMECUBE",
     "Instruction Booklet for more information", "", ""},
    {"An error has occurred.", "", "Turn the power OFF and check", "the NINTENDO GAMECUBE",
     "Instruction Booklet for", "further instructions."}
};

enum{
    MESSAGE_COVER_OPEN = 0,
    MESSAGE_NO_DISK,
    MESSAGE_WRONG_DISK,
    MESSAGE_RETRY_ERROR,
    MESSAGE_FATAL_ERROR
};


/*---------------------------------------------------------------------------*
 * Prototypes
 *---------------------------------------------------------------------------*/
static void InitWindows(void);

static void __status_refresh(DEMOWinInfo *handle);

static void Run_Demo(void);

static void MNU_read(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_seek(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_cancel(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_prepare_stream(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_get_play_addr(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_get_error_status(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_cancel_stream(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_stop_stream_at_end(DEMOWinMenuInfo *menu, u32 item, u32 *result);


/*---------------------------------------------------------------------------*
 * Main Control Menu Stuff!
 *---------------------------------------------------------------------------*/
DEMOWinMenuItem control_menu_items[] = 
{
    { "ISSUE COMMAND",              DEMOWIN_ITM_SEPARATOR,  NULL,                   NULL },
    { "  DVDRead",                  DEMOWIN_ITM_NONE,       MNU_read,               NULL },
    { "  DVDSeek",                  DEMOWIN_ITM_NONE,       MNU_seek,               NULL },
    { "  DVDCancel",                DEMOWIN_ITM_NONE,       MNU_cancel,             NULL },
    { "  DVDPrepareStream",         DEMOWIN_ITM_NONE,       MNU_prepare_stream,     NULL },
    { "  DVDCancelStream",          DEMOWIN_ITM_NONE,       MNU_cancel_stream,      NULL },
    { "  DVDStopStreamAtEnd",       DEMOWIN_ITM_NONE,       MNU_stop_stream_at_end, NULL },
    { "  DVDGetStreamPlayAddr",     DEMOWIN_ITM_NONE,       MNU_get_play_addr,      NULL },
    { "  DVDGetStreamErrorStatus",  DEMOWIN_ITM_NONE,       MNU_get_error_status,   NULL },
    { "",                           DEMOWIN_ITM_TERMINATOR, NULL,                   NULL }
};

DEMOWinMenuInfo control_menu = 
{
   "DVD Error Handling Demo",     // title
    NULL,                   // window handle
    control_menu_items,     // list of menu items
    50,                     // max num of items to display at a time
    DEMOWIN_MNU_NONE,       // attribute flags?

    // user callbacks for misc menu operations
    NULL, NULL, NULL, NULL, 
    // private menu info members; do not touch
    0, 0, 0, 0, 0
}; 
DEMOWinMenuInfo *control_menu_ptr;


/*---------------------------------------------------------------------------*
 * Debug and Status window stuff!
 *---------------------------------------------------------------------------*/
DEMOWinInfo *debug_win;     // debug window
DEMOWinInfo *status_win;    // status window
DEMOWinInfo *errmsg_win;    // error message window


 /*---------------------------------------------------------------------------*
    Name:               InitWindows
    
    Description:        Initialize windows
                                
    Arguments:          none
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void InitWindows(void)
{
    DEMOWinInfo *p;

    control_menu_ptr = DEMOWinCreateMenuWindow(&control_menu, 20, 20);
    p = control_menu_ptr->handle;

    // Intialize a window for showing status of DVD commands
    // By passing "__status_refresh" as the last argument, the window system 
    // calls "__status_refresh" once every frame. We use this function to
    // handle errors in this demo.
    status_win = DEMOWinCreateWindow((u16)(p->x2+5), p->y1, 620, (u16)(p->y1+100),
                                     "Status", 0, __status_refresh);

    // Initialize a window for debug output
    debug_win  = DEMOWinCreateWindow((u16)(p->x2+5), (u16)(p->y1+105), 620, 434,
                                     "Debug", 1024, NULL);

    // Initialize a window for showing error message
    errmsg_win = DEMOWinCreateWindow(120, 150, 520, 250,
                                     "Error message", 0, NULL);

    // Open status and debug windows. We don't open error message window until
    // we hit an error.
    DEMOWinOpenWindow(debug_win);
    DEMOWinOpenWindow(status_win);

    // Tell the pointer to the debug window to the "file select" system 
    InitSelectFile(debug_win);
}

 /*---------------------------------------------------------------------------*
    Name:               __status_refresh
    
    Description:        This is the error handling part. This function is called
                        once every frame.
                                
    Arguments:          handle              Window handle for the status window
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void __status_refresh(DEMOWinInfo *handle)
{
    static u32          counter;
    s32                 message;
    u32                 i;  
    CommandBlockData    *data;
    

    // Clear status window (we only use the first two rows)
    DEMOWinClearRow(handle, 0);
    DEMOWinClearRow(handle, 1);
    
    // We use "user data" as the information of the command that was issued
    // last.
    data = DVDGetUserData((DVDCommandBlock*)&FileInfo);
    
    if (data)
    {
        // If it's a read command, show the progress
        if (data->command == COMMAND_READ)
        {
            DEMOWinPrintfXY(handle, 0,  0, "Now loading....%3d%%(%d/%d)",
                            100*DVDGetTransferredSize(&FileInfo)/data->readLength,
                            DVDGetTransferredSize(&FileInfo), data->readLength);
        }
    }
    
    message = -1;

    // This part is for debugging purpose. Show the drive's status on status
    // window.
    switch(DVDGetDriveStatus())
    {
      case DVD_STATE_END:
        FileInfoIsInUse = FALSE;
        DEMOWinPrintfXY(handle, 0,  1, "Command finished");
        break;
      case DVD_STATE_FATAL_ERROR:
        DEMOWinPrintfXY(handle, 0,  1, "Fatal error occurred");
        message = MESSAGE_FATAL_ERROR;
        break;
      case DVD_STATE_BUSY:
        break;
      case DVD_STATE_NO_DISK:
        DEMOWinPrintfXY(handle, 0,  1, "No disk");
        message = MESSAGE_NO_DISK;
        break;
      case DVD_STATE_COVER_OPEN:
        DEMOWinPrintfXY(handle, 0,  1, "Cover open");
        message = MESSAGE_COVER_OPEN;
        break;
      case DVD_STATE_WRONG_DISK:
        DEMOWinPrintfXY(handle, 0,  1, "Wrong disk");
        message = MESSAGE_WRONG_DISK;
        break;
      case DVD_STATE_RETRY:
        DEMOWinPrintfXY(handle, 0,  1, "Please retry");
        message = MESSAGE_RETRY_ERROR;
        break;
    }

    // If necessary, launch error message window to show the error message to
    // the user.
    if (message >= 0)
    {
        DEMOWinOpenWindow(errmsg_win);
        counter = (counter+1) % 60;
        for (i = 0; i < 6; i++)
        {
            if (counter < 45)
            {
                DEMOWinPrintfXY(errmsg_win, 0, (u16)i, ErrorMessages[message].line[i]);
            }
            else
            {
                DEMOWinClearRow(errmsg_win, (u16)i);
            }
        }
    }
    else
    {
        DEMOWinCloseWindow(errmsg_win);
    }
    
} // end __status_refresh()


 /*---------------------------------------------------------------------------*
    Name:               MNU_cancel
    
    Description:        Issue DVDCancel. This function is called when DVDCancel
                        is selected in the control window.
                                
    Arguments:          menu, item, result      unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MNU_cancel(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu, item, result)
    DVDCancelAsync((DVDCommandBlock*)&FileInfo, NULL);
}


 /*---------------------------------------------------------------------------*
    Name:               MNU_read
    
    Description:        Issue DVDRead, after letting the user to select a file
                        to read.
                                
    Arguments:          menu                Winmenuinfo for control window
                        item, result        unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MNU_read(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(item)
#pragma unused(result)
    s32             length;
    char*           file;

    // Avoid using the same FileInfo/CommandBlock at the same time. 
    if (FileInfoIsInUse)
    {
        return;
    }

    DEMOWinLogPrintf(debug_win, "\nIssuing a read...\n");
    DEMOWinLogPrintf(debug_win, "Press A to choose a file to read or change directory.\nPress B to exit.\n");

    // Launch a window to let the user to select a file. 
    // The return value is the pointer to the selected file.
    file = SelectFile(menu->handle);

    DVDOpen(file, &FileInfo);
    // If the size of the file is larger than the buffer size, we only read
    // the first DVD_BUFFER_SIZE bytes.
    length = (s32)OSRoundUp32B(MIN(DVDGetLength(&FileInfo), DVD_BUFFER_SIZE));

    // Pass information to __status_refresh()
    Data.command = COMMAND_READ;
    Data.readLength = length;
    DVDSetUserData((DVDCommandBlock*)&FileInfo, (void*)&Data);

    FileInfoIsInUse = TRUE;
    DVDReadAsync(&FileInfo, Buffer, length, 0, NULL);

}

 /*---------------------------------------------------------------------------*
    Name:               MNU_seek
    
    Description:        Issue DVDSeek, after letting the user to select a file
                        to seek to.
                                
    Arguments:          menu                Winmenuinfo for control window
                        item, result        unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MNU_seek(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(item)
#pragma unused(result)
    char*           file;

    // Avoid using the same FileInfo/CommandBlock at the same time. 
    if (FileInfoIsInUse)
    {
        return;
    }

    DEMOWinLogPrintf(debug_win, "\nIssuing a seek...\n");
    DEMOWinLogPrintf(debug_win, "Press A to choose a file to seek to or change directory.\nPress B to exit.\n");

    // Launch a window to let the user to select a file. 
    // The return value is the pointer to the selected file.
    file = SelectFile(menu->handle);

    DVDOpen(file, &FileInfo);

    // Pass information to __status_refresh()
    Data.command = COMMAND_SEEK;
    DVDSetUserData((DVDCommandBlock*)&FileInfo, (void*)&Data);

    FileInfoIsInUse = TRUE;
    DVDSeekAsync(&FileInfo, 0, NULL);

}

 /*---------------------------------------------------------------------------*
    Name:               StreamCallback
    
    Description:        Callback for DVDPrepareStream. This function starts
                        audio streaming
                                
    Arguments:          result, fileinfo        unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void StreamCallback(s32 result, DVDFileInfo* fileInfo)
{
#pragma unused(result, fileInfo)

    // Set volume to maximum.
    AISetStreamVolLeft(255);
    AISetStreamVolRight(255);

    AISetStreamSampleRate(AI_SAMPLERATE_48KHZ);

    // Start audio streaming
    AISetStreamPlayState(AI_STREAM_START);
}

 /*---------------------------------------------------------------------------*
    Name:               MNU_prepare_stream
    
    Description:        Issue DVDPrepareStream, after letting the user to select
                        a track to play. Note that the track actually starts
                        being played in StreamCallback()
                                
    Arguments:          menu                Winmenuinfo for control window
                        item, result        unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MNU_prepare_stream(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(item)
#pragma unused(result)
    char*           file;

    // Avoid using the same FileInfo/CommandBlock at the same time. 
    if (FileInfoIsInUse)
    {
        return;
    }

    DEMOWinLogPrintf(debug_win, "Press A to choose a track or change directory.\nPress B to exit.\n");

    // Launch a window to let the user to select a file. 
    // The return value is the pointer to the selected file.
    file = SelectFile(menu->handle);
    
    DVDOpen(file, &FileInfo);

    // Pass information to __status_refresh()
    Data.command = COMMAND_PREPARE_STREAM;
    DVDSetUserData((DVDCommandBlock*)&FileInfo, (void*)&Data);

    FileInfoIsInUse = TRUE;
    DVDPrepareStreamAsync(&FileInfo, 0, 0, StreamCallback);
}

 /*---------------------------------------------------------------------------*
    Name:               MNU_cancel_stream
    
    Description:        Turn the volume off and issue DVDCancelStream.
                                
    Arguments:          menu, item, result  unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MNU_cancel_stream(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused (menu, item, result)
    // Avoid using the same FileInfo/CommandBlock at the same time. 
    if (FileInfoIsInUse)
    {
        return;
    }

    AISetStreamVolLeft(0);
    AISetStreamVolRight(0);

    // Pass information to __status_refresh()
    Data.command = COMMAND_CANCEL_STREAM;
    DVDSetUserData((DVDCommandBlock*)&FileInfo, (void*)&Data);

    FileInfoIsInUse = TRUE;
    DVDCancelStreamAsync((DVDCommandBlock*)&FileInfo, NULL);
}

 /*---------------------------------------------------------------------------*
    Name:               MNU_stop_stream_at_end
    
    Description:        Issue DVDStopStreamAtEnd
                                
    Arguments:          menu, item, result  unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MNU_stop_stream_at_end(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused (menu, item, result)
    // Avoid using the same FileInfo/CommandBlock at the same time. 
    if (FileInfoIsInUse)
    {
        return;
    }

    // Pass information to __status_refresh()
    Data.command = COMMAND_STOP_STREAM_AT_END;
    DVDSetUserData((DVDCommandBlock*)&FileInfo, (void*)&Data);

    FileInfoIsInUse = TRUE;
    DVDStopStreamAtEndAsync((DVDCommandBlock*)&FileInfo, NULL);
}

 /*---------------------------------------------------------------------------*
    Name:               PlayAddressCallback
    
    Description:        Callback for DVDGetStreamPlayAddr. Prints the result
                        of the function.
                                
    Arguments:          result              result of DVDGetStreamPlayAddr
                        block               unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void PlayAddressCallback(s32 result, DVDCommandBlock* block)
{
#pragma unused(block)
    // Note that result shows the play address only when it's a positive number.
    // If it's negative, it can be either canceled or fatal error.
    if (result >= 0)
        DEMOWinLogPrintf(debug_win, "Play Address is 0x%08x\n", result);
}

 /*---------------------------------------------------------------------------*
    Name:               MNU_get_play_addr
    
    Description:        Issue DVDGetStreamPlayAddr
                                
    Arguments:          menu, item, result      unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MNU_get_play_addr(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused (menu, item, result)
    // Avoid using the same FileInfo/CommandBlock at the same time. 
    if (FileInfoIsInUse)
    {
        return;
    }

    // Pass information to __status_refresh()
    Data.command = COMMAND_GET_PLAY_ADDR;
    DVDSetUserData((DVDCommandBlock*)&FileInfo, (void*)&Data);

    FileInfoIsInUse = TRUE;
    DVDGetStreamPlayAddrAsync((DVDCommandBlock*)&FileInfo, PlayAddressCallback);
}

 /*---------------------------------------------------------------------------*
    Name:               ErrorStatusCallback
    
    Description:        Callback for DVDGetStreamErrorStatus. Prints the result
                        of the function.
                                
    Arguments:          result              result of DVDGetStreamErrorStatus
                        block               unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void ErrorStatusCallback(s32 result, DVDCommandBlock* block)
{
#pragma unused(block)
    // Note that result shows the error status only when it's a positive number.
    // If it's negative, it can be either canceled or fatal error.
    if (result >= 0)
        DEMOWinLogPrintf(debug_win, "Error status is 0x%08x\n", result);
}

 /*---------------------------------------------------------------------------*
    Name:               MNU_get_error_status
    
    Description:        Issue DVDGetStreamErrorStatus
                                
    Arguments:          menu, item, result          unused
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MNU_get_error_status(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused (menu, item, result)
    // Avoid using the same FileInfo/CommandBlock at the same time. 
    if (FileInfoIsInUse)
    {
        return;
    }

    // Pass information to __status_refresh()
    Data.command = COMMAND_GET_ERROR_STATUS;
    DVDSetUserData((DVDCommandBlock*)&FileInfo, (void*)&Data);

    FileInfoIsInUse = TRUE;
    DVDGetStreamErrorStatusAsync((DVDCommandBlock*)&FileInfo, ErrorStatusCallback);
}


 /*---------------------------------------------------------------------------*
    Name:               Run_Demo
    
    Description:        Main loop of the demo.
                                
    Arguments:          none
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void Run_Demo(void)
{
    DEMOWinPadInfo pad;

    DEMOWinLogPrintf(debug_win, "-------------------------------\n");
    DEMOWinLogPrintf(debug_win, "DVD error handling sample\n");
    DEMOWinLogPrintf(debug_win, "-------------------------------\n");
    DEMOWinLogPrintf(debug_win, "- Stick Up/Down to move cursor\n");
    DEMOWinLogPrintf(debug_win, "- Button A to select an item\n");
    DEMOWinLogPrintf(debug_win, "- Button B to exit a menu\n");
    DEMOWinLogPrintf(debug_win, "While you are executing a command, open cover, make no disk,\n");
    DEMOWinLogPrintf(debug_win, "put other disks to see how to handle errors\n");
    
    while(1)
    {
        // Launch control window. This funcion returns when B is pressed.
        DEMOWinMenu(control_menu_ptr);

        DEMOWinLogPrintf(debug_win, "-------------------------------------------\n");
        DEMOWinLogPrintf(debug_win, "\nUse Stick Up/Down to scroll debug buffer.\n");
        DEMOWinLogPrintf(debug_win, "\nHit Start to resume the demo.\n");

        DEMOBeforeRender();
        DEMOWinRefresh();
        DEMODoneRender();
        DEMOWinPadRead(&pad);

        // Let the user to scroll the debug window. Press start button to 
        // go to the top of the outer loop and open the control window again.
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
                // get out of the inner loop
                break;
            }

            DEMOBeforeRender();
            DEMOWinRefresh();
            DEMODoneRender();


        } // debug buffer scroll loop

    } // forever loop


} // end Init_Player_Windows()




void main(void)
{
    void*           arenaLo;

    arenaLo = OSGetArenaLo();
    Buffer = arenaLo;
    OSSetArenaLo((void*)((u32)arenaLo + DVD_BUFFER_SIZE));

    // clear user data
    DVDSetUserData((DVDCommandBlock*)&FileInfo, (void*)NULL);

    DEMOInit(NULL);
    AIInit(NULL);

    DEMOWinInit();

    InitWindows();

    Run_Demo();

} // end main
