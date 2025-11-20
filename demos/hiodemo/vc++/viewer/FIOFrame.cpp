// FIOFrame.cpp : implementation file
//

#include "stdafx.h"
#include "usbsample.h"
#include "FIOFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFIOFrame, CChildFrame)
/////////////////////////////////////////////////////////////////////////////
// CFIOFrame

CFIOFrame::CFIOFrame()
{
	m_strTitle = _T("File I/O Window");
}

CFIOFrame::~CFIOFrame()
{
}


BEGIN_MESSAGE_MAP(CFIOFrame, CChildFrame)
	//{{AFX_MSG_MAP(CFIOFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFIOFrame message handlers

int CFIOFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CChildFrame::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}
