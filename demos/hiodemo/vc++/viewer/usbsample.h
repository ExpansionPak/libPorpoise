// usbsample.h : main header file for the USBSAMPLE application
//

#if !defined(AFX_USBSAMPLE_H__CF40AD07_2E83_43A6_8940_6EAD6E00ED1C__INCLUDED_)
#define AFX_USBSAMPLE_H__CF40AD07_2E83_43A6_8940_6EAD6E00ED1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CUsbsampleApp:
// See usbsample.cpp for the implementation of this class
//

class CUsbsampleApp : public CWinApp
{
public:
	CUsbsampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUsbsampleApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	HMENU m_hMDIMenu;
	HACCEL m_hMDIAccel;
	
public:
	CString	m_strFIORootDirectory;
	bool	m_bActiveServer;

	void StartServer();
	void StopServer();
	//{{AFX_MSG(CUsbsampleApp)
	afx_msg void OnAppAbout();
	afx_msg void OnAppStartserver();
	afx_msg void OnUpdateAppStartserver(CCmdUI* pCmdUI);
	afx_msg void OnAppChooseDir();
	afx_msg void OnUpdateAppChooseDir(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USBSAMPLE_H__CF40AD07_2E83_43A6_8940_6EAD6E00ED1C__INCLUDED_)
