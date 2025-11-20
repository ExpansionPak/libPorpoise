/*---------------------------------------------------------------------------*
  Project:  Dolphin GD demo
  File:     gd-win32-ui.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-win32-ui.c $
    
    1     10/09/01 11:20a Hirose
    Initial check in.
   
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   This code provides common procedure for host applications
 *---------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
extern void       main( void );

static ATOM       MyRegisterClass(HINSTANCE hInstance);

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
extern char*      AppName; // defined in each application

static HINSTANCE  MyInstance;
static HWND       MyWindow;

/*------------------------------------------------------------------*
    Main
 *------------------------------------------------------------------*/
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       iCmdShow)
{
    MyInstance = hInstance;

    MyRegisterClass(hInstance);

    // This window won't be actually displayed.
    MyWindow = CreateWindow(AppName, AppName, WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL, NULL, hInstance, NULL);

    main();

    return 0;
}

/*------------------------------------------------------------------*
    Window callback
 *------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

/*------------------------------------------------------------------*
    Window class registration
 *------------------------------------------------------------------*/
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = AppName;
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

    return RegisterClassEx(&wcex);
}

/*------------------------------------------------------------------*
    Message output
 *------------------------------------------------------------------*/
void GDReport(char* msg, ...)
{
    va_list marker;
    char    strbuffer[256];

    va_start(marker, msg);
    vsprintf(strbuffer, msg, marker);

    MessageBox(MyWindow, strbuffer, AppName, MB_OK);
}

// Stuff needed by ASSERTMSG (found in GD libs)
#ifdef _DEBUG
void OSPanic(char* file, int line, char* msg, ...)
{
    va_list marker;
    char    strbuffer0[256];
    char    strbuffer1[128];

    va_start(marker, msg);
    vsprintf(strbuffer0, msg, marker);

    sprintf(strbuffer1, " in \"%s\" on line %d.", file, line);
    strcat(strbuffer0, strbuffer1);

    MessageBox(MyWindow, strbuffer0, AppName, MB_ICONEXCLAMATION | MB_OK);

    abort();
}
#endif

/*======================================================================*/
