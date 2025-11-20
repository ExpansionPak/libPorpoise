// MultiChannel.cpp: implementation of the CMultiChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "usbsample.h"
#include "MultiChannel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

struct MCCErrorMessage {
	INT nError;
	LPCTSTR lpMessage;
};

#define	NUMBEROF_ERRORS	(sizeof(ErrMsgTable)/sizeof(ErrMsgTable[0]))

static MCCErrorMessage ErrMsgTable[] ={
	{MCC_ERR_NOERR,"No error occurred.",},
	{MCC_ERR_UNINITIALIZED,"MCC is not initialized."},
    {MCC_ERR_TIMEOUT,"TIMEOUT error occurred."},
    {MCC_ERR_PING,"Failed PING."},
    {MCC_ERR_HIO_INIT,"Failed initialization of communication."},
    {MCC_ERR_HIO_MAILBOX_READ,"Unable to read from Mailbox."},
    {MCC_ERR_HIO_MAILBOX_WRITE,"Unable to write to Mailbox."},
    {MCC_ERR_HIO_READ,"Unable to read from the shared memory."},
    {MCC_ERR_HIO_WRITE,"Unable to write to the shared memory."},
    {MCC_ERR_HIO_STATUS_READ,"Unable to obtain interrupt status."},
    {MCC_ERR_INFO_FLUSH,"Internal error"},
    {MCC_ERR_INFO_LOAD,"Internal error"},
    {MCC_ERR_BLOCK_IS_NOT_ENOUGH,"Not enough space in the memory block."},
    {MCC_ERR_INVALID_PARAMETER,"Incorrect argument of the function is specified."},
    {MCC_ERR_INVALID_CHANNEL,"Channel ID is incorrect."},
    {MCC_ERR_INVALID_SIZE,"Size specified at reading/writing is invalid."},
    {MCC_ERR_INVALID_OFFSET,"Offset specified at reading/writing is invalid."},
    {MCC_ERR_CHANNEL_OPENED,"The channel is open."},
    {MCC_ERR_CHANNEL_CLOSED,"The channel is closed."},
    {MCC_ERR_CHANNEL_LOCKED,"The channel is locked."},
    {MCC_ERR_CHANNEL_UNLOCKED,"The channel is unlocked."},
    {MCC_ERR_CHANNEL_BUSY,"The channel is in use."},
};

LPCTSTR CMCCException::GetMessageString(INT nErr)
{
	for(int i=0;i<NUMBEROF_ERRORS;i++){
		if( ErrMsgTable[i].nError == nErr )
			return ErrMsgTable[i].lpMessage;
	}
	return "";
}

LPCTSTR CMCCException::What() const
{
	if( m_nError < 0 )
		return m_strMsg;
	return GetMessageString(m_nError);
}

static volatile INT nDevIndex;

static BOOL WINAPI hioEnumCallback(s32 ch);

static BOOL WINAPI hioEnumCallback(s32 ch)
{
	nDevIndex = ch;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiChannel::CMultiChannel()
{
	m_nDevice = -1;
}

CMultiChannel::~CMultiChannel()
{
	Terminate();
}

void CMultiChannel::Init(MCCCallbackSysEvent syscallback)
{
	nDevIndex = m_nDevice = -1;
	if( !MCCEnumDevices( hioEnumCallback ) ){
		throw CMCCException("No device");
	}
	m_nDevice = nDevIndex;
	if( m_nDevice < 0 ){
		throw CMCCException(MCCGetLastError());
	}
	m_nDevice = nDevIndex;
	if( !MCCInit( MCCExiChannel(m_nDevice),0,syscallback) ){
		throw CMCCException(MCCGetLastError());
	}
}

void CMultiChannel::Terminate()
{
	MCCExit();
}

