/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - main module
File:     main.c

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

// ============================================================
//	MCC BITMAP VIEWER - main module
// ============================================================
// [Description]
//	Viewer to display selected bitmap file of DVD or FIO on TV screen.

//	HEADER INCLUDE
// ================
#include <dolphin.h>
#include <dolphin/mcc.h>	// MCC module.
#include <dolphin/tty.h>	// MCC:tty module.
#include <dolphin/fio.h>	// MCC:fio module.
#include <string.h>
#include <ctype.h>

#include "core.h"	// Core loop manager.
#include "corePAD.h"	// PAD input module for core loop manager.
#include "coreTV.h"	// TV display module for core loop manager.
#include "mccViewer.h"	// Viewer module.
#include "mccFiler.h"	// Filer module.

//	INTERNAL VARIABLES
// ================
static volatile BOOL bInitialized=FALSE;

//	FUNCTION PROTOTYPES
// ================
static void MyOSInit( void );
static BOOL MyMccInit( void );

// CALLBACK PROTOTYPES
// ================
static void beginTask( void* param );
static void mccCallback( MCCSysEvent event );


// ========================================
//	CALLBACKS
// ========================================

//	Initializes CORE SYSTEM
// ==============================
static void beginTask( void* param )
{	param=param;
	//	Initializes each module.
	CoreTV_Begin();
	CorePAD_Begin();
	//	Initializes filer and viewer.
	filerInitialize();
	viewerInitialize();
	//	Registers task for each module
	CoreTaskAdd( CoreTV_Task, NULL, 10, NULL );
	CoreTaskAdd( CorePAD_Task, NULL, 10, NULL );
}

//	MCC callback
// ==============================
static void mccCallback( MCCSysEvent event )
{	if(event == MCC_SYSEVENT_INITIALIZED)
		bInitialized=TRUE;
}


// ========================================
//	FUNCTIONS
// ========================================

// Initializes OS.
// ==============================
static void MyOSInit ( void )
{
	OSReport("\nOS Initialize...\n====================\n");
	//	OS Initialize
    OSInit();
    VIInit();
	PADInit();
    DVDInit();
    return;
}

//	Initializes MCC
// ==============================
static BOOL MyMccInit ( void )
{	BOOL result=FALSE;

	OSReport("\nMCC Initialize...\n====================\n");
	//	MCC Initialize
	if(!MCCInit( MCC_EXI_1, 0, mccCallback )){
		OSReport("MCCInit.NG\n");
	}else{
		OSReport("wait for mcc initialize by host.\n");
		while(!bInitialized){};
		OSReport("mcc ready.\n");
		if(!TTYInit( MCC_EXI_1, MCC_CHANNEL_8 )){
			OSReport("TTYInit.NG\n");
		}else{
			OSReport("TTYInit.READY\n");
			if(!FIOInit( MCC_EXI_1, MCC_CHANNEL_1, 10 )){
				OSReport("FIOInit.NG\n");
			}else{
				OSReport("FIOInit.READY\n");

				// query:wait for server start.
				OSReport("wait for server start.\n");
				while((TTYQuery()+FIOQuery()*2)!=3){};
				OSReport("connect to server.\n");

				result=TRUE;
			}
		}
	}
	return result;
}


// ========================================
//	main
// ========================================
void main()
{
	MyOSInit();
	if(!MyMccInit()){
		OSHalt("MCC Initialize error.\nBye!!\n");
	}else{
		CoreBegin( beginTask );
	}
}
