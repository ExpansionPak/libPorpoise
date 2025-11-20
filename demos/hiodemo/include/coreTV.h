/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - TV module for core loop manager
File:     coreTV.h

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

#ifndef __CORE_TV
#define __CORE_TV

//	CALLBACK PROTOTYPES
// ================
typedef void (*CORE_TV_CALLBACK)( u8* graphics );

//	API for core loop manager
// ================
void CoreTV_Begin();
void CoreTV_Task( void* );
void CoreTV_SetCallback( CORE_TV_CALLBACK callback );

//	API
// ================
u32	ColorValue( int colorCode);
u32	tvGetFrameWidth();
u32	tvGetFrameHeight();
//
void primFill	( u8* xfb, u32 colorVal );
void primPoint	( u8* xfb, u32 colorVal, u32 x, u32 y );
void primLine	( u8* xfb, u32 colorVal, u32 x, u32 y, u32 w );
void primBox	( u8* xfb, u32 colorVal, u32 x, u32 y, u32 w, u32 h );
void primPutAt	( u8* xfb, u8* image, u32 x, u32 y, u32 w, u32 h );

#endif __CORE_CONS