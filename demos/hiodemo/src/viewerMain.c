//----------------------------------------------------------------------
//	(C)2000-2001 Nintendo
//----------------------------------------------------------------------
#include <dolphin.h>
#include "viewerMcc.h"
#include "viewerDraw.h"

static void myMainInit( void );

//----------------------------------------------------------------------
//	Main
//
void main( void ) {

	OSInit();
	DVDInit();
	VIInit();
	PADInit();

	myMainInit();

	viewerResetEventFlag();
	while( 1 ) {
		while( viewerGetEventFlag() == FALSE ) {};

		// Notification sent from host

		viewerDrawMain();

		viewerResetEventFlag();
	}
}

//----------------------------------------------------------------------
//
static void myMainInit( void ) {

	viewerMccSystemInit();
	viewerDrawInit();
}
