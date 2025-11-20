//----------------------------------------------------------------------
//	(C)2000-2001 Nintendo
//----------------------------------------------------------------------
#ifndef	_VIEWERMCC_H_
#define	_VIEWERMCC_H_

#define	VIEWER_TTY_CHANNEL_ID		(MCCChannel)(MCC_CHANNEL_1)
#define	VIEWER_FIO_CHANNEL_ID		(MCCChannel)(MCC_CHANNEL_2)
#define	VIEWER_VIEWER_CHANNEL_ID	(MCCChannel)(MCC_CHANNEL_3)

#define	VIEWER_MCC_NOTIFY_ID		2
#define	VIEWER_MCC_NOTIFY_COMPLETE	0
#define	VIEWER_MCC_NOTIFY_ERROR		1

void viewerMccSystemInit( void );
void viewerMccWrite( u8 data );
void viewerResetEventFlag( void );
BOOL viewerGetEventFlag( void );

#endif	//_VIEWERMCC_H_
