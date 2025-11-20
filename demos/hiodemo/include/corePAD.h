/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - PAD module for core loop manager
File:     corePAD.h

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

#ifndef __CORE_PAD
#define __CORE_PAD

//	CALLBACK PROTOTYPES
// ================
typedef void (*CORE_PAD_CALLBACK)( u16 up, u16 down );

//	FUNCTION PROTOTYPES
// ================
void CorePAD_Begin();
void CorePAD_Task( void* );
void CorePAD_SetCallback( CORE_PAD_CALLBACK callback );

#endif __CORE_PAD