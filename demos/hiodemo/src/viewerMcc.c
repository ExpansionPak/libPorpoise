//----------------------------------------------------------------------
//	(C)2000-2001 Nintendo
//----------------------------------------------------------------------
#include <dolphin.h>
#include <dolphin/mcc.h>
#include <dolphin/tty.h>
#include <dolphin/fio.h>
#include "viewerMcc.h"

static s32		mccExiChannelNumber;
static volatile BOOL	mccEnumDevicesCallbackFlag;
static volatile BOOL	mccInitFlag;
static volatile BOOL	mccOpenFlag;
static volatile BOOL	mccPingFlag;
static volatile BOOL	receiveEventUserFlag;
static u8		writeBuffer[32] ATTRIBUTE_ALIGN(32);

static BOOL mccEnumDevicesCallback( s32 channel );
static void mccSysEventCallback( MCCSysEvent sysEvent );
static void mccEventCallback( MCCChannel chID , MCCEvent event , u32 value );

//----------------------------------------------------------------------
//	Initialize multi-channel communications (MCC)
//
void viewerMccSystemInit( void ) {

	// Find EXI channel to which USB I/F is connected
	mccExiChannelNumber = -1;
	mccEnumDevicesCallbackFlag = FALSE;
	if( MCCEnumDevices( mccEnumDevicesCallback ) == FALSE ) {
		OSHalt("MCCEnumDevices() error\nTerminated\n");
	}
	// Wait for callback
	while( mccEnumDevicesCallbackFlag == FALSE ) {};
	if( mccExiChannelNumber < 0 ) {
		OSHalt("Could not get EXI channel number\nTerminated\n");
	}
	OSReport("EXI channel number %d\n",mccExiChannelNumber);

	OSReport("Waiting for host program to start\n");

	// MCC system initialization
	mccInitFlag = FALSE;
	if( MCCInit( (MCCExiChannel)mccExiChannelNumber , 0 , mccSysEventCallback ) == FALSE ) {
		OSHalt("MCCInit() error\nTerminated\n");
	}
	// Wait for callback
	while( mccInitFlag == FALSE ) {};
	OSReport("MCCInit() normal completion\n");

	OSReport("\nMCC system initialization in progress\n");

	// Console output initialization
	if( TTYInit( (MCCExiChannel)mccExiChannelNumber , VIEWER_TTY_CHANNEL_ID ) == FALSE ) {
		OSHalt("TTYInit() error\nTerminated\n");
	}
	OSReport("TTYInit() normal completion\n");

	// File I/O initialization
	if( FIOInit( (MCCExiChannel)mccExiChannelNumber , VIEWER_FIO_CHANNEL_ID , 4 ) == FALSE ) {
		OSHalt("FIOInit() error\nTerminated\n");
	}
	OSReport("FIOInit() normal completion\n");

	OSReport("Click 'Enable Server' on the host\n");

	// Check whether host is functioning
	while( FIOQuery() == FALSE ) {};
	OSReport("FIOQuery() normal completion\n");

	// Check whether host is functioning
	while( TTYQuery() == FALSE ) {};
	OSReport("TTYQuery() normal completion\n");

	// MCC channel open
	mccOpenFlag = FALSE;
	if( MCCOpen( VIEWER_VIEWER_CHANNEL_ID , 1 , mccEventCallback ) == FALSE ) {
		OSHalt("MCCOpen() error\nTerminated\n");
	}
	// Wait for callback
	while( mccOpenFlag == FALSE ) {};
	OSReport("MCCOpen() normal completion\n");

	// MCC operation confirmation
	mccPingFlag = FALSE;
	if( MCCPing() == FALSE ) {
		OSHalt("MCCPing() error\nTerminated\n");
	}
	// Wait for callback
	while( mccPingFlag == FALSE ) {};
	OSReport("MCCPing() normal completion\n");

	viewerResetEventFlag();

	OSReport("MCC system initialization completed\n");
	TTYPrintf("MCC system initialization completed\n");
}

//----------------------------------------------------------------------
//	viewer MccWrite
//
void viewerMccWrite( u8 data ) {

	int		i;

	for( i=0 ; i<32 ; i++ ) {
		writeBuffer[i] = 0;
	}
	writeBuffer[0] = data;
	if( MCCWrite( VIEWER_VIEWER_CHANNEL_ID , 0 , writeBuffer , 32 , MCC_SYNC ) != FALSE ) {
		MCCNotify( VIEWER_VIEWER_CHANNEL_ID , VIEWER_MCC_NOTIFY_ID );
	}
}

//----------------------------------------------------------------------
//	receiveEventUserFlag = FALSE;
//
void viewerResetEventFlag( void ) {

	receiveEventUserFlag = FALSE;
}

//----------------------------------------------------------------------
//	check receiveEventUserFlag
//
BOOL viewerGetEventFlag( void ) {

	return( receiveEventUserFlag );
}

//----------------------------------------------------------------------
//	MCCEnumDevices callback
//
static BOOL mccEnumDevicesCallback( s32 channel ) {

	mccExiChannelNumber = channel;
	mccEnumDevicesCallbackFlag = TRUE;
	return( FALSE );
}

//----------------------------------------------------------------------
//	MCCSysEvent callback
//
static void mccSysEventCallback( MCCSysEvent sysEvent ) {

	switch( sysEvent ) {
		case	MCC_SYSEVENT_INITIALIZED:
			mccInitFlag = TRUE;
			break;
		case	MCC_SYSEVENT_PING_RESULT:
			mccPingFlag = TRUE;
			break;
	}
}

//----------------------------------------------------------------------
//	Channel event callback
//
static void mccEventCallback( MCCChannel chID , MCCEvent event , u32 value ) {

	switch( event ) {
		case	MCC_EVENT_CONNECTED:
			mccOpenFlag = TRUE;
			break;
		case	MCC_EVENT_USER:
			if( chID == VIEWER_VIEWER_CHANNEL_ID ) {
				if( receiveEventUserFlag == TRUE ) {
					// Return, because rendering in progress
					return;
				}
				if( value == 1 ) {
					receiveEventUserFlag = TRUE;
				}
			}
			break;
	}
}
