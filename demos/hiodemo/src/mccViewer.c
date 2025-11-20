/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - viewer module
File:     mccViewer.c

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

// ============================================================
//	MCC BITMAP VIEWER - filer module
// ============================================================
// [Descriptioin]
//	Bitmap display module for MCC bitmap viewer

//	HEADER INCLUDE
// ================
#include <dolphin.h>
#include "coreTV.h"	// TV display module for core loop manager.
#include "mccViewer.h"	// Viewer module.

// INTERNAL VARIABLES
// ==============================
static u8*	viewerBmpBuffer = NULL;		// Address of bitmap data.
static u32	viewerBmpWidth,viewerBmpHeight;	// Bitmap size.
static u32	viewerStatus = 0;		// Status of drawing status.

// CALLBACK PROTOTYPES
// ==============================
static void tvCallback( u8* graphics );


// ============================================================
//	API
// ============================================================

// Initializes TV screen.
// ==============================
void viewerInitialize()
{
	//	Initializes drawing status.
	viewerStatus = 0;
	//	Registers drawing callback.
	CoreTV_SetCallback( tvCallback );
}

// Draws bitmap on TV screen.
// ==============================
void viewerDrawBitmap( void *buffer, u32 bmpWidth, u32 bmpHeight )
{
	// Registers bitmap information.
	viewerBmpBuffer	= (u8*)buffer;
	viewerBmpWidth	= bmpWidth;
	viewerBmpHeight	= bmpHeight;
	// Starts to draw bitmap.
	viewerStatus = 1;
}


// ============================================================
//	CALLBACKS
// ============================================================

// Callback for TV drawing
// ==============================
static void tvCallback( u8* graphics )
{	u32 fbW=tvGetFrameWidth(),fbH=tvGetFrameHeight();

	// Drawing status
	// ------------------------------
	switch( viewerStatus ){
	case 0:{	// Initial state.( It will turn into this status when starting, or clearing screen is requested. 
		primFill( graphics, ColorValue(7) );
	}	break;
	case 1:	
	case 2:{	// Clears background.
		primFill( graphics, ColorValue(7) );
		viewerStatus++;
	}	break;
	case 3:
	case 4:{	// Draws bitmap.
		u32	iWidth = viewerBmpWidth/2, iHeight = viewerBmpHeight;
		primPutAt( graphics, viewerBmpBuffer, (fbW/4-iWidth)/2, (fbH-iHeight)/2, iWidth, iHeight );
		viewerStatus++;
	}	break;
	case 5:{	// Drawing completes.
	}	break;
	}
}
