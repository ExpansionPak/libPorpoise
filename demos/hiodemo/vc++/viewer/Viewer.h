// Viewer.h: interface for the CViewer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIEWER_H__1F763ABB_8732_4DF1_8B3C_64423E6057C3__INCLUDED_)
#define AFX_VIEWER_H__1F763ABB_8732_4DF1_8B3C_64423E6057C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Dolphin/mcc.h"

class CViewerException
{
protected:
	CString m_strMsg;
public:
	CViewerException(LPCTSTR pMsg){
		m_strMsg = pMsg;
	}
	LPCTSTR What() const {
		return m_strMsg;
	}
};

class CViewer
{
public:
	enum {
		EV_SEND_FILENAME = 1,
		EV_SEND_FILENAME_R,
	};
	enum {
		ST_NOERROR,
		ST_ILLEGAL_FORMAT,
		ST_ILLEGAL_SIZE,
		ST_ILLEGAL_COLORS,
	};

	INT m_nLastResultStatus;

protected:
	MCCChannel	m_Channel;
	INT	m_nBlocks;
	BOOL	m_bOpen;
public:
	INT SendFileName(LPCTSTR lpFileName);
	void Stop();
	void Start();
	CViewer(MCCChannel ch = MCC_CHANNEL_8,INT blocks = 1);
	virtual ~CViewer();

	operator MCCChannel() const;
};

inline CViewer::operator MCCChannel() const
{
	return m_Channel;
}

#endif // !defined(AFX_VIEWER_H__1F763ABB_8732_4DF1_8B3C_64423E6057C3__INCLUDED_)
