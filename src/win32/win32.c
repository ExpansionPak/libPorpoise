//============================================================================
// PC Dolphin Emulator Version 2.8
// Nintendo of America, Inc.
// Product Support Group
// Dante Treglia II
// danttr01@noa.nintendo.com
// April 12, 2000
// Copyright (c) 2000 Nintendo of America Inc.
//============================================================================

#include <win32/resource.h>
#include <win32/win32.h>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <commdlg.h>

#include "dolphin/pad.h"

// Create Output Window
#define OUTPUT_WINDOW


//----------------------------------------------------------------------------
// Global Variables
HINSTANCE   global_hInstance;
HWND        global_hWnd;
HDC         global_hDC;
HGLRC       global_hRC;
WinKey winKey;
HANDLE outputWinHandle = NULL;
HANDLE inputWinHandle = NULL;
unsigned int retraceCount;
char initialPath[256];
char installPath[256];

//----------------------------------------------------------------------------
// Foward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                WinDestroy(void);
void                main(void);

//----------------------------------------------------------------------------
// OpenGL MultiTexture Extsions
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB;
// For Non MultiTexture Cards
int multiTexOn;
void APIENTRY glActiveTextureHACK (GLenum target) {}
void APIENTRY glMultiTexCoord2fHACK (GLenum target, GLfloat s, GLfloat t) { glTexCoord2f(s, t);}
void APIENTRY glMultiTexCoord4fHACK (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) { glTexCoord4f(s, t, r, q);}

//----------------------------------------------------------------------------
char outStr[] = 
"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\
Win32 Dolphin Emulator\n\
Nintendo of America, Inc.\n\
Nintendo Technology Development\n\
support@noa.com \n\
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n";
char glWarning[] =
"************************************************************\n\
*** The installed OpenGL drivers do not support The ARB  ***\n\
*** Multi-Texture Extensions. You will not be able to    ***\n\
*** run demos that support the Multi-Texture features    ***\n\
*** of this emulator. Please contact support@noa.com     ***\n\
*** for more information.                                ***\n\
************************************************************\n\n";

//----------------------------------------------------------------------------
// Win32 Main. 
// Note: Control is released to main()
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
LPSTR lpCmdLine, int nCmdShow) {
	unsigned long size = 256;
	unsigned long type;
	HKEY key;
	long result;

	// Store instance handle in our global variable
	global_hInstance = hInstance;

	// Create Windows
	WinInit();

	// Grab Paths
	GetCurrentDirectory(256, initialPath);
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_STRING, 0, KEY_QUERY_VALUE, &key);
	assert(result == ERROR_SUCCESS && "DemoInit: Registry path not found!");
	result = RegQueryValueEx(key, "Path", 0, &type, installPath, &size);
	assert(result == ERROR_SUCCESS && "DemoInit: Registry path subkey not found!");
	RegCloseKey(key);

	// User's Main
	main();

	return 0;
}

//----------------------------------------------------------------------------
// Needs to be Called every frame to check Win32 status
void WinSwapBuffers(void) {
	MSG msg;

	winKey.downCnt = 0;
	winKey.upCnt = 0;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)	return;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	SwapBuffers(global_hDC);

	retraceCount ++;
}

//----------------------------------------------------------------------------
// Release Window Information
void WinDestroy(void) {
	DestroyWindow(global_hWnd);
}

//----------------------------------------------------------------------------
// Start the Output window
void WinStartConsole() {
	DWORD cCharsWritten;

	AllocConsole();
	outputWinHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	inputWinHandle = GetStdHandle(STD_INPUT_HANDLE);

	WriteConsole(outputWinHandle, outStr, strlen(outStr), &cCharsWritten, NULL);
}

//----------------------------------------------------------------------------
// Set Pixel Format
void WinSetDCPixelFormat(HDC hDC) {
	int fmtIndex;
	PIXELFORMATDESCRIPTOR   pxlfmt;

	ZeroMemory(&pxlfmt, sizeof(pxlfmt));
	pxlfmt.nSize = sizeof(pxlfmt);
	pxlfmt.nVersion = 1;
	pxlfmt.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pxlfmt.iPixelType = PFD_TYPE_RGBA;
	pxlfmt.cColorBits = 24;
	pxlfmt.cDepthBits = 16;
	pxlfmt.cAlphaBits = 8;
	pxlfmt.iLayerType = PFD_MAIN_PLANE;

	// Set Pixel Format
	fmtIndex = ChoosePixelFormat(hDC, &pxlfmt);
	if (SetPixelFormat(hDC, fmtIndex, &pxlfmt) == FALSE)
	{
		MessageBox(global_hWnd, "The current pixel depth is not supported. Contact support@noa.com", 
		"Pixel Depth", MB_OK | MB_ICONERROR);
		exit(0);
	}
}

//----------------------------------------------------------------------------
// Grab multi-texture extensions
void WinGLExtInit(void) {
	DWORD cCharsWritten;
	GLubyte * ext;

	// Check for the extension
	ext = (GLubyte *) glGetString(GL_EXTENSIONS);
	if (!strstr(ext, "GL_ARB_multitexture"))
	{
		WriteConsole(outputWinHandle, glWarning, strlen(glWarning), &cCharsWritten, NULL);
		MessageBox(global_hWnd, "Failed to initialize GL_ARB_multitexture Ext. Contact support@noa.com", 
		"No Multi-Texture", MB_OK | MB_ICONERROR);
		glActiveTextureARB = &glActiveTextureHACK;
		glMultiTexCoord2fARB = &glMultiTexCoord2fHACK; 
		glMultiTexCoord4fARB = &glMultiTexCoord4fHACK; 
		multiTexOn = 0;
	}
	else
	{
		// Get the extension Pointers
		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
		assert(glActiveTextureARB != NULL);
		glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) wglGetProcAddress("glMultiTexCoord2fARB");
		assert(glMultiTexCoord2fARB != NULL);
		glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC) wglGetProcAddress("glMultiTexCoord4fARB");
		assert(glMultiTexCoord4fARB != NULL);
		multiTexOn = 1;
	}
}

//----------------------------------------------------------------------------
// Create the Window
void WinInit() { 
	DWORD error;
	WNDCLASS wc;
	RECT rect;
	DWORD style = WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE;

	// Register Class
	wc.style            = CS_OWNDC;
	wc.lpfnWndProc      = (WNDPROC)WndProc;
	wc.cbClsExtra       = 0;
	wc.cbWndExtra       = 0;
	wc.hInstance        = global_hInstance;
	// wc.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_WIN32);
	wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground    = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName     = NULL;
	wc.lpszClassName    = APP_STRING;
	RegisterClass(&wc);

	// Adjust Window Size
	rect.left = 0;
	rect.right = 640;
	rect.top = 0;
	rect.bottom = 480;
	AdjustWindowRect(&rect, style, FALSE);

#ifdef OUTPUT_WINDOW
	// Output Window
	WinStartConsole();
#endif

	// Create Window
	global_hWnd = CreateWindow(APP_STRING, APP_STRING, 
	style,
	0, 0, 
	rect.right-rect.left, 
	rect.bottom-rect.top,
	NULL, NULL, 
	global_hInstance, NULL);

	// Make Sure
	if (!global_hWnd)
	{
		MessageBox(global_hWnd, "The Main Window could not be Created!", 
		"Initialize", MB_OK | MB_ICONERROR);
		error  = GetLastError();
		exit(0);
	}

	global_hDC = GetDC(global_hWnd);

	// Setup OpenGL
	WinSetDCPixelFormat(global_hDC);
	global_hRC = wglCreateContext(global_hDC);
	wglMakeCurrent(global_hDC, global_hRC);

	// OpenGL Extensions
	WinGLExtInit();

	return;
}

//----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message)
	{
		case WM_CREATE:
			return 0;

		case WM_DESTROY:
			// Clean Up
			if (outfile) fclose(outfile);
			FreeConsole();
			wglDeleteContext(global_hRC);
			ReleaseDC(global_hWnd, global_hDC);
			PostQuitMessage(0);
			exit(0);
			return 0;

		case WM_KEYDOWN:
			switch (wParam)
			{
				// Stick (Arrows)
				case VK_LEFT:
					winKey.stickX = -127;
					break;
				case VK_RIGHT:
					winKey.stickX = 127;
					break;
				case VK_UP:
					winKey.stickY = 127;
					break;
				case VK_DOWN:
					winKey.stickY = -127;
					break;
				// SubStick (SWAD)
				case 'A':
					winKey.substickX = -127;
					break;
				case 'D':
					winKey.substickX = 127;
					break;
				case 'W':
					winKey.substickY = 127;
					break;
				case 'S':
					winKey.substickY = -127;
					break;
				// Digital Pad
				case 'J':
					winKey.button |= WINKEY_PAD_BUTTON_LEFT;
					break;
				case 'L':
					winKey.button |= WINKEY_PAD_BUTTON_RIGHT;
					break;
				case 'I':
					winKey.button |= WINKEY_PAD_BUTTON_UP;
					break;
				case 'K':
					winKey.button |= WINKEY_PAD_BUTTON_DOWN;
					break;
				// Buttons
				case VK_NUMPAD2:
					winKey.button |= WINKEY_PAD_BUTTON_A;
					break;
				case VK_NUMPAD4:
					winKey.button |= WINKEY_PAD_BUTTON_B;
					break;
				case VK_NUMPAD5:
					winKey.button |= WINKEY_PAD_TRIGGER_Z;
					break;
				case VK_NUMPAD6:
					winKey.button |= WINKEY_PAD_BUTTON_X;
					break;
				case VK_NUMPAD8:
					winKey.button |= WINKEY_PAD_BUTTON_Y;
					break;
				case VK_NUMPAD7:
					winKey.button |= WINKEY_PAD_TRIGGER_L;
					break;
				case VK_NUMPAD9:
					winKey.button |= WINKEY_PAD_TRIGGER_R;
					break;
				case VK_NUMPAD0:
					winKey.button |= WINKEY_PAD_BUTTON_MENU;
					break;
				// Trigger
				case VK_NUMPAD1:
					winKey.triggerLeft = 255;
					break;
				case VK_NUMPAD3:
					winKey.triggerRight = 255;
					break;

				case VK_ESCAPE:
					WinDestroy();
					break;
			}
			if (winKey.downCnt < 64) winKey.down[winKey.downCnt++] = wParam;
			return 0;

		case WM_KEYUP:
			switch (wParam)
			{
				// Stick 
				case VK_LEFT:
				case VK_RIGHT:
					winKey.stickX = 0;
					break;
				case VK_UP:
				case VK_DOWN:
					winKey.stickY = 0;
					break;
				// SubStick
				case 'A':
				case 'D':
					winKey.substickX = 0;
					break;
				case 'W':
				case 'S':
					winKey.substickY = 0;
					break;
				// Digital Pad
				case 'J':
					winKey.button &= !WINKEY_PAD_BUTTON_LEFT;
					break;
				case 'L':
					winKey.button &= !WINKEY_PAD_BUTTON_RIGHT;
					break;
				case 'I':
					winKey.button &= !WINKEY_PAD_BUTTON_UP;
					break;
				case 'K':
					winKey.button &= !WINKEY_PAD_BUTTON_DOWN;
					break;
				// Buttons
				case VK_NUMPAD2:
					winKey.button &= !WINKEY_PAD_BUTTON_A;
					break;
				case VK_NUMPAD4:
					winKey.button &= !WINKEY_PAD_BUTTON_B;
					break;
				case VK_NUMPAD5:
					winKey.button &= !WINKEY_PAD_TRIGGER_Z;
					break;
				case VK_NUMPAD6:
					winKey.button &= !WINKEY_PAD_BUTTON_X;
					break;
				case VK_NUMPAD8:
					winKey.button &= !WINKEY_PAD_BUTTON_Y;
					break;
				case VK_NUMPAD7:
					winKey.button &= !WINKEY_PAD_TRIGGER_L;
					break;
				case VK_NUMPAD9:
					winKey.button &= !WINKEY_PAD_TRIGGER_R;
					break;
				case VK_NUMPAD0:
					winKey.button &= !WINKEY_PAD_BUTTON_MENU;
					break;
				// Trigger
				case VK_NUMPAD1:
					winKey.triggerLeft = 0;
					break;
				case VK_NUMPAD3:
					winKey.triggerRight = 0;
					break;
			}

			// Store Key information
			if (winKey.upCnt < 64) winKey.up[winKey.upCnt++] = wParam;
			return 0;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

//----------------------------------------------------------------------------
// Functions to Convert Endianess
typedef unsigned __int8     u8;
typedef unsigned __int16    u16;
typedef unsigned __int32    u32;
typedef unsigned __int64    u64;
void EndianSwap32(void* buffer) {
	u32 output;
	((u8*)&output)[0] = ((u8*)buffer)[3];
	((u8*)&output)[1] = ((u8*)buffer)[2];
	((u8*)&output)[2] = ((u8*)buffer)[1];
	((u8*)&output)[3] = ((u8*)buffer)[0];
	*((u32*)buffer) = output;
}
void EndianSwap16(void* buffer) {
	u16 output;
	((u8*)&output)[0] = ((u8*)buffer)[1];
	((u8*)&output)[1] = ((u8*)buffer)[0];
	*((u16*)buffer) = output;
}
u32 EndianSwapReturn32(void* buffer) {
	u32 output;
	((u8*)&output)[0] = ((u8*)buffer)[3];
	((u8*)&output)[1] = ((u8*)buffer)[2];
	((u8*)&output)[2] = ((u8*)buffer)[1];
	((u8*)&output)[3] = ((u8*)buffer)[0];
	return output;
}
u16 EndianSwapReturn16(void* buffer) {
	u16 output;
	((u8*)&output)[0] = ((u8*)buffer)[1];
	((u8*)&output)[1] = ((u8*)buffer)[0];
	return output;
}


unsigned __int32 __cntlzw(unsigned __int32 d)
{
     u32 i;
     for (i = 0; i < 32; i++) {
        if (d & (1 << (31-i))) break;
     }
     return ((i == 32) ? 31 : i);
}

