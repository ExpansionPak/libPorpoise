// HioAccess.cpp: implementation file
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "color.h"
#include "HioAccess.h"

#include <dolphin/hio.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// DEFINE
//////////////////////////////////////////////////////////////////////

#define HIO_ACCESS_BUFFER_SIZE	(32)
#define HIO_ACCESS_RGB_ADDRESS	(0x1000)
#define HIO_ACCESS_MSG_ADDRESS	(HIO_ACCESS_RGB_ADDRESS+HIO_ACCESS_BUFFER_SIZE)

#define HIO_ACCESS_DATA_IDX_RED		0
#define HIO_ACCESS_DATA_IDX_GREEN	1
#define HIO_ACCESS_DATA_IDX_BLUE	2
#define HIO_ACCESS_DATA_IDX_ACK		3

#define HIO_ACCESS_CMD_REQMSG	0x0000000F
#define HIO_ACCESS_CMD_CLRMSG	0x00000000


//////////////////////////////////////////////////////////////////////
// Callback
//////////////////////////////////////////////////////////////////////

static CHioAccess* cHioAccess=NULL;

BOOL WINAPI hioEnumCallback( s32 chan )
{	cHioAccess->m_nExiChan = chan;
	return FALSE;
}

void WINAPI hioCallback( void )
{	cHioAccess->HioCallback();
}


//////////////////////////////////////////////////////////////////////
// CColorApp construction

CHioAccess::CHioAccess()
{	cHioAccess=this;
}

CHioAccess::~CHioAccess()
{
	HIOExit();
}


//////////////////////////////////////////////////////////////////////
// API
//////////////////////////////////////////////////////////////////////

BOOL CHioAccess::Initialize()
{
	if(!HIOEnumDevices(hioEnumCallback)){
	}else{
		if(!HIOInit( m_nExiChan, hioCallback )){
		}else{
			ClearMemory();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CHioAccess::SetColorRed(BYTE value)
{	// via adapter memory
	return WriteToMemory( HIO_ACCESS_RGB_ADDRESS, HIO_ACCESS_DATA_IDX_RED, value );
}

BOOL CHioAccess::SetColorGreen(BYTE value)
{	// via adapter memory
	return WriteToMemory( HIO_ACCESS_RGB_ADDRESS, HIO_ACCESS_DATA_IDX_GREEN, value );
}

BOOL CHioAccess::SetColorBlue(BYTE value)
{	// via adapter memory
	return WriteToMemory( HIO_ACCESS_RGB_ADDRESS, HIO_ACCESS_DATA_IDX_BLUE, value );
}

BOOL CHioAccess::ChangeMode(BOOL bSet)
{	// use mail box
	return HIOWriteMailbox( (bSet!=FALSE)?HIO_ACCESS_CMD_REQMSG:HIO_ACCESS_CMD_CLRMSG );
}

BOOL CHioAccess::GetMessage( CString& message )
{	// via adapter memory
	BYTE buffer[HIO_ACCESS_BUFFER_SIZE+1];

	if(!ReadFromMemory( HIO_ACCESS_MSG_ADDRESS, buffer )){
	}else{
		buffer[HIO_ACCESS_BUFFER_SIZE]='\0';
		message = (LPCSTR)buffer;
		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////

BOOL CHioAccess::ClearMemory()
{
	BYTE buffer[HIO_ACCESS_BUFFER_SIZE];
	for(int i=0;i<HIO_ACCESS_BUFFER_SIZE; i++) buffer[i]=0;
	//	clear memory
	if(!HIOWrite( HIO_ACCESS_RGB_ADDRESS, buffer, HIO_ACCESS_BUFFER_SIZE )){
	}else{
		//	clear memory
		if(!HIOWrite( HIO_ACCESS_MSG_ADDRESS, buffer, HIO_ACCESS_BUFFER_SIZE )){
		}else{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CHioAccess::ReadFromMemory(DWORD addr, BYTE* data)
{
	//	read data from adapter's memory
	if(!HIORead( addr, data, HIO_ACCESS_BUFFER_SIZE )){
	}else{
		return TRUE;
	}
	return FALSE;
}

BOOL CHioAccess::WriteToMemory(DWORD addr, BYTE offset, BYTE value)
{
	BYTE buffer[HIO_ACCESS_BUFFER_SIZE];
	//	read data from adapter's memory
	if(!ReadFromMemory( addr, buffer )){
	}else{
		//	set color value
		buffer[offset] = value;
		//	write back data
		if(!HIOWrite( addr, buffer, HIO_ACCESS_BUFFER_SIZE )){
		}else{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CALLBACK
//////////////////////////////////////////////////////////////////////

void CHioAccess::HioCallback( )
{	// no use
	AfxMessageBox("Detect MB writting.");
}
