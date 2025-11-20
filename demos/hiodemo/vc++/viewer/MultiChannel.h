// MultiChannel.h: interface for the CMultiChannel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MULTICHANNEL_H__9B33747A_1785_4E08_8B98_D53AE7AFFCA4__INCLUDED_)
#define AFX_MULTICHANNEL_H__9B33747A_1785_4E08_8B98_D53AE7AFFCA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Dolphin/mcc.h"

class CMCCException
{
protected:
	INT m_nError;
	CString m_strMsg;
public:
	CMCCException(INT nErr = 0){
		m_nError = nErr;
	}
	CMCCException(LPCTSTR pMsg){
		m_strMsg = pMsg;
		m_nError = -1;
	}
	LPCTSTR What() const;

	static LPCTSTR GetMessageString(INT nErr);
};

class CMultiChannel  
{
protected:
	INT	m_nDevice;
public:
	void Terminate();
	void Init(MCCCallbackSysEvent syscallback = NULL); // throw(CMCCException);
	CMultiChannel();
	virtual ~CMultiChannel();
};

#endif // !defined(AFX_MULTICHANNEL_H__9B33747A_1785_4E08_8B98_D53AE7AFFCA4__INCLUDED_)
