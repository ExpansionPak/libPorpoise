/*---------------------------------------------------------------------*

Project:  mcc viewer server - app. class
File:     viewer_server.h

(C) 2000 Nintendo

-----------------------------------------------------------------------*/
// viewer_server.h : main header file for the VIEWER_SERVER application
//

#if !defined(AFX_VIEWER_SERVER_H__5C4FD4BA_B742_11D4_9146_0000E8DB33B8__INCLUDED_)
#define AFX_VIEWER_SERVER_H__5C4FD4BA_B742_11D4_9146_0000E8DB33B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CViewer_serverApp:
// See viewer_server.cpp for the implementation of this class
//

class CViewer_serverApp : public CWinApp
{
public:
	CViewer_serverApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewer_serverApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CViewer_serverApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWER_SERVER_H__5C4FD4BA_B742_11D4_9146_0000E8DB33B8__INCLUDED_)
