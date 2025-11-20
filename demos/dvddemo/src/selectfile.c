/*---------------------------------------------------------------------------*
  Project:  File selection window system for Dolphin DVD error handling demo
  File:     selectfile.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/dvddemo/src/selectfile.c $
    
    1     6/15/01 9:03p Hashida
    Initial revision.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <dolphin.h>
#include <demo/DEMOWin.h>
#include <string.h>
#include <stdio.h>
#include "selectfile.h"

/*---------------------------------------------------------------------------*
 * Directory listing stuff!
 *---------------------------------------------------------------------------*/
#define MAX_DIR_ENTRIES     512
#define MAX_FILENAME_LENGTH 128

static DEMOWinMenuItem dir_entry[MAX_DIR_ENTRIES+1];
static DEMOWinMenuInfo dir_list;

static char dir_entry_name[MAX_DIR_ENTRIES][MAX_FILENAME_LENGTH];

static void MNU_select_file(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_change_dir_up(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static void MNU_change_dir(DEMOWinMenuInfo *menu, u32 item, u32 *result);
static DEMOWinMenuInfo *__create_dir_menu(void);
static char toupper(char c);
static BOOL compare_strings(char *s1, char *s2);

static DEMOWinInfo          *Debug_win;


void InitSelectFile(DEMOWinInfo *debug_win)
{
    Debug_win = debug_win;
}


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
    DEMOWinLogPrintf(Debug_win, " '%s' was selected\n", menu->items[item].name);

    *result = (u32)menu->items[item].name;

} // end __select_file


char* SelectFile(DEMOWinInfo *handle)
{
    DEMOWinMenuInfo *m; // directory menu
    u32             val = 0;

    DEMOWinSetWindowColor(handle, DEMOWIN_COLOR_CAPTION,
                          50, 50, 50, 255);
    do 
    {
        m = __create_dir_menu();
        if (m)
        {      
            DEMOWinCreateMenuWindow(m, (u16)(handle->x2-10), (u16)(handle->y1+24));
            val = DEMOWinMenu(m);
            DEMOWinDestroyMenuWindow(m);
        }
        else
        {
            DEMOWinLogPrintf(Debug_win, "Failed to create directory list!\n");
        }
            
    } while (!val);

    DEMOWinSetWindowColor(handle, DEMOWIN_COLOR_RESET, 0, 0, 0, 0);
    
    return (char*)val;
}

static void MNU_change_dir_up(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(menu)
#pragma unused(item)
#pragma unused(result)

    menu->items[item].flags |= DEMOWIN_ITM_EOF;

    DEMOWinLogPrintf(Debug_win, "\nChanging to parent directory...\n");

    DVDChangeDir("..");

    *result = 0;

} // MNU_change_dir_up

/*---------------------------------------------------------------------------*
 * Name        : 
 * Description : 
 * Arguments   : None.
 * Returns     : None.
 *---------------------------------------------------------------------------*/

static void MNU_change_dir(DEMOWinMenuInfo *menu, u32 item, u32 *result)
{
#pragma unused(result)

    DVDDir      dir;
    DVDDirEntry dirent;

    u32 i;


    DEMOWinLogPrintf(Debug_win, "\nChanging directory to: '%s'...\n", menu->items[item].name);

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
        DEMOWinLogPrintf(Debug_win, "\nUnable to change directory...??\n");
        menu->items[item].flags &= ~DEMOWIN_ITM_EOF;
    }

    DVDCloseDir(&dir);
    
    *result = 0;
        
} // MNU_change_dir

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

