// Viewer.cpp: implementation of the CViewer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "usbsample.h"
#include "Viewer.h"
#include "MultiChannel.h"
#include "ChildView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static CViewer *pThis = NULL;
extern CChildView*	g_pFIOView;

void WINAPI viewerCallback( MCCChannel chID, MCCEvent event, u32 value )
{
	TRACE("viewerCallback Channel=%d event=%d value=%d\n",chID,event,value);
	switch( event ){
	case MCC_EVENT_USER:
		if( value == CViewer::EV_SEND_FILENAME_R ){
			ASSERT(pThis != NULL);
			BYTE Temp[4];
			if( MCCRead(MCCChannel(*pThis),0,Temp,4,MCC_SYNC) ){
				pThis->m_nLastResultStatus = Temp[0];
				if( Temp[0] != 0 )
					g_pFIOView->AddString(" Viewer : Failed.\n");
			} else {
				pThis->m_nLastResultStatus = -1;
			}
			AfxGetMainWnd()->SendMessage(WM_SETCURSOR,HTCLIENT,0);
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CViewer::CViewer(MCCChannel ch,INT blocks)
{
	ASSERT(pThis == NULL);
	pThis = this;
	m_Channel = ch;
	m_nBlocks = blocks;
	m_nLastResultStatus = 0;
	m_bOpen = FALSE;
}

CViewer::~CViewer()
{
	Stop();
	pThis = NULL;
}

void CViewer::Start()
{
	TRACE("CViewer::Start\n");
	if( !MCCOpen(m_Channel,m_nBlocks,viewerCallback) ){
		CString msg;
		INT nErr = MCCGetLastError();
		msg.Format("Viewer : MCCOpen failed.(%d:%s)",nErr,CMCCException::GetMessageString(nErr));
		throw CViewerException(msg);
	}
	m_bOpen = TRUE;
}

void CViewer::Stop()
{
	TRACE("CViewer::Stop\n");
	if( m_bOpen && !MCCClose(m_Channel) ){
		CString msg;
		INT nErr = MCCGetLastError();
		msg.Format("Viewer : MCCClose failed.(%d:%s)",nErr,CMCCException::GetMessageString(nErr));
		throw CViewerException(msg);
	}
	m_bOpen = FALSE;
}

INT CViewer::SendFileName(LPCTSTR lpFileName)
{
	TCHAR Buffer[512];
	TRACE("CViewer::SendFileName(%s)\n",lpFileName);
	memset(Buffer,0,sizeof(Buffer));
	lstrcpy(Buffer,lpFileName);
	m_nLastResultStatus = -2;
	if( !MCCWrite(m_Channel,0,Buffer,sizeof(Buffer),MCC_SYNC) ){
		CString msg;
		m_nLastResultStatus = -1;
		INT nErr = MCCGetLastError();
		msg.Format("Viewer : MCCWrite failed.(%d:%s)",nErr,CMCCException::GetMessageString(nErr));
		throw CViewerException(msg);
	}

	if( !MCCNotify(m_Channel,EV_SEND_FILENAME) ){
		CString msg;
		INT nErr = MCCGetLastError();
		m_nLastResultStatus = -1;
		msg.Format("Viewer : MCCNotify failed.(%d:%s)",nErr,CMCCException::GetMessageString(nErr));
		throw CViewerException(msg);
	}
	return m_nLastResultStatus;
}
